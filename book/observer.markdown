^title Observer
^section Design Patterns Revisited

The Observer
pattern is one of the most wildly successful programming patterns in existence.
It was <span name="devised">devised</span> in tandem with the larger Model-View-Controller architecture
which is structure underlying countless applications.

<aside name="devised">

Like so many things in software, it was invented by Smalltalkers in the seventies.
Lispers probably came up with in the sixties and didn't bother writing it down.

</aside>

As web pages have started to become web applications, it's becoming widely used there too. It's so important that Java put it in its core library ([`java.util.Observer`](http://docs.oracle.com/javase/7/docs/api/java/util/Observer.html)) and C# baked them right into the *language* (the [`event`](http://msdn.microsoft.com/en-us/library/8627sbea.aspx) keyword).

But the world of game development can be strangely cloistered at times. Just in case you haven't left the abbey in a while, let me walk you through a motivating example.

## Achievement Unlocked

Say you're adding an achievements system to your game. These will be dozens of different badges players can earn for completing certain specific milestones, like "Kill 100 Monkey Demons", "Fire a Shotgun in the Dark", "Fall of a Bridge", or "Chug Ten Cans of Nutri-Beer".

This is tricky to implement cleanly since you have such a wide range of achievements that can be unlocked by varying but very specific behaviors. You really don't want tendrils of your achievement system twining their way through every dark corner of your codebase. Sure, "Fall of a Bridge" is somehow tied to the <span name="physics">physics engine</span>, but do you really want to see a call to `unlockFallOffBridge()` right in the middle of the linear algebra in your collision resolution algorithm?

<aside name="physics">

This is a rhetorical question. No self-respecting physics programmer would ever let you sully their beautiful mathematics with something as pedestrian as *gameplay*.

</aside>

What we'd like, as always, is to have all the code concerned with one aspect of the game nicely lumped in one place. The challenge is that achievements are triggered by a bunch of different parts of the game. How can that work without coupling the achievement code to all of these?

That's what the observer pattern is for. It's a pattern for letting one piece of code trigger a notification that something happened *without actually caring who gets the notification*. For example, you've got some physics code that handles gravity and tracking which bodies are relaxing calmy on nice flat surfaces and which are plummeting towards sure demise. To handle the "Fall of a Bridge" achievement, you could just jam the code right in there, but that's a mess. Instead, the code that handles entity falling just does:

**TODO: make real code**

    bool wasOnSurface = body->isOnSurface();
    body->accelerate(GRAVITY);
    body->update();
    if (wasOnSurface && !body->isOnSurface())
    {
      notify(EVENT_START_FALL, body);
    }

All it does is say, "Uh, I don't know if anyone cares, but this thing just fell. Do with that as you will."

The little achievement engine then registers itself to receive that notification. Whenever the physics code sends that notification, the achievement code receives it. It then checks to see if the body is for the player. If so, it checks its own bookkeeping data to see what the less-than-graceful hero happened to be standing on prior to his encounter with classical mechanics. If it's a bridge, it unlocks the proper achievement and sets of the fireworks and fanfare, all with no involvement from the physics code.

In fact, you can change the set of achievements or tear out the entire achievement system without touching a line of gameplay code. It will still send out its notifications, oblivious to the fact that nothing may be receiving them anymore.

## How it Works

If you don't know the mechanics of the actual pattern, you can probably guess
them just from the description, but do keep things easy on you, I'll just walk
through it quickly.

We'll start with the nosy class that wants to know when some other object does something interesting. It does this by implementing this:

<span name="signature"></span>

^code observer

<aside name="signature">

The actual signature of this method is totally up to you. That's why this is the Observer *pattern* and not the "Observer ready-made chunk of code you can just copy and paste into your game". Usually, the notify method takes the object that's sending the notification and some other generic "data" parameter that you can use to stuff whatever you want into.

If you're using a language with generics or templates, you'll probably use them here, but it's also perfectly fine to have some more specifically tailored to your use case. Here, I'm just hardcoding it to take a game entity, and some arbitrary enum that describes what happened to the entity.

</aside>

A concrete class becomes an observer by implementing this interface. In our example, it's the achievement system, so you'd have something like this:

^code achievement-observer

It receives a notification by having its `onNotify()` method called by the object that's doing something interesting. In Gang of Four parlance, that's called the
"Subject". In our example, it's the physics engine.

The subject has two jobs. First, it holds the list of observers that are waiting oh-so-patiently for a missive from it:

<span name="stl"></span>

^code physics-list

<aside name="stl">

In real code, you would invariably use a dynamically-sized list or vector type here instead of a dumb array. I'm sticking with the basics here to keep things simple for people coming from other languages that don't know C++'s standard library.

</aside>

The important bit is that the subject exposes a *public* API for modifying that list:

^code physics-register

This way, outside code is responsible for adding *itself* to the list of observers that the subject will notify. This way, the subject can communicate to the observers without its code being coupled to them. That's the clever part about this pattern.

Then, the last piece is actually sending the notification. The basic method for that is simple:

^code physics-notify

When the physics engine does something noteworthy, it calls that. It will then walk the observer list and give them all the heads up. Pretty simple, right? Just one class that maintains a list of pointers to instances of some interface.

It's hard to believe that something so straightforward is the cornerstone of the architecture of thousands of programs and app frameworks.

## What's the Catch?

This sounds pretty swell, and the pattern is certainly successful. But within
games, it seems to have gotten a mixed reception. Part of this is the observer
pattern usually rides along with MVC, and that isn't very popular in games.

Any large software project has to be broken into relatively independent pieces
if it's going to be developed by a large team. People can't get work done if
they're constantly stepping on each other's toes. Melvin Conway tells us:

<span name="conway"></span>

> Any organization that designs a system ... will inevitably produce a design whose structure is a copy of the organization's communication structure.

<aside name="conway">

While he didn't have the hubris to call it "Conway's Law", he coined this in his well-known paper "[How Do Committees Invent?](http://www.melconway.com/research/committees.html)". It's well worth reading.
</aside>

This has been enshrined as "Conway's Law" and most people consider it a tongue-in-cheek observation of the foibles of office politics. But Conway was serious here, and it is quite true: if two parts of a program need to interact, the people writing those two parts will probably have to talk to each other.

So, if you're trying to figure out how to break your huge programming project down into small pieces of work you can assign to people, one easy way to do it is to ask, "What are my different people good at?"

In enterprise software, user interface programming skill is relatively uncommon. There are many talented "band-end" engineers who are actively hostile to doing UI work. Meanwhile, the engineers who *do* have the rare gift of making software that's joyful to use are usually discinclined to work on code that's deep in the bowels of the program and far removed from end users.

It should come as no suprise that the model-view-controller paradigm is popular there. The architecture directly reflects the separation between front-end and back-end programmers. The observer pattern is a particularly good fit because the communication is mostly one way: I don't know about you, but most of the back-end programmers I've met don't even want to know what the UI folks are doing.

The skill breakdown in games is a little more varied. Because games, especially top-tier ones, are quite technically challenging in a bunch of a different ways, large game teams have a slew of <span name="specialists">specialists</span>. You've got people who only do graphics and shaders, others who live and breathe physics, maybe a dedicated audio coder, AI engineers, and then the catch-all "gameplay" programmer.

<aside name="specialists">

Looking at things this way, it's no surprise the <a href="component.html" class="pattern">Component</a> is so popular: it slices up your codebase directly along the lines of your org chart, just like Conway predicted.

</aside>

Without a clear front-end/back-end separation, cramming MVC into a game is hard, so the observer pattern hasn't gotten a free ride there. When I ask other game programmers if they use it, some simply haven't heard of it. Some do use it, but I also hear the same couple of explanations for why they avoid it:

 *  "It's too heavyweight." When some piece of code needs to interact with
    another, they feel the observer pattern adds a lot of overhead, both
    conceptually and computationally.

 *  "It's too laggy." Since "observer" often gets mixed up with "events" and
    "messages" and other asynchronous communication systems, some people worry
    that using the observer pattern will add unacceptable delays between when
    a notification is sent and when it's received.

 *  "It does too much dynamic allocation." Games are one of the few domains
    where programmers still focus heavily on memory allocation. Since an object
    sending notifications may need to send it to several listeners, they worry
    about the memory churn of dynamically allocating that list.

 *  "It's *too* decoupled." While avoiding coupling is the *goal* here, some
    feel that it makes a program too hard to reason about. When you're trying
    to trace some behavior as it flows through the system, direct call chains
    are easy to follow. If everything is wired up at runtime using observers,
    the communication structure of the game is an emergent property of how
    which listeners were registered to which objects. That can make it much
    harder to figure out why something isn't working just be looking at the
    code.

My goal with this chapter is to try to have answers for at least the first
three concerns here. That's not to say they aren't valid, but I think they can
often be addressed. The fourth one is a real challenge, and we'll talk about
that some too.

## are too heavyweight?

- talked to some people who heard of pattern but don't know details.
- impresision was it was too "heavyweight" or "slow".
- direct calls, even if adding coupling would be better.
- somehow notifying observers without coupling them seemed like magic, and magic
  is synomym for "weird code that will be slow and hard to debug"
- here's actual pattern:

### actual observer pattern

- observer pattern is class pattern for doing this
- have a "subject" -- thing doing something noteworthy. in example, this is
  piece of game engine that handles enemy death.
- have one or more "observers" that want to know when subject does something.
- to do this, subject maintains list of observers.
- they have basic "notify" method that can be called to be notified.
- observers can add and remove themselves to subject's list.
- when subject does something interesting, walks list and notifies all observers.
- doesn't care if list is empty. doesn't know concrete types of observers.
- if observer doesn't register itself, subject doesn't care.
- all subject knows is that it *can* be observed, and minimal machinery to
  notify.

- just walking a list and calling virtual functions
- tiny bit of overhead compared to static method call, but pretty tiny.
- just virtual method calls, no magic. no rpcs, serializations, etc.

## are too slow?

- similar to previous section. concern above was "slow" in sense of performance
- doing too much work.
- other fear i've heard is too "slow" in sense that it's doing something
  asynchronous
- may be little total work, but maybe there's some sort of queing or async
- notifications have to be put in pipeline and may not actually get pulled off
- and handled until much later
- don't want something sluggish and laggy
- impression comes because pattern is synonymous with "event" and that is also
  used to describe things like message queue and event systems that *are*
  queued and async

- [why patterns are really important. need to have crystal clear names with
  precise semantics associated with them.]

- now can see not case here. no queueing, no async, no pipelines. just immediate
  calls.

- does mean should be aware that subject will by *synchronously* calling
  notifications
- will be on same thread
- if notification handler is slow, will block subject
- sounds scary
- important to know, but not end of world
- just need to be aware of it and make sure have good discipline about event
  handlers finishing quickly
- do minimal work and get out of way
- if have to do something more expensive, handler just enqueues work to be
  handled later or on other thread
- this is just well-understood part of ui prog
- not rocket science, just takes a little care
- if really worried about blocking in handlers, then queueing *is* what you
  want. see message queue

## too much dynamic alloc?

- most other progs don't sweat this
- but game devs care deeply about perf and memory
- much like embedded progs, game may have limited memory, need to be very
  stable, and run for a long time
- lot of dynamic allocation is slow, and can cause fragmentation which can over
  time cause alloc failure.
- gradually less of issue as more games dev in gc languages like java, but
  valid concern
- but we're smart, we can handle. here's two refinements to pattern that avoid
  dynamic alloc. add some complexity though. simpler first.

### linked observers

- basic idea of pattern is have one subject that needs to maintain list of
  observers so it can notify them
- in most impls, subject *has* list of them: separate array or vector of
  pointers to observers
- but observer is itself a class
- in simple implementations, it's pure virtual. just interface.
- if willing to add a bit of state too, then can make observers themselves
  contain list.
- instead of subject maintaining separate list, observers themselves will be
  linked list of nodes.

... code ...

- does have one important limitation: observer can only observer one subject.
- since it *is* node and links to next observer in list, can only be part of
  one list
- in practice, often good enough. while subject often has more than one observer,
  observer usually only observes one thing

### binding pool

- what if don't want that limitation, but still want to avoid dynamic alloc?
- can we have an observer sys that allows subject to have multiple observers
  and observer to observe multiple subjects?
- yes, we can!
- bit complex, but walk through it
- two key ideas:
- 1. since doing many-to-many mapping, arbitrary possible pairs of bindings
  between subject and obs. so will define object represents that
- binding defines one pair of observer and subject
- subject maintains list of bindings for every obj observing it
- earlier, observer themselves were list, but now level of indirection
- good because now we can have multiple bindings lists that contain same
  observer
- this gives us way to have many to many mapping
- but seems like we still need dynamic alloc for bindings
- key idea two: use an object pool
- all bindings are stored in reusable pool
- allocates its memory up front and reuses binding objects
- clever bit is since bindings are linked list, can reuse that machinery for
  free list

## any remaining problems?

- think those are pretty cool techniques to make observers more usable in
  confines of games
- still not panacea
- two problems worth thinking about, one technical and one more abstract

### destruction

- sample code up there side-stepped important issue: what happens when subject
  or observer gets deleted?
- if naively delete observer, subject still has dangling pointer to it. if it
  notifies, bad
- destroying subject is easier since observer doesn't refer to it in most impl
- even that can be problematic: observer may still expect to be notified. may
  want to know when it gets destroyed
- can solve this couple ways:
- simplest is to do nothing: its observer's responsibility to unregister itself
  from its subjects before being deleted.
- since most observers already have some kind of reference to subject anyway,
  usually simple to add a "removeListener" call in dtor.
- likewise, subject can send one final dying breath notification when being
  deleted in case observers want to know

- if want something safer, can make observer automatically unregister itself
  from subjects when destroyed. does require observer to maintain poitners to
  all subjects its watching, which adds complexity.

- may think not an issue in gc languages. nothing gets destroyed manually, so
  no chance of notifying dead observer
- kind of true: won't notify freed memory, but can cause other problem
- called "lapsed listener"
- even though observer object itself will not be explicitly deleted and will
  stay in memory since subject is referring to it, doesn't mean it's still
  *useful*.
- consider case where observer is some UI screen.
- subject is main character in game
- showing some data about character: health and stuff
- player exits screen
- screen doesn't remember to unregister listener
- at this point screen is gone, but subject still has ref to it
- every time character health changes, sends notification
- "zombie" screen receives notification, updates widgets and does other stuff
  all the while totally pointless
- lesson here is have to be disciplined about registration

### what's going on

- other problem with observer pattern comes directly from problem it solves:
  gets rid of coupling
- goal is to keep pieces of codebase decoupled
- does that too well
- if you actually need to trace behavior as it flows through codebase, can be
  very hard
- instead of seeing "ah, yes, A calls B here which then calls C"
- have "A notifies". "uh, let me see if i can find what registers itself to
  observe that. hmm, maybe B? i'll have to check in the debugger and see if it
  actually is registered"
- instead of being able to statically tell how code is wired together,
  communication structure is emergent property of how things happened to be
  imperatively registered at runtime
- have worked with codebases that use observer very heavily for critical
  behavior and can be nightmare trying to find out what's going on (or failing
  to)

- one rule of thumb is that if it's critical that a subject and observer to be
  wired up for program to function correctly, observer is probably wrong fit
- observer works best when subject really doesn't care if it is observed
- lets you layer "secondary" behavior on top of core shell
- if that secondary behavior is critical, more explicit coupling may be right
  answer

## functions?

- in 90s when gof came out and oop was hotness, observer pattern made lot of
  sense
- today, oop is still huge, but fp also increasingly popular
- programmers increasingly comfortable with functions, higher-order functions,
  etc
- leads to new ways to approach this
- one facet of pattern rubs people wrong today is that observer is *class*
- have to implement entire interface just to receive notification
- feels really heavyweight. is!
- makes it impossible to have one class that can observe different subjects with
  different methods to handle it
- more modern approach is to have observer just be *fn* or reference to method
- in languages with first class fns, and especially closures, much more common
  way to do observers
- events in c# register fns, not objects
- event listeners in js can be obects implementing informat eventlistener
  interface, but can also just be fns. almost always use latter
- can even do in c++ with pointers to member fns

## reactive?

- if write big program using lots of observers, one thing realize is that
  observer code is often really dumb boilerplate:

    - receives event that some state changes
    - imperatively modify some piece of ui or other derived state to reflect new
      state

- after a while, really want to automate that
- people been trying to make that happen for *long* time: "dataflow programming"
- frp
- lately, more humble approach "data binding" starting to actually work
- more declarative approach
- you say, "this piece of data or ui is derived from this other data using
  this transformation"
- it then automatically updates the derived stuff whenever original stuff changes
- don't have to write imperative code or wire things up
- starting to get some traction
- when it works well, great
- when it doesn't work well, really really awkward
- general problem with declarative systems compared to imperative:
- make 90% of cases easier, make 10% of remaining cases a eye-stabbing nightmare
- part of appeal of observer pattern for me is simplicity and easy out here
- sure, not as advanced as data binding, but when need to dip into imperative
  code or manually wiring stuff up, doesn't get in way

---

http://www.gotw.ca/gotw/083.htm
http://molecularmusings.wordpress.com/2011/09/19/generic-type-safe-delegates-and-events-in-c/