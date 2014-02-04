^title Event Queue
^section Decoupling Patterns

## Intent

*Decouple when a message or event is sent from when it is received.*

## Motivation

Unless you live under one of the few rocks that still lacks Internet access, you've probably already heard of an <span name="message">"event queue"</span>. If not, maybe "message queue", or "event loop", or "message pump" rings a bell. To refresh your memory, let's walk through a couple of common manifestations of the pattern.

<aside name="message">

For most of the chapter, I use "event" and "message" interchangeably. In the few places where the distinction matters, it will be obvious. Otherwise, just read both of those words as "thing you stick in a queue".

</aside>

### GUI event loops

If you've ever done any <span name="event-driven">user interface</span> programming, then you're well acquainted with the *events*. Every time the user interacts with your program -- clicks a button, pulls down a menu, or presses a key -- the operating system generates an event. It throws this object at your app, and your job is to grab it and hook it up to some interesting behavior.

<aside name="event-driven">

This application style is so common, it's considered a paradigm: <a href="http://en.wikipedia.org/wiki/Event-driven_programming">*event-driven programming*</a>.

</aside>

You rarely write this plumbing code yourself, but the way this works is that somewhere deep in the bowels of your code is an *event loop*. It looks roughly like this:

^code event-loop

The call to `getNextEvent()` pulls the next unprocessed user input into your app. You route it to an event handler and, like magic, your application comes to life. The interesting part is that the application *pulls* in the event when *it* wants it. The operating system doesn't just immediately <span name="interrupt">jump</span> to some code in your app when the user pokes at a peripheral.

<aside name="interrupt">

In constrast, *interrupts* from the operating system *do* work like that. When an interrupt happens, the OS stops whatever your app was doing and forces it to jump to an interrupt handler. This abruptness is why interrupts are so hard to work with.

</aside>

That means when user input comes in, it needs to go somewhere so that the operating system doesn't lose it between when the user does something and when your app gets around to calling `getNextEvent()`. That "somewhere" is a *queue*.

When user input comes in, the OS adds it to a queue of unprocessed events. When you call `getNextEvent()`, it pulls the oldest event off the queue and hands it to your application. (If there are no events in the queue, it usually blocks and waits until one comes in.)

### Central event bus

If the above section is news to you, don't sweat it. Most <span name="game-loop">games</span> aren't event-driven like this, but it is common for a game to have it's own event queue as the backbone of its nervous system. You'll often hear "central" or "global" or "main" used to describe it. It's used for high level communication between game systems that want to stay decoupled.

<aside name="game-loop">

If you want to *why* they aren't, crack open the <a href="game-loop.html" class="pattern">Game Loop</a> chapter.

</aside>

Say your game has a <span name="tutorial">tutorial</span> system to show help popups cued off of specific in-game events. For example, the first time the player vanquishes a foul beastie, you want to show a little balloon that says, "Press X to grab the loot!"

<aside name="tutorial">

Tutorial systems are a pain to implement gracefully and most players will spend only a fraction of their time using it, so it feels like they aren't worth the effort. But that fraction where they *are* using the tutorial can be invaluable for easing the player into your game.

</aside>

Your gameplay and combat code is likely complex enough as it is. The last thing you want to do is stuff a bunch of checks for triggering tutorials in there. Instead, what some games do is have a central event queue. Any game system can throw an event on it, so the combat code can add an "enemy died" event every time you slay a foe.

<span name="blackboard">Likewise</span>, any game system can *receive* events from the queue. The tutorial engine registers itself with the queue and indicates it wants to receive "enemy died" events. This way, knowledge of an enemy dying makes its way from the combat system over to the tutorial engine without the two being directly aware of each other.

<aside name="blackboard">

This model where you have a shared space where entities can post information and get notified when others post is similar to <a href="http://en.wikipedia.org/wiki/Blackboard_system">blackboard systems</a> in the AI field.

</aside>

I thought about using this as the example for the rest of the chapter, but I'm not generally a fan of global systems. It is a common technique, but I don't want you to think that event queues *have* to be global. Instead, we'll use something more scoped.

### Say what?

Speaking of often underappreciated features, let's add sound to our game. Humans are visual creatures, but hearing is deeply connected to our emotions and our sense of physical space. The right simulated echo can make a black screen feel like an enormous cavern, and a well-timed violin adagio can make your heartstrings hum in sympathetic resonance.

To get our game wound for sound, we'll start with the simplest possible approach and see how it goes. We add a little <span name="singleton">"audio engine"</span> to the game. It has an API for playing a sound with some given identifier at a given volume:

<aside name="singleton">

While I almost always shy away from the <a href="singleton.html" class="gof-pattern">Singleton</a> pattern, this is one of the places where it make make sense. I'm doing something simpler and just making the API static.

</aside>

^code sync-api

Its implementation is responsible for loading the appropriate sound resource, finding an available channel to play it on, and starting it up. This chapter isn't about some hardware's actual audio API, so I'll conjure up an imaginary one. Using it, we write our method like so:

^code sync-impl

We check that in, create a few sound files, and then start sprinkling `playSound()` calls throughout our codebase like some magical audio fairy. In our UI code, when the selected menu item changes, we play a little bloop:

^code menu-bloop

This would be a short chapter if that API worked without any problems, so let's see where it goes awry. The first thing we notice is that sometimes when you switch menu items, the whole screen freezes for a few frames. We've hit our first issue:

* **Problem 1: The API blocks the caller until the audio engine has completely processed the request.**

Our `playSound()` method is *synchronous* -- it doesn't return back to the caller until noise is coming out of the speakers. If a sound file has be be loaded off the disc, that may be a while.

Ignoring that for now, we move on. In the AI code for an enemy, we add a call to let out a wail of anguish whenever it takes damage from the player. Nothing warms a gamer's heart quite like inflicting simulated pain on a virtual living being.

It works, but occasionally causes a weird issue. Sometimes when the hero does a mighty attack, it hits two enemies in the exact same frame. In response to that, we try to play the wail sound twice simultaneously. <span name="hatsworth">If</span> you know anything about audio, you know playing multiple sounds stacks their waveforms on top of each other. When those are the *same* waveform, it sounds like *one* sound played *twice as loud*.

<aside name="hatsworth">

I ran into this exact issue working on <a href="http://en.wikipedia.org/wiki/Henry_Hatsworth_in_the_Puzzling_Adventure">Henry Hatsworth in the Puzzling Adventure</a>. My solution there is virtually identical to what we'll be covering here.

</aside>

There's a similar problem in big boss fights where we have piles of minions running around and all sorts of action going on. The hardware can only play so many sounds at one time. When we try to go over that limit, sounds get dropped or audibly cut off.

To fix that, we need to look at the entire *set* of sounds we're playing and aggregate and prioritize them. Unfortunately, our audio API handles each `playSound()` call independently and forgets about the previous ones as soon as it returns. It only sees sounds through a pinhole, one request at a time.

* **Problem 2: Requests cannot be analyzed or aggregated.**

These problems seem like mere annoyances once the real deal-breaker falls in our lap. By this point, we've got calls to our audio API strewn throughout the codebase in lots of different game systems. But our game engine is running on modern multi-core hardware. To take advantage of those cores, we those game systems on different threads -- rendering on one, AI on another, etc.

Since our API is synchronous, it runs on the *caller's* thread. When we call it from different game systems, we're hitting our API concurrently from a number of threads. Look at that sample code. See any thread synchronization? Me either.

All hell breaks lose. This is particularly egregious because we intended to have a *separate* thread for audio. So it's just sitting there totally idle now.

* **Problem 3: Requests are processed on the wrong threads.**

### Leave a message

The common theme to these problems is that a call to `playSound()` is interpreted by the audio engine to mean, "Drop everything and play the sound right now!" That immediacy is a problem for the audio implementation.

The game system calls `playSound()` at *its* convenience, but not necessarily when it's convenient for the audio engine to *process* that request. To fix that, we want to decouple *receiving* a request from *processing* it. Just like a GUI event system, the place where messages hang out between when the request came in and when the audio engine processes it is a queue.

## The Pattern

A **queue** stores a series of **notifications or requests** in last-in, first-out order. Sending a notification simply **enqueues the request and returns**. The request processor then **processes items from the queue later** at an appropriate time.

Requests can be **handled directly**, or **routed to interested parties**. This **decouples the sender from the receiever** both **statically** and **in time**.

---

## When to Use It

I sometimes see people reach for event queues when they want to decouple the event sender from an event receiver. Queues definitely do this, but if *all* you need to do is decouple *who* is sending from *who* is receiving, it may be overkill. The <span name="simple">simple</span> <a href="observer.html" class="pattern">Observer</a> and <a href="command.html">Command</a> patterns or other abstract interfaces are enough to handle that.

<aside name="simple">

I feel like I mention this in practically every chapter, but it's worth emphasizing. As a programmer, your most precious resource is often *simplicity*. As you work on your game, you'll be adding features, adding code, adding *stuff*. That almost always comes at the expense of simplicity.

Few things burn time as much as just trying to cope with your program's complexity, so hold on to as much simplicity as you can while you can.

</aside>

Queues do more than that. There are a bunch of ways to express it, but I like to think of it in terms of pushing and pulling. Often, you have some piece of code that wants another piece of code to do some work. The most natural way for the former to express that is to explicitly initiate the request. It *pushes* that request to the code that will do the work.

At the same time, the most natural way for the worker to process that request is to *pull* in a request at some appropriate later time in its run cycle. When you have a push model and one side and a pull model on the other, you need some mechanism to handle that slippage. That's what a queue can do that other decoupling interfaces don't. They let you decouple not just *who*, but *when*.

This is a powerful difference, but it doesn't come for free. Queues are more complex and use more memory than synchronous messaging. You have to be careful to ensure the queue doesn't overflow or grow in an unbounded way.

Queues give the code processing items more control: they can aggregate or discard requsts and generally have a more global view of the set of outstanding requests. But they take control from the sender. All it can do is throw it on the queue and hope something good will come of it.

In particular, queues are a poor fit when the sender requires a response. Not only will the response not be available immediately, it can't even guarantee it will come at all. If the queue is full or the code processing it decides to ignore it, the request may fall on the floor. Queues work best when you can fire and forget.

You *can* make responses work with a queue. Typically the response is asynchronous too. This is common when you have a "global" event queue that anything can push to and pull from. Here, A can send an asynchronous request that gets consumed by B. B then responds by throwing another thing on the queue which gets consumed in return by A.

<span name="distributed">As</span> you can imagine orchestrating this kind of communication is complex. Do you really want to turn the internals of your game into a distributed programming problem?

<aside name="distributed">

Of course, if your game uses networking, it already *is* a distributed programming problem, but that doesn't mean you should make things even harder on yourself.

</aside>

## Keep in Mind

### A "global" event queue is still a global

One common use of this pattern is for a sort of Grand Central Station that any part of the game can send to and receive from. It's a powerful piece of infrastructure. But "powerful" doesn't always mean "good".

It took a while, but most of us finally learned the hard way that globals are bad. When you have a piece of state that any part of the program can poke at, all sorts of subtle interdependencies can creep in. While this pattern wraps that state up in a nice managed data structure, it's still effectively a global, with all of the danger that that implies.

### The state of the world may change between when a message is sent and received

Say we've got this event queue for our game systems to talk to each other. The AI for an entity in the game posts a "entity died" event to that queue when some virtual minion shuffles off its mortal coil. That hangs out in the queue for who knows how many frames until it eventually works its way to the front and gets processed.

Meanwhile, we've got an experience system that wants to track the heroine's bodycount and reward her for her grisly efficiency. It receives these "entity died" events so it can determine what kind of entity was slain and perhaps how difficult the kill was so it can calculate an appropriate reward.

To do that determination requires looking at various pieces of state in the world. At the very least, the experience system will want to get a hold of the entity that died to see what sort it was. It may want to inspect its surroundings to see what other obstacles or entities were nearby.

But if the event isn't received until later, that stuff may be gone. The entity may have been deallocated, and other nearby objects may have wandered off.

When you receive an event, you have to be careful not to make too many assumptions about how the *current* state of the world lines up with how the world was *when the event was raised*.

The practical consequence of this is that events and messages in a queued system tend to be a little more data heavy than in a synchronous style. With the latter, the notification can just be "hey, something happened" and the receiver can then look around to see what actually occurred and its details. With a queue, some of that data needs to be captured at the moment the event is sent and stuffed into the event object itself as a snapshot of that ephemeral moment in the past.

### You can get stuck in feedback loops

All event and message systems have to worry about cycles:

1. A sends an event.
2. B receives it and responds by sending an event.
3. That event happens to be one that A cares about, so it receives it.
4. In response, it sends an event...
5. Go to 2.

Unless your event senders and receivers are strictly partitioned in a way that prevents communication cycles, the above can happen. If your messaging system is *synchronous*, you'll find about a cycle quite quickly: it will overflow the stack and crash your game.

When you have a queue in place, the asynchrony gives the game a chance to unwind the stack. That means a feedback loop may not actually crash the game. Instead, it will probably keep running but behave in a way that's clearly busted. A more pernicious possibility is that it will keep running in a way that's *not* clearly busted.

If the endless loop of events cascading through the system aren't very <span name="log">visible</span> in the game, you may just end up wasting CPU cycles processing them without realizing it. One guideline for avoiding this problem is to avoid sending events from within code that's called in response to a event.

<aside name="log">

A little debug logging in your event system is probably a good idea.

</aside>

## Sample Code

We've gotten a head start on this section already. The beginning of this chapter walked through the synchronous audio API and the problems that caused. It has the right functionality -- the public API we want, and the right low level audio calls. All that's left for us to do now is fix the problems with it.

### Deferring a sound request

The first problem is that our API is *blocking*. When some piece of code wants to play a sound, it can't get back to the other things its doing until `playSound()` finishes loading a sound resource and actually starts making the speaker wiggle.

Let's try the simplest solution we can come up with. We'll make `playSound()` return quickly by deferring the work. First, we define a little data structure to store the details of the request:

^code play-message

Then we'll give `Audio` a field to store an instance of it, along with a little flag to track if it's been set. Now, `playSound()` just fills that in:

^code pending

This way, `playSound()` returns virtually instantly -- it's not really doing anything anymore. But we do still actually have to play the sound, of course. We can't just delete that code. It needs to go somewhere, and that somewhere is an `update()` method:

<span name="update"></span>

^code defer-update

<aside name="update">

Yup, you guessed it. This is the <a href="update-method.html" class="pattern">Update Method</a> pattern.

</aside>

All it does is check to see if there is a pending request. If so, it processes it and clears it. Now we just need to call this. The typical answer is to invoke it from your main <span name="game loop">game loop</span>.

<aside name="game loop">

Another pattern, <a href="game-loop.html" class="pattern">Game Loop</a>! They all tie together! There is a method to the madness!

</aside>

This lets us call it at an appropriate time where we're OK with the processing taking a little while. In real code, you actually don't want to do a blocking call to load a resource right from your game loop. Instead, you'd throw this on a separate thread or use an asynchronous lower-level API. In the interest of simplicity, please tolerate a little hand waving here.

We seem to have solved our first problem. Calling `playSound()` is now super fast, and we actually do the processing at a better, well-defined point in our execution. But what if we try to call `playSound()` *twice* before we get a chance to call `update()`?

Our simplest-possible-idea only supports a single pending request. To go beyond that takes us a step closer to the heart of this pattern...

### Multiple sound messages

Obviously, we need `Audio` to have room to store multiple pending play messages. Now, your <span name="prof">algorithm professor</span> may have told you to to use something more existing here. Maybe a heap, or a hash table, or at least a linked list. But, in practice, the best way to store a collection of homogenous items is almost always just a flat array:

<aside name="prof">

As an algorithm professor, their job is publish novel analyses of data structures. They aren't exactly incentivized to get you to stick to simple ones!

</aside>

1. It doesn't do any dynamic allocation.
2. There's no memory overhead for bookkeeping information or pointers.
3. It's contiguous in memory, which is <span name="locality">cache</span> friendly.

<aside name="locality">

For lots more on what being "cache friendly" means, see the chapter on <a href="data-locality.html" class="pattern">Data Locality</a>.

</aside>

So let's do that:

^code pending-array

We can tune the size of the array to have as much room as we need to cover our worst case. To play a sound, we just add a new message to the end of the array:

^code array-play

Then when we update, we'll process all of the pending messages:

^code array-update

This works fine, but it does presume we can process every sound request in a single call to `update()`. That's probably true in our example, but this chapter is about event *queues*, not event *buffers*.

In order for `update()` to work on just a few requests at a time, it needs to be able to pull requests out of the buffer while leaving others in there. We want to ensure that the oldest pending requests get priority, so it needs to process those first. In other words, we need an actual queue.

### A ring buffer

There are a bunch of ways to implement queues, but my favorite is called a *ring buffer*. It preserves everything that's good about a simple array, but lets us incrementally remove items from the front of the queue.

Now, I know what you're thinking. If we remove items from the beginning of the array, don't we have to shift all of the remaining items over? Isn't that slow?

This is why you learned how great linked lists are: it's super easy to remove nodes from the list without having to shift things around. Well, it turns out you can implement a queue without any shifting in a simple array too. I'll walk you through it, but first let's get precise on some terms.

* The **head** of the queue is the oldest pending request, and the one that will be processed *next*. Since `playSound()` appends to the end of the array, it's the element in there with the lowest index.

* The **tail** is the other end of the queue. It's the slot in the array where the next enqueued request will go. Note that it's just *past* the end of the queue. You can think of it as a half-open range if that helps.

In other words, in our array, the head is on the left, and the tail of the queue grows towards the right. Let's code that up. First, we'll tweak our fields a bit to make these two markers explicit in the class:

^code head-tail

In the implementation of `playSound()`, `numPending_` has been replaced by `tail_`, but otherwise it's the same:

^code tail-play

The more interesting change is in `update()`. To keep things simple in the example, let's say that we only process a single request at a time in a call to update. That looks like this:

^code tail-update

The changes are pretty small. It just processes the request at the current head and then discards that by bumping the index of the head to the right. We tell if the queue is empty by seeing if there's no distance between the head and tail. Now we've got a queue: we can add to the end and remove from the front.

We have an obvious problem, though. As we run stuff through this queue, over time the head and tail will keep crawling to the right. Eventually, `tail_` will hit the end of the array, that `assert()` will fire, and party time is over.

In our implementation now, `MAX_PENDING` isn't the maximum number of queued requests, it's the maximum number of requests we can handle, ever!

This is where it gets clever. If you'll notice, while the tail is creeping forward, the head is too. That means we've got array elements at the beginning of the array that aren't being used any more.

So what we'll do is make the tail of the queue wrap back around to the beginning of the array when it runs off the end. That's why it's called a *ring* buffer: it acts like a circular array of cells.

Implementing this is surprisingly simple. When we enqueue an item, we just need to make sure the tail wraps around to the beginning of the array when it reaches the end, like so:

^code ring-play

See how the `tail_++` was replaced with an increment and a modulo? That wraps back around for us. The other change is the assert. We need to ensure the queue doesn't overflow. We can't determine that just by the position of the `tail_`, since the queue will work its way throughout the whole array.

Instead, we need to check if the head and tail have *collided*. As long as there are fewer than `MAX_PENDING` requests in the queue, there will be a little chunk of unused cells between the head and tail. If the queue fills up, those will be gone and -- like some weird backwards Ouroboros -- adding a new item to the end of the tail will start wiping out items in the head. The assertion checks to ensure that won't happen.

Implementing `update()` is equally simple. We just wrap the head around too:

^code ring-update

There we go: A growable queue with a <span name="capacity">fixed maximum capacity</span> with no dynamic allocations, no copying elements around, and the cache friendliness of a simple array.

<aside name="capacity">

If the hardcoded maximum capacity bugs you, you could use a growable array. When the queue gets full, you allocate a new array twice (or some other constant multiplier) the size of the current array, then copy all of the items over.

While that seems like it might be a performance hit, you can prove that even when you grow like this, enqueuing an item has constant *amortized* complexity.

</aside>

### Aggregating requests

We've solved our first problem and now we've got a basic asynchronous queue up and running. Let's move on to some of the knock-on issues. The next one is that if we have two requests to play the same sound, they interfere with other.

This is easy to solve now. Since we have a queue, we know which requests are going to be processed. All we need to do is discard a request if its identical to one that's already pending:

^code drop-dupe-request

When we have two requests to play the same sound, we collapse them to a single request for whichever was loudest. This "aggregation" is pretty rudimentary, but you could use the same technique to do more interesting batch processing of requests depending on your needs.

Note that we're doing the aggregation when the request is being enqueued, not when it's being processed. That's a little easier on our queue since we don't waste slots on duplicate requests that will end up being collapsed. It's also simpler to implement this way.

It does, however, put the processing burden on the caller. Since this is in `playSound()` we will walk the queue synchronously before it returns. If the queue is particularly large, that could be <span name="slow">slow</span>. In that case, it may make more sense to aggregate in `update()` instead.

<aside name="slow">

Another way to avoid the *O(n)* cost of the scanning the queue is to use a different data structure. If we use a hash table keyed on the `SoundId`, then we can check for duplicates in constant time.

</aside>

There's an important consideration to keep in mind here. When we're doing this aggregation, our window into the set of "simultaneous" requests is only as big as the queue. If we process requests more quickly and the queue size stays small, then we'll have fewer opportunities to batch things together. Likewise, if processing lags behind and the queue gets more full, we'll find more things to collapse.

The intent of this pattern is to make it so that the requester doesn't care when the request gets processed. But if you start scanning the queue and treating it like a live data structure to be played with, then lag between request and processing can become visible to the user. Make sure you're OK with that.

### Spanning threads

The last problem was the most pernicious one. With a synchronous audio API, whatever thread is running the code that calls `playSound()` will be the thread that processes that request. That's often not what you want.

On today's <span name="multicore">multi-core</span> hardware, you need more than one thread if you want to get the most out of your chip. There an infinite array of ways to distribute a game's processing across multiple threads, but a common strategy is to move each domain of the game code onto its own thread: audio, rendering, AI, etc.

<aside name="multicore">

Straight-line code only runs on a single core at a time. If you don't use threads, even if you do the crazy asynchronous-style programming that's popular these days, the best you'll do is keep one core busy. On a four core chip, that means you're only using 25% of the CPU that you could be.

Outside of games, programmers compensate for that by splitting their application into multiple independent *processes*. Each can run on a different core and the OS will keep them all busy. Games (the game client itself, game *servers* are a bit different) are almost always a monolithic process, so a bit of threading really helps.

</aside>

By this point, we're in much better shape to handle that. We have three critical pieces already:

1. The code for requesting a sound is decoupled from the code that plays it.
2. We have a queue for marshalling between the two.
3. That queue is completely encapsulated from the rest of the program.

All that's left for us is to make the two functions that modify the queue -- `playSound()` and `update()` thread-safe. Ideally, I'd whip up a bit of concrete sample code to show you how to do that. But, since this is a book about architecture and patterns, I try not to get mired in the details of any specific API.

Threaded code is particularly hard to show in a lucid way. There are a lot of subtle corner cases that are easy to get wrong, and even the most trivial threading problem seems to spur fierce debate about the best way to do it. Mutex? Semaphor? Condition variables? Critical section?

Any of those can work. At a high level, all that really needs to happen is that we ensure that queue isn't modified concurrently. Since `playSound()` does a very small amount of work -- basically just assigning a few fields -- it can lock very a short period of time without causing much stress on the game. Likewise, `update()` can wait on something like a condition variable so that it doesn't burn any CPU time unless there's actual work to be done. Thread-safe *and* efficient!

## Design Decisions

### What goes in the queue?

I've been using "event" and "meesage" as if they were synonymous because, for most purposes, it doesn't matter what you're stuffing in the queue. You get the same decoupling and aggregation capabilities regardless of what goes through the pipe.

But it is useful to think about what your queing. In our audio example, it was *requests*, or <span name="command">*messages*</a>. Think of them as verbs. We put them in the queue because we want to perform them later.

<aside name="command">

Another word for "verb" here is "command". And, indeed, this is close to the <a href="command.html" class="gof-pattern">Command pattern</a>, and queues are often used there too.

</aside>

In other use cases we've talked about, the queue holds "events" -- things that happened in the past. We put them in the queue because we want to *respond* to them later. Depending on the problem you're trying to solve, you'll naturally do one or the other.

* **When you queue events:**

    You're basically doing an asynchronous <a href="observer.html" class="gof-pattern">Observer pattern</a>.

    * *You are likely to allow multiple listeners.* Since the queue contains
        things that already happened, the sender probably doesn't care what responds to it. From it's perspective, it's in the past and is already forgotten. That means is easy to allow *multiple* things to respond without confusion.

    * *The scope of the queue tends to be broader.* Event queues are often used to *broadcast* events to any and all interested parties. To allow maximum flexibility for which parties can be interested, these queues tend to be less encapsulated.

* **When you queue messages:**

    You can think of this more as an asynchronous API to a specific service. You have some outside code that wants an action to happen and often knows *who* should do that action. It just doesn't control *when* the action was done.

    * *You are more likely to have a single listener.* Like in our audio API example, the queued messages are requests specifically for *the audio API* to play a sound. If other random parts of the game engine started stealing messages off the queue, it wouldn't do us much good.

        I say "more likely" here though, because it is still possible to enqueue messages without caring which specific system processes it, as long as it gets processed the way you expect. In that case, you're doing something akin to the <a href="service-locator.html" class="pattern">Service Locator pattern</a>, where you decouple a caller from the concrete type implementing the call.

### Who can read from the queue?

In our audio example, the queue was internal to the API and only the audio engine itself dequeued items. In something an event loop for a user interface, multiple places in the application can register event listeners. You sometimes hear the terms "single-cast" and "broadcast" to distinguish these, and both are useful.

* **A single-cast queue:**

    This is the natural fit when a queue is part of a class's API. Like in our audio example, from the caller's perspective, they just see a `playsound()`method they can call.

    * *The queue becomes an implementation detail of the reader.* All the sender knows is that it sent a message.

    * *The queue is more encapsulated.* All other things being equal, more  encapsulation is usually better.

    * *You don't have to worry about contention between listeners.* When multiple things can read from the queue, you have to decide if they *all* get every item (broadcast) or if *each* item in the queue is parcelled out to *one* listener (something more like a work queue).

        In either case, the listeners may end up doing redundant work or interfere with each other, and you have to think carefully about the behavior you want. With a single listener, that complexity disappears.

* **A broadcast queue:**

    This is how most "event" systems work. If you have ten listeners when an event comes in, all ten of them will see the event.

    * *Events can get dropped on the floor.* A corollary to the above is that if you have *zero* listeners, all zero of them will see the event. In most broadcast systems, if there are no listeners at the point in time that an event is processed, the event just gets discarded.

        For something like user input, this is typically what you want. It would be strange if those events were kept in the queue indefinitely. If you registered a mouse event handler later after your program had been running for a while, all of the sudden you would get a huge stream of events for every twitch the user had done since your app started.

        Where a single-cast queue feels like "listening" means putting on a record and starting at the beginning, a broadcast queue -- as the name implies -- is more like tuning into a show already in progress. You only catch things that happen after you tune in.

    * *You often need to filter events.* To enable many listeners, broadcast queues are usually publicly visible to much of the program. This means you can have many listeners. With many events and many listeners, you can end up with *m &times; n* event handlers to invoke.

        To cut that down to size, most broadcast event systems let a listener control which subset of the events they care about. For example, you may say you only want to receive mouse events, or events within a certain region of the UI.

* **A work queue:**

    This is similar to a broadcast queue where you have multiple listeners. The difference is that each item in the queue will only go to *one* of them. This is a common pattern for doing work concurrently where you have incoming jobs and a pool of threads that can handle them.

    * *You have to schedule.* Since any given item will only go to one listener, the queue has to have some logic to figure out the best way to choose which listener is best for a given message. This may be as simple as a round robin, or something more complex based on priority or tracking which listeners are still busy and which aren't.

### How many can write to the queue?

This is the flip side of the previous design choice. This pattern can accomodate all of possible <span name="configs">configurations</span>: one-to-one, one-to-many, many-to-one, or many-to-many.

<aside name="configs">

You sometimes here "fan-in" used to describe many-to-one communication systems, and "fan-out" for one-to-many.

</aside>

* **With one writer:**

    This is style is most similar to the synchronous Observer pattern. You have one priveleged object that generates events that others can then receive. This is how GUI application event systems work, where the operating system itself is the one generator of events.

    * **You implicitly know where the event is coming from.** Since there's only one object that can add to the queue, any listener knows where it's coming from.

    * **You usually allow multiple readers.** You can have a strict one-sender-one-receiver queue, but those are less common and start to feel less like the communication structure this pattern is about and more just a plain queue data structure.

* **With multiple writers:**

    This is how our audio engine example works. Since `playSound()` is a public API call, any part of the codebase can add a request to the queue. "Global" or "central" event buses work like this too.

    * **You have to be more careful of cycles.** Since anything can potentially put something onto the queue, it's easier to accidentally enqueue something in the middle of handling an event. If you aren't careful, that will trigger a cascade.

    * **You'll likely want some reference the sender in the event itself.**
    When a listener gets an event, it doesn't know who sent it, since it could be anyone. If that's something they need to know, you'll need to pack that into the event object so that the listener can use it.

### What is the lifetime of the objects in the queue?

With synchronous notifications, by the time execution has returned back to the message sender, all processing of the message is done. That means the message itself can safely live on the stack.

Now that we've got a queue in there, the message needs to outlive the call to enqueue it. If you're using a garbage collected language, you don't need to worry about this too much. Just stuff it in the queue and it will stick around as long as it's needed. In C++, you'll need to decide how you want to ensure the object sticks around.

* **Pass ownership:**

    This is the traditional way to do things when managing memory manually. When a message gets queued, the queue claims it and the sender no longer owns it. When it gets processed, the receiver takes ownership and is responsible for deallocating it.

* **Share ownership:**

    These days, now that even C++ programmers are more comfortable with garbage collection, shared ownership à la `shared_ptr` is more typical. With this, the message will stick around as long as needed and be automatically freed when its fully processed.

* **The queue owns it:**

    <span name="pool">Another option</span> is to have messages *always* live on the queue. Instead of allocating the message itself, it requests a "fresh" one from the queue. The queue returns a reference to a message already in memory inside the queue, and the sender fills it in. When the message gets processed, the receiver just refers to the same message in the queue.

    <aside name="pool">

    In other words, the queue is also implementing the <a href="object-pool.html" class="pattern">Object Pool</a> pattern.

    </aside>

## See Also

* I've mentioned this a few times already, but in many ways, this pattern is the asynchronous cousin to the well-known <a href="observer.html class="gif-pattern">Observer pattern</a>. Unless you actually need the asynchrony, observers are a simpler and often faster choice.

* When the queue contains "messages" or "requests" for something to be done, those objects start to sound very similar to <a href="command.html" class="gof-pattern">Commands</a>.

* [Finite state machines](http://en.wikipedia.org/wiki/Finite-state_machine), which are similar to the Gang of Four's <a href="state.html" class="gof-pattern">State pattern</a> respond to an incoming stream of inputs.

    If you don't want your state machine to respond to inputs synchronously, it may make sense to put a queue in there. If you have a bunch of these state machines communicating with each other, each with a little queue of pending inputs -- usually called a *mailbox* -- then you've basically re-invented the [actor model](http://en.wikipedia.org/wiki/Actor_model) of computation.

    The [Erlang](http://www.erlang.org/) programming language is based around this model. Where event queues are a *pattern* you have to implement yourself in other languages, in Erlang, they are built right in. Channels in the [Go](http://golang.org/) programming language

* Like many patterns, this one has a number of aliases. One of the more established terms is "message queue". That's describes a similar communication pattern, but is usually used to talk about a higher level manifestation of it. Where we talked about event queues *within* an application, "message queues" usually refer to systems for communicating *between* applications.

    There are a number of established middleware packages for this: ZeroMQ, RabbitMQ, etc. If you see "MQ" in the name, now you know what it stands for.

* Another similar term is "publish/subscribe", sometimes abbreviated to "pubsub". Like "message queue", those terms usually refer to larger distributed systems and less the humble coding patterns we're focused on.
