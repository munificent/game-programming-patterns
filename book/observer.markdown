^title Observer
^section Design Patterns Revisited

*Sorry, this chapter hasn't been written yet!*

- intro
    - outside of games, observer one of most successful design patterns in existence.
    - key piece of mvc which is dominant paradigm for client applications.
    - now that web is becoming more app-like, becoming critical there.
    - so important, java put in core lib, c# added first-class language support for it.
    - often called "events" or "notifications".
    - specific use of it popular now called "data binding".

## achievements

- one of patterns part of collective unconscious of devs, so feels weird to
  explain
- walk through example
- say you're adding an achievement sys to game
- lots of different achievements: number of enemies killed, first time you
  kill certain enemy, use item, etc.
- tricky to implement cleanly since so many parts of game behavior may trigger
  achievement
- really don't want tendrils of achievement sys wound through entire codebase
- ultimately, achievement triggered by something "happening" in game engine.
- may require other data analysis to tell if achievement was unlocked, but
  always some final bit to trigger
- for example, "killed 100 monkey demon" achievement needs to track how many
  monkey demons you've killed. but achievement will always be triggered when
  100th one dies
- would be cool if game engine could just holler, "hey, killed something!"
- doesn't care if anything listens, doesn't even know. just sends general
  notification.
- achievement system could then watch this notification.
- when enemy is killed, it gets notified and it can look through bookkeeping
  data. "oh, that was demon monkey. how many have we killed so far? oh 99, this
  is it!"
- this way, game engine totally unaware of achievement system. could remove it
  completely without changing game code.
- can add new achievements without touching rest of engine (assuming doesn't
  need to be triggered by new *kind* of change.)

## observer pattern

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

## sounds great

- sounds awesome, and pattern certainly successful outside of games.
- seems to be mix reception in games.
- part of this is mvc not as popular in games.
- large projects must be broken into relatively independent pieces if going to
  be developed by large team. (conway's law)
- mvc worked well in business. people with ux expertise often not same as people
  with data skills, or server skills.
- skill breakdown among game devs is different. more domain: render, physics,
  ai, audio. part of why component pattern so popular: follows inclinations of team.

- of course, different parts of game codebase need to communicate too
- why not observer? some games do use it, but not that many.
- unfamiliarity one reason: tend to do things other people do.
- asked around once, and heard a few concrete reasons:

- too heavyweight
  - when a needs to talk to b, observer pattern adds lot of overhead
- too slow part two
  - worry that don't know when observer will receive notification after subject
    sends it
  - does it just go in some queue or something?
- too much dynamic allocation
  - subject needs list of observers, which can grow and shrink over time.
  - game devs hate allocation
- too decoupled
  - "action at a distance"
  - hard to tell who will actually get notified
  - hard to trace flow of stuff through codebase

- will try to have answer for first three. fourth is real concern we'll talk
  about
- of course, not trying to say panacea. will talk a bit about why might not
  want to use it

### too heavyweight

- talked to some people who heard of pattern but don't know details.
- impresision was it was too "heavyweight" or "slow".
- direct calls, even if adding coupling would be better.
- somehow notifying observers without coupling them seemed like magic, and magic
  is synomym for "weird code that will be slow and hard to debug"
- but since seen actual details above, obviously not that heavyweight.
- just walking a list and calling virtual functions
- tiny bit of overhead compared to static method call, but pretty tiny.
- just virtual method calls, no magic. no rpcs, serializations, etc.

### too slow

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

### too much dynamic alloc

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

## implementation tricks

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

## remaining problems

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