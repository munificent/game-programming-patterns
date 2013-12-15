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

## too heavyweight

- talked to some people who heard of pattern but don't know details.
- impresision was it was too "heavyweight" or "slow".
- direct calls, even if adding coupling would be better.
- somehow notifying observers without coupling them seemed like magic, and magic
  is synomym for "weird code that will be slow and hard to debug"
- but since seen actual details above, obviously not that heavyweight.
- just walking a list and calling virtual functions
- tiny bit of overhead compared to static method call, but pretty tiny.
- just virtual method calls, no magic. no rpcs, serializations, etc.

## too slow

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

## too much dynamic alloc

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

## linked observers

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

## binding pool





---

random notes:

originally intended to write chapter to address two complaint with observer:

1. it's unpredictable because you don't know when a notification happens. can
   be slow.

here, just wanted to show normal implementation is synchronous and
deterministic. doesn't have to pump message loop, use threads, etc.

2. it requires dynamic allocation when adding.

i *thought* i had a solution for this that cleverly threaded the list of
observers through the observers themselves. but that has the unpleasant
side effect that a given observer can only observe one object.

maybe that's ok? talk about specific 1-many observable/observer design.

another option is to talk about using an object pool for the observer/obserable
pair objects.

also discuss using generic callbacks instead of having to implement interface.

talk about reactive programming.

useful for things where observable doesn't care if being observed. rule of thumb
is if subject needs to be observed, then shouldn't use pattern.

one problem is you can get too many updates: make five small changes to subject
and each one triggers wave of notifications. since it's synchronous, can't
batch them. see message queue.

what happens when observer is deleted?
- subject still has reference to it.
- option: do nothing. get dangling pointer. observer is responsible for
  unregistering itself from everything it's observing before going away.
- option: use some kind of smart pointer.
- option: observer has reference to subject and automatically unregisters self.
  harder if observer can observe multiple subjects.
- for book, should probably just note problem and go with first solution.



http://www.gotw.ca/gotw/083.htm
http://molecularmusings.wordpress.com/2011/09/19/generic-type-safe-delegates-and-events-in-c/