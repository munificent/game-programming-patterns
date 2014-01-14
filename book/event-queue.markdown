^title Event Queue
^section Decoupling Patterns


## Intent

*italicized sentence*

## Motivation

**Bunch of paragraphs, maybe some subheaders**

## The Pattern

**Paragraph**

## When to Use It

**Couple of paras**

## Keep in Mind

**A few subheaders, paragraph or two each**

## Sample Code

**Bunch of paragraphs, maybe some subheaders**

## Design Decisions

**Few subheader questions, bullet list of answers for each

## See Also

**Bullet list**

- if want something lighter-weight and faster, observer is close cousin

---

random notes:

- motivations
  - decouple when event happens from when respond to it
  - make responding to event not block sender
  - respond to event on different thread
  - batch or coalesce events together
  - prioritize and drop events
  - respond to events at appropriate time

- have to think about lifetime of event object itself

- need to ensure that when event is received later, world state is still
  meaningful and useful for handler
  - for example, send "monster died" event
  - achievement system wants to track it
  - gets event five frames later
  - by then, monster object is deleted, so achievement system can't tell what
    kind of monster it was or anything else about it

- observer will crash on cycles. queue can handle it (though can get stuck in
  feedback loop)

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
- coalescing or batching events together
- event priority to handle too many queued events
- talk about ordering. is it a strict queue or can things be delivered out of
  order? how does threading affect that?
- relate to job systems and worker threads?
- relate to event-based programming
- game loop is sort of like event loop where "event" is a tick
  - some simple games can actually use os event loop for game loop by updating
    game on "idle" events. not precise enough for real-time games
- publish/subscribe
  - very similar model
  - usually used to describe behavior of distributed systems, and not within
    single application
- sample code should walk through ring buffer
- contrast with actor model: each actor has queue, but event/message is sent
  to specific actor and not broadcast
  - mention erlang
  - maybe talk about single global queue, then discuss more localized queues
  - eventually consider each game entity has its own queue
  - ta-da actor pattern

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
