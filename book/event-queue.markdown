^title Event Queue
^section Decoupling Patterns

## Intent

*Decouple when a message or event is generated from when it is processed.*

## Motivation

Unless you live under one of the few rocks that still lacks reliable Internet access, there's a good chance you've already heard of an "event queue" before. If not, maybe "message queue", or "event loop", or "message pump" will ring a bell. These are all related terms for a roughly similar idea.

This pattern comes up frequently under a number of guises because it solves a bunch of different problems. To refresh your memory, I'll walk through a couple of the most common manifestations of it.

### GUI event loops

If you've ever done any <span name="event-driven">user interface</span> programming, then you're well acquainted with the *events*. Every time the user interacts with your program -- clicks a button, pulls down a menu, or presses a key -- the operating system turns this into an event object. It sends this object at your app, and your job is to receive it and hook it up to some interesting behavior.

<aside name="event-driven">

This application architecture is so common, it's considered a paradigm: <a href="http://en.wikipedia.org/wiki/Event-driven_programming">*event-driven programming*</a>.

</aside>

It's rare for application programmers to write this code these days, but the way this works is that somewhere deep in the bowels of your code is an *event loop*. It looks something roughly like this:

^code event-loop

When you call `getNextEvent()` that pulls the some unprocessed user input into your app. Then you route it to the appropriate event handler and, like magic, your application comes to life.

The <span name="interrupt">interesting</span> part is that the application *pulls* in an event when *it* wants it. When the user presses the mouse, the operating system doesn't just immediately invoke some code in your app that *pushes* the event in.

<aside name="interrupt">

In constrast, *interrupts* from the operating system *do* work like that. When an interrupt happens, the OS stops whatever your app was doing and forces it to jump to an interrupt handler. This abruptness is why interrupts are so hard to work with.

</aside>

That means when the user input comes in, it needs to go... somewhere... so that we don't lose it between when the user does something and when your app gets around to calling `getNextEvent()`. That "somewhere" is a *queue*.

When user input comes in, the OS adds it to a queue of unprocessed events. When you call `getNextEvent()`, it pulls the oldest event off the queue and hands it to your application. (If there are no events in the queue, it usually blocks and waits until one comes in.)

As an application programmer, you don't usually see this event queue, but it's there.

### Central event bus

If the above section is news to you, don't sweat it. Most <span name="game-loop">games</span> aren't event-driven like this. Instead, the cadence that drive's the game's core application loop is usually time.

<aside name="game-loop">

For all of the gory details behind this, crack open the <a href="game-loop.html" class="pattern">Game Loop</a> chapter.

</aside>

But it *is* pretty common for games to have some other central event queue as part of the backbone of the nervous system of the program. You'll often here "central" or "global" or "main" used to describe it. It's used for high level communication between game systems that want to stay decoupled.

For example, say your game has a <span name="tutorial">tutorial</span> system. You need little help popups to appear queued off of specific in-game events. For example, the first time the player vanquishes a foul beastie, you want to show a little help balloon that says, "Press X to grab the loot!"

Your gameplay and combat code is likely complex enough as it is. The last thing you want to do is stuff a bunch of checks for triggering tutorials in there. (Not to mention all of the other places in the codebase that end up triggering a help balloon.)

<aside name="tutorial">

In-game tutorials are one of those features that always seems to be a challenge to implement gracefully. Worse, it's a chunk of code that any given player will only use once or twice in the entire lifetime of the game.

That makes it easy to underestimate how helpful a good tutorial can be. It's a hassle, but your user will love you for it.

</aside>

Instead, what some games do is have an event queue. This is publicly visible to pretty much the entire game. Any game system can throw an event on the queue. For example, the combat code can add a "enemy died" event every time you slay a foe.

<span name="blackboard">Likewise</span>, any game system can receive events from the queue. So the tutorial
engine would register itself with the queue and indicate it wants to known about "enemy died" events. This way, knowledge of an enemy dying makes its way from the combat system over to the tutorial engine without the two being directly aware of each other.

<aside name="blackboard">

In the field of AI, there is a very similar architectural pattern called a <a href="http://en.wikipedia.org/wiki/Blackboard_system">blackboard system</a>.

</aside>

This would have made a decent example for the rest of this chapter. But I'm not generally a fan of global systems like this. It is a common technique, but I don't want you to think that event queues *have* to be global.

Instead, we'll use something more scoped. Event queues can solve a bunch of different problems. To get the most mileage out of this chapter, I'm going to contrive a <span name="perfect">motivating</span> example that jams all of those problems together. Then we'll slap a queue in and watch it knock those problems down, one after the other.

<aside name="perfect">

Of course, in your games, you're unlikely to find problems so cleanly suited to a particular pattern. This is the luxury of getting to cherry-pick examples for a book.

</aside>

### Say what?

- working on adding audio to game
- often overlooked facet of game dev
- humans are primarily visual but sound is deeply keyed to our emotions and sense of physical space
- good sound can make you feel surrounded by the game and make heartstrings ring with every action the hero takes

- lots of things trigger sounds: collisions in physics engine, using items, gameplay events like leveling up, level events like opening door, npc ai yelling at you, and hero herself
- start with simple approach
- have little "audio engine" in game
- it has api for playing a sound given id and volume
  (3d sound would have more complex api)
- knows how to find appropriate sound file and throw it at hardware to make it
  play

- start sprinkling call throughout codebase
- in ui code, when selected menu item changes, trigger sound
- run into first problem: audio api takes a little while to locate sound
  resource and copy buffer of data to somewhere hardware can play it
- since api is synchronous, that blocks ui! hitches for a few frames

- **problem 1: requesting a sound to play blocks the caller until the audio
  engine has started playing the sound**

- ignoring that for now, we move on. in code for monster, trigger "oof" when
  takes damage from hero

- works pretty well, but occasional problem
- can sometimes hit two monsters in exact same frame
- when that happens, plays sound twice at *exact* same time
- if you know anything about audio, knows that basically stacks the waveforms
- do that with the *same* waveform and it's just like multiplying it
- in other words, sounds like *one* "oof" sound played twice as long
- sounds jarring and wrong
- [ran into exact problem in hatsworth]

- **problem 2: redundant requests are handled individual and behave wrongly since they are unaware of each other**

- things get worse in big boss fights with lots of enemies and action going on
- hardware can only play so many sounds at one time
- if try to play too many sounds at once, some get dropped or cut off
- sounds designers anticipated this by recording some "multi sounds"
- so, if have bunch of small explosions around same time, instead of using a bunch of channels on them, can use a single channel to play one "big bang" sound
- but need to know that we're playing a bunch of small sounds at once
- since audio just gets individual api calls, can't look at them in aggregate

- **problem 3: can't analyze, group, and process a series of requests at once**

- now let's get to real nasty problem
- got lots of calls to audio api in game now, coming from lots of different
  systems
- scary part is many of those systems running on different threads!
- since audio api is sync, code runs on *caller's* thread
- if gameplay thread triggers sound at same time as physics thread, all hell
  breaks loose
- or have to do careful sync, which slows down both threads
- and (as typical) want audio to be on its *own* thread

- **problem 4: can't respond to requests on different thread from caller

### Leave a message

- common thread in these problems is that audio api we expose means play sound
  *right now*
- the right-now-ness is the problem
- point in time that caller requests a sound is time that's convenient for
  *caller*, not audio engine
- to solve, decouple when we request sound be played and when audio engine
  actually plays it

- simplest way to do that would be to have a slot in audio engine for "requested sound"
- calling api just stores sound id in that slot and returns
- later, when audio engine is updated by game loop, checks slot and starts sound
- problem is that if you make *two* audio calls before engine processes first,
  slot is already full
- want a bunch of slots
- calling api will fill next available one
- updating audio engine walks them in the order they were filled so oldest
  request is handled first
- that's a queue!

## The Pattern

A **queue** stores a series of **notifications or requests** in last-in, first-out order. Sending a notification simply **enqueues the request and returns**. The request processor then **processes items from the queue later** at an appropriate time.

Requests can be **handled directly**, or **routed to interested parties**. This **decouples the sender from the receiever** both **statically** and **in time**.

## When to Use It

- queueing is useful when you have stream of stuff coming in -- events, request
  notifications
- want to isolate source of requests from object or objects that will be
  responding to or processing them
- lots of patterns decouple stuff like that: observer and command are two
  similar ones
- difference here is queue decouples sender/receiver *temporally*
- sender not only doesn't know who will respond, doesn't know when

- gives responder more control and context over handling stuff
- can see entire list of things to respond
- can respond at appropriate time in game loop, or on different thread
- sender sacrifices control

- good bit more complex than other solutions
- queue takes memory
- have to make sure gets processed in timely manner
- normally i start with something simpler and sync like observer and then go
  to queue when know i need it
- "need it" usually means sending event or message is causing problems for
  sender

- this is probably obvious, but queues are poor fit when sender needs response
- fire and forget
- sometimes see async responses too: send async event, handler sends async
  response
- then original sender processes it
- works *sometimes* but sketchy. original sender may have changed state or not
  be in good place to handle response by time it gets it
- do you really want effective complexity of networking code *inside* game
  engine?

## Keep in Mind

### global queue is still global

- many games have global event queue for sharing high-level events with any
  system that wants them
- useful for broadcasting events to any system that wants them
- lets high-level parts of codebase interact without coupling
- think playing sounds based on physics
- showing tutorial hints as player progresses through level, etc.

- but global is still bad
- global queue has same problems as other globals
- big mutable blog of data anything in game can write to (send event) and
  read to (receive event)

### state of world may have changed between sender/receiver

- say using above global queue
- say game entity sends "died" event when dies
- now say achievement system wants to monitor that to see if you kill certain
  number of some kind of entity
- with synchronous event system, even can just be simple event enum
- event receiver can then go inspec world to see what happened
- so achievement system gets died event, looks to see which entity in world has
  zero hp, etc.

- now consider queued event
- dying entity enqueues "die" event
- at end of frame, entity is removed from world
- next frame, go to process queue
- achievement system sees died event, but can no longer find entity
- since don't know when event will be handled, can't rely on state of world
  being same between when sent and when handled
- means event/message objects themselves larger and more self-contained
- with sync events, it's basically "something happened, look at world to figure
  out what"
- with queued, it's "here's exactly what happened"

### lifecycle of event/message object

- with sync notification, event itself can be just couple of parameters to
  fn, or object on stack
- since done with it by time sending message is done, lifetime is easy
- since queue decouples when event is handled, event object needs to live longer
  than call to send it

- not particularly hard, but important to remember
- discuss options below

### feedback cycles

- all event systems have to worry about cycles:
- sender sends event, receiver responds to it by sending event which is in turn
  handled by original sender
- if not careful, can get stuck in loop
- with sync system, will quickly overflow stack and crash
- easy bug to notice and fix

- queues won't crash, which is "better"
- can actually support feedback loops correctly
- but often don't want feedback loop
- even if handled safely, not usually what you're trying to express
- keep eye on events and make sure don't see same series of them bouncing
  around
- good guideline is to avoid sending events in code that is responding to one

## Sample Code

- before start solving problems, let's create them
- set up synchronous audio api
- shows needed functionality

- have audio class
- likely to be singleton, though pains me to admit
- any code that wants to play sound calls into it:

    sync code

- good to note first off that code does have one good thing going for it
- its simple
- always best to start simple and make sure actually have problem before start
  layering on complexity to solve it

- let's see how does
- first problem was that audio api was synchronous
- indeed, now that we have code can see it is
- loads resource, and that blocks until sound is memory
- if not cached and has to hit disc, slow!
- playSound doesn't return until that's done so this would indeed block ui
  and cause hitching

### a deferred call

- let's go with simplest solution can come up with
- make playSound return quickly by deferring call
- first, define little struct that represents request to play sound

    struct

- audio will have instance of this
- when call play sound, just fill it out and set it as present

    store and set flag

- this way playSound returns instantly
- but do still actually have to play sound
- so add update method [see pattern] to audio

    update

- we'll call this at appropriate time in game loop
- [see game loop]
- so we can handle it taking little while
- possibly even run on different thread
- or make load api call itself async

- great, playsound is fast and ui is quick now

- but what happens if we try to play two sounds between calls to update?
- if already have one call deferred, can't handle deferring another one
- since we have multiple sound channels, entirely valid to play more than one

### buffer

- solution is to have audio have room for multiple deferred calls
- will have collection of sounds that have been requested but not yet processed
- playsound adds to collection
- update removes them all

- while algo prof may have told you otherwise, when you have bunch of items
  best way to store them is almost always just a flat contiguous array:

  1. no dynamic allocation [see obj pool]
  2. no memory overhead for pointers and bookkeeping
  3. super fast, cache friendly

- let's try that
- give audio fixed array of requests and keep track of how many are in it

    fields in class

- adding item is simple: just insert in next open slot

    playsound

- update now just processes all of them

    update all

- works ok, but assumes we can process every request in one frame
- probably true in our example, but chapter is event *queue*, not event buffer
- how would we handle not processing them all in one lump?
- need to be able to remove one at a time
- in other words, need an actual queue

- bunch of ways you can implement queues, this just one
- one of favorites

### ring buffer

- want this to work like queue, so update should process oldest request first
- that means first item in array

- i know what you're thinking
- this is why you learned how great linked lists are: super easy to remove node
  from list
- if we remove the first item from array, don't we have to shift all the others
  over to fill gap?
- isn't that super slow?

- this is where it gets clever
- not just dumb array: "ring buffer"
- called "ring" because instead of fixed array with beginning and end cells,
  treat it like a *circle* of cells

- right now, oldest request is always at index zero
- newest is at numRequests - 1
- as items are added, head of queue grows to right
- when items are removed, instead of *shifting* to left, we just move *tail*
  end

**TODO: illustration**

- need to add one field: head
- index of slot that *next* request will go in
- in other words, one past more recent request

- whenever add item, increase nummessage, and also bump head

    code

- now can implement remove
- all need to do is dec nummessages
- will implicitly move tail to the right
- no shifting required

- problem now is queue keeps crawling forward through array
- what happens when head reaches end of array?

- this is cool part: just wraps around
- when increment head, mod it with size
- when it goes off end, will automatically wrap around to beginning
- since tail has moved to right already, those cells are now free to be reused
- as long as nummessages stays below max, never go ouroboros and have head
  start eating into tail

- just need to update remove to correctly calculate tail pos
- needs to handle wrapping around to

    mod in remove

- there we go, a growable queue (up to max) with no alloc, constant time ops,
  and super cache friendly

### dupes

- solved first problem, move to next
- can queue up bunch of requests to play same sound
- will defer processing to update call
- but that still processes all of them individually
- if get bunch of requests to play same sound, will stack and sound busted

- easy to solve now
- we know which requests are already pending for next frame
- when enqueuing another request, just check for dupe first
- if have one, ignore other request

    check for dupe

- here, doing collapsing in called because simplest and keeps queue less full
- does put bit of processing in caller

- other option would be to collapse dupes in update
- to do that, every time dequeue request, also scan and zero out any identical
  ones

    collapse in update

### aggregates

- technique also lets us solve third problem
- if have a  bunch of requests for small bang sound enqueued, want to replace
  with call to single loud sound
- when see request for little bang, look for others
- if find some, bump up intensity of first and zero out others

### hopping threads

- last problem was threading
- with sync api, code to play sound was directly called by code that wanted to
  trigger sound
- whatever thread caller was on is thread api would run on
- often not what you want
- audio often run on separate thread

- now that have queue in place, in much better shape to handle that

- have three critical pieces already
- code for requesting sound is decoupled from code that plays sound
- have queue between two
- queue is encapsulated from rest of program

- all that's left is to make requesting a sound and processing the queue
  threadsafe
- would like to show concrete code here, but book avoids specific apis
- threading in particular hard to show in clear way
- in our case, basics are pretty simple
- requesting sound can acquire mutex
- processing sounds aquires same mutex
- if queue is empty, process waits on condition var until stuff to process

## Design Decisions

### what is queued?

- talked about few different uses of this pattern
- in addition to audio ex also os event queue and global event queue
- now draw attention to *what* being queued
- in audio ex, queuing "requests", or "messages"
- [or "commands" see pattern]
- think of queue as holding verbs, things you want to do in future

- in many uses, queue holds "events" things that happened in the past
- put in queue because want to *respond* to events in future
- depending problem solving, will do one or the other

* **queue of events**

    - basically refinement of observer pattern

    * more likely to allow multiple listeners
        - since queue describes things that happened, sender less likely to
          care who knows about it

    * more likely to be global
        - often use this as way to broadcast to any interested party
        - to allow max flex for "interested party", tends to be more central
          and more global

    * more common

* **queue of messages**

    - think of this as asynchronous api
    - outside code specifically wants action to happen, and often wants to
      control who does action
    - just doesn't control *when* action is done

    * more likely to be single listener

      - like in our example since queue has specific requests, tied to thing
        that will handle
      - doesn't have to be though

### how many can read from queue?

- in example, only audio engine processes queued items
- in os event loop, reading event is public api

* **one reader**

  - natural answer when queue is part of class's api
  - queue becomes implementation detail of class
  - from caller's perspective, reader isn't visible as distinct thing
  - more encapsulated, which is always good

* **many readers**

  - readers *receive* from queue, but don't *consume*
  - if have ten readers, all ten will see same item
  - don't parcel them out one at a time to each reader
  - is broadcast mechanism, not subdividing work
  - [worker queues are useful too, of course, just different pattern]

  - means events usually dropped if no readers
  - basic model is events are queued
  - when processed, send queued events to all listeners at that point
    time
  - if not listeners, event usually discarded
  - with explicit single listener, if reader doesn't process, usually
    stays in queue

  - ex: user input events are discarded if you don't register listener
    in your app
  - if you wait ten minutes while the user clicks around, then register
    mouse handler, don't immediately get a spew of events for every
    click already happened
  - single reader like watching something on tape
  - many reader like "tuning into a show already in progress"

  * usually need some kind of filtering

  - if have lot of events and lot of readers, can end up sending a *lot*
    of objects around
  - to be efficient, listeners can usually specify which "kind" of events
    they care about
  - think "mousemove" "mousedown" etc.
  - often as simple as a type id like that
  - can be more sophisticated

### how many can write to queue?

- flip side of above option
- pattern accomodates all configs: 1-1 1-many, many-1, many-many

- in example, requesting sound was public api, so any part of code could write
  request to queue

- in things like global event bus, anyone can send event

* **one writer**

  - closest to observer pattern
  - have one priveleged object that raises events
  - other objects can tune in
  - usually allow multiple readers

  - encapsulated, which is good

* **many writers**

  - if have multiple readers, basically "global" event bus
  - very similar to blackboard pattern in ai

## See Also

- if want something lighter-weight and faster, observer is close cousin

- commands often queued

- finite state machines often respond to stream of inputs
- if don't want them to respond synchronously, can give fsm queue to handle
  them. often call 'mailbox'
- state chapter has more on fsms
- when have bunch of fsms with mailboxes sending messages to each other, have
  re-invented actor model
- [erlang language based on this]

- like many patterns, often has a lot of names:

- "message queue" is established term for same pattern
  - usually used to talk about messaging at higher level between applications
    and not as much within them
  - lots of middleware for this: zeromq, rabbitmq, etc. if see "mq" in name
    now know what stands for

- "publish/subscribe"
  - very similar model
  - usually used to describe behavior of distributed systems, and not within
    single application

- queue itself is often special purpose example of object pool

---

random notes:

- dovetails really nicely with fsms, which want to receive stream of events

- 1-many, many-1, and many-to-many queues
  - 1-many: broadcast events. like observer pattern
    - more likely to use "events"
  - many-1: sink. think audio engine where anything can tell it to play a sound
    and it has one internal queue is uses to process those requests
    - more likely to use "messages"
  - many-many: like global event bus. anything can announce something
    interesting. anything can receive it. filtering becomes much more important.
    - see also blackboards

- talk about threading
  - with observer, message receiver always runs on *sender's* thread
  - with queue, can hop threads so receiver runs on thread it expects to run on
- with a global event queue, that means objects listening to queue can end up
  with infinite lifetime if not explicitly unregistered. talk about memory
  leak
- event priority to handle too many queued events
- talk about ordering. is it a strict queue or can things be delivered out of
  order? how does threading affect that?
- relate to job systems and worker threads?
- relate to event-based programming
- game loop is sort of like event loop where "event" is a tick
  - some simple games can actually use os event loop for game loop by updating
    game on "idle" events. not precise enough for real-time games
- sample code should walk through ring buffer

- channels in go
- talk a bit about message queues as middleware in business apps: zeromq, etc.
- queueing is helpful for networking.
  - gives meaningful serializable thing to send to server
  - see command pattern

- talk about how user input is usually queue
  - link to game loop which discusses pulling input versus push
  - also talk about in general about when to pull events off queues and how that
    ties to game loop

- have to think about what event/message represents. is it notice that thing
  happened, or request for thing to happen?
  - for example, do we sent "object collided" event and wait for audio engine
    to receive that and play sound, or do we sent "play collision sound"
    message?
  - depends on sender and intended receiver
  - if queue is global, event is usually what happened
  - if queue is specific to obj or system, usually message: request for action
  - if sender and intended receiver are in different domains (physics and audio)
    then usually event. if in same domain (audio component of actor and audio
    engine) to be command or message

- can we talk about how queues let you go from push to pull?

http://www.gamedev.net/topic/136778-event-queue-and-events-in-games/
http://gamedev.stackexchange.com/questions/14383/best-way-to-manage-in-game-events
http://gamedev.stackexchange.com/questions/7718/event-driven-communication-in-game-engine-yes-or-no?rq=1
