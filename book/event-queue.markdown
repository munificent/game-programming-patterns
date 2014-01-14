^title Event Queue
^section Decoupling Patterns

## Intent

*Buffer messages or events to decouple the sender from when and how it is processed.*

## Motivation

- pattern can be used to solve a bunch of similar but not identical problems
- to get most of out chapter, use example that jams all problems together
- then we'll knock em down one after another
- in reality, of course, won't be such perfect fit
- this way can see lots of facets of pattern

**todo: can fit discussion of event/message/command here? mention global event queue**

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
- in physics code, when objects collide trigger sound
- run into first problem: audio api takes a little while to locate sound
  resource and copy buffer of data to somewhere hardware can play it
- since api is synchronous, that blocks the physics engine!

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

**Bunch of paragraphs, maybe some subheaders**

## Design Decisions

**Few subheader questions, bullet list of answers for each

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

http://www.gamedev.net/topic/136778-event-queue-and-events-in-games/
http://gamedev.stackexchange.com/questions/14383/best-way-to-manage-in-game-events
http://gamedev.stackexchange.com/questions/7718/event-driven-communication-in-game-engine-yes-or-no?rq=1
