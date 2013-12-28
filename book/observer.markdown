^title Observer
^section Design Patterns Revisited

You can't throw a rock at a hard drive without hitting an application built using the <span name="devised">Model-View-Controller</span> architecture, and underlying that is the Observer pattern. It's not an understatement to say that this is one of the most successful software patterns in the world.

<aside name="devised">

Like so many things in software, MVC was invented by Smalltalkers in the seventies. Lispers probably claim they came up with it in the sixties but didn't bother writing it down.

</aside>

As web *pages* have turned into web *applications*, it's taking over there too. Observer is so important that Java put it in its core library ([`java.util.Observer`](http://docs.oracle.com/javase/7/docs/api/java/util/Observer.html)) and C# baked it right into the *language* (the [`event`](http://msdn.microsoft.com/en-us/library/8627sbea.aspx) keyword).

But the world of game development can be strangely cloistered at times, so maybe this is all news to you. In case you haven't left the abbey in a while, let me walk you through a motivating example.

## Achievement Unlocked

Say you're adding an achievements system to your game. It will feature dozens of different badges players can earn for completing specific milestones, like "Kill 100 Monkey Demons", "Fire a Shotgun in the Dark", "Fall of a Bridge", or "Complete a Level Wielding Only a Dead Weasel".

This is tricky to implement cleanly since you have such a wide range of achievements that are unlocked by all sorts of different behaviors. You really don't want tendrils of your achievement system twining their way through every dark corner of your codebase. Sure, "Fall of a Bridge" is somehow tied to the <span name="physics">physics engine</span>, but do you really want to see a call to `unlockFallOffBridge()` right in the middle of the linear algebra in your collision resolution algorithm?

<aside name="physics">

This is a rhetorical question. No self-respecting physics programmer would ever let you sully their beautiful mathematics with something as pedestrian as *gameplay*.

</aside>

What we'd like, as always, is to have all the code concerned with one aspect of the game nicely lumped in one place. The challenge is that achievements are triggered by a bunch of different parts of the game. How can that work without coupling the achievement code to all of those?

That's what the observer pattern is for. It lets one piece of code announce that something interesting happened *without actually caring who receives the notification*.

For example, you've got some physics code that handles gravity and tracking which bodies are chilling on nice flat surfaces and which are plummeting towards sure demise. To handle the "Fall of a Bridge" achievement, you could just jam the code right in there, but that's a mess. Instead, the code can just do:

^code physics-update

All it does is say, "Uh, I don't know if anyone cares, but this thing just fell. Do with that as you will."

The achievement system registers itself to receive that notification. Whenever the physics code sends it, the achievement receives it and checks to see if the body is for the player. If so, it checks its own bookkeeping data to see what the less-than-graceful hero happened to be standing on prior to his encounter with classical mechanics. If it's a bridge, it unlocks the proper achievement and sets of the fireworks and fanfare, all with no involvement from the physics code.

<span name="tear">In fact</span>, you can change the set of achievements or tear out the entire achievement system without touching a line of the physics engine. It will still send out its notifications, oblivious to the fact that nothing is receiving them anymore.

<aside name="tear">

Of course, if you *permanently* remove achievements and nothing else ever listens to the physics engine's notifications, you may as well remove the notification code too. But during the game's evolution, it's nice to have this flexibility.

</aside>

## How it Works

If you don't already know the mechanics of the actual pattern, you could probably guess
them just from the above description, but to keep things easy on you, I'll just walk
through it quickly.

We'll start with the nosy class that wants to know when another other object does something interesting. It accomplishes that by implementing this:

<span name="signature"></span>

^code observer

<aside name="signature">

The precise signature of `onNotify()` is up to you. That's why this is the Observer *pattern* and not the "Observer ready-made code you can paste into your game". Usually, the notify method is passed the object that sent the notification and a generic "data" parameter you stuff other details into.

If you're using a language with generics or templates, you'll probably use them here, but it's also fine to hand-tailor to your use case. Here, I'm just hardcoding it to take a game entity, and an enum that describes what happened.

</aside>

Any concrete class that implements this becomes an observer. In our example, that's the achievement system, so we'd have something like so:

^code achievement-observer

The notification method is called by the object that did something worth observing. In Gang of Four parlance, that object is called the
"subject". It has two jobs. First, it holds the list of observers that are waiting oh-so-patiently for a missive from it:

<span name="stl"></span>

^code subject-list

<aside name="stl">

In real code, you would use a dynamically-sized collection instead of a dumb array. I'm sticking with the basics here for people coming from other languages that don't know C++'s standard library.

</aside>

The important bit is that the subject exposes a *public* API for modifying that list:

^code subject-register

That allows outside code to control who receives notifications. The subject communicates with the observers, but isn't *coupled* to them. In our example, no line of physics code will mention achievements. Yet, it can still notify the achievements system. That's the clever part about this pattern.

It's also important that the subject has a *list* of observers instead of a single one. It makes sure that observers aren't implicitly coupled to *each other*. For example, say the audio engine also observes the fall event so that it can play an appropriate sound. If the subject only supported one observer, when the audio engine registered itself, that would *un*register the achievements system.

That means those two systems would be interfering with each other, and in a particularly nasty way, since one effectively disables the other. Supporting a list of observers ensures that each observer is treated independently from the others. As far as they know, each is the only thing in the world with eyes on the subject.

The other job of the subject is sending notifications:

^code subject-notify

Now we just need to hook all of this into the physics engine so that it can send notifications and the achievement system can wire itself up to receive them. We'll stay close to the original *Design Patterns* recipe and <span name="event">inherit</span> `Subject`:

^code physics-inherit

This lets us make `notify()` in `Subject` protected. That way the physics engine can send notifications, but code outside of it cannot. Meanwhile, `addObserver()` and `removeObserver()` are public, so anything that can get to the physics system can observe it.

<aside name="event">

If this were real code, I would absolutely avoid using inheritance here. Instead, I'd make `Physics` *have* an instance of `Subject`. Instead of observing the physics engine itself, the subject would be a "falling event" object. Observers could register themselves using something like:

^code physics-event

To me, this is the difference between "observer" systems and "event" systems. With the former, you observe *the thing that did something interesting*. With the latter, you observe an object that represents *the thing that happened*.

</aside>

Now, when the physics engine does something noteworthy, it calls `notify()` just like in the original motivation example above. That walks the observer list and gives them all the heads up. Pretty simple, right? Just one class that maintains a list of pointers to instances of some interface.

It's hard to believe that something so straightforward is communication backbone of thousands of programs and app frameworks.

## What About Games?

If it's such a big deal outside of the game industry, how does it fare inside?
Here, it seems to have gotten a mixed reception. Part of this is that the Observer
pattern usually rides along with MVC, and that isn't very popular in games.

In most applications I've seen, "MVC" really means a <span name="controller">two-layer</span> approach: front end and back end. That separation makes sense in most applications, which have some complex state that needs to be modified and persisted. There's some database or file storage layer, and a pile of complex business logic defining which kind of modifications are allowed.

<aside name="controller">

The "controller" part of MVC is the third wheel that no one knows what to do with and every framework designer redefines to mean whatever they want.

</aside>

Meanwhile, the user interface is fairly separate from that. It changes more frequently, and may have multiple different views into the same underlying data. A chart and grid of cells really are "views" into a distinct "model", which is the tabular data of a spreadsheet. It's still conceptually a "spreadsheet" even if you ditch the chart and radically change the UI.

Games don't have the same <span name="separation">separation</span>. Is a visual effect particle's position "front-end"? It's part of the user experience and doesn't get persisted or affect gameplay. But a particle is positioned in space in the game world, and that position interacts physically with the world. Is the game world the "model"? Are physics the "business rules"?

<aside name="separation">

I think this is part of the reason the <a href="component.html" class="pattern">Component</a> pattern is so popular: games *can* be pretty effectively divided into different domains like physics, AI, and rendering. Components are the MVC of games.

That's not to say MVC is *never* used in games. I have seen games built around it in a way that works well.

</aside>

Without MVC, the Observer pattern hasn't gotten a free ride. Instead, when I've asked other game developers what they think about it, I've heard a few explanations for why not. (That is, when they've even heard of it to begin with.)

The rest of this chapter is going to be about those problems and what we can do to solve them. Once those are out of the way, we'll talk a bit about where I think the pattern makes sense and where it doesn't.

### "It's Too Slow"

The first time I asked about this pattern, the response I got was, "It's too slow". They didn't know the details of the pattern, but their default stance on anything that smelled like a "design pattern" was that it involved piles of classes and indirection and other creative ways of squandering CPU cycles.

The Observer pattern gets a particularly bad rap here because it's been known to hang around with some shady characters named "events", <span name="names">"messages"</span>, and even "data binding". Some of those systems *can* be slow (often deliberately). They involve things like queuing or doing dynamic allocation for each notification.

<aside name="names">

This is why I think documenting patterns is important. When we get fuzzy about terminology, we lose the ability to communicate clearly and succintly. You say, "Observer" and someone hears "Messaging" because either no one bothered to write down the difference or they didn't happen to read it.

That's what I'm trying to do with this book. And, to cover my bases, I've got a chapter on messaging too: <a href="message-queue.html" class="pattern">Message Queue</a>.

</aside>

But, now that you've seen how the pattern is actually implemented, you know that isn't the case. Sending a notification is just walking a list and calling some virtual methods. Granted, it's a *bit* slower than a statically dispatched method, but that cost is negligible in all but the most performance critical code.

I find this pattern works best outside of critical code paths anyway, so you can usually afford the dispatch cost. Otherwise, there's virtually no overhead. We aren't allocating objects for messages. There's no queueing. It's just an indirection over a method call.

### It's too *fast?*

In fact, you have to be careful because the Observer pattern *is* synchronous. The subject invokes its observers directly, which means it can't resume its work until all of the observers have returned from their notification methods. A slow observer can block a subject.

This sounds scary, but in practice it's not the end of the world. It's just something you have to be aware of. UI programmers -- who've been doing event-based programming like this for ages -- have an established motto for this: "stay off the UI thread".

If you're responding to a event synchronously, you need to finish and return control as quickly as possible so that the UI doesn't lock up. When you have slow to work to do, push it onto another thread or a work queue. It takes a little discipline, but it's not rocket science.

## "It Does Too Much Dynamic Allocation"

Whole tribes of the programmer clan -- including many game developers -- have moved
onto garbage collected languages, and dynamic allocation isn't the boogie man
that it used to be for most people. But for performance critical software like
games, memory allocation still matters, even in managed languages. <span name="fragment">Dynamic</span> allocation takes time as does reclaiming memory, even if it happens automatically.

<aside name="fragment">

Many game developers are less worried about allocation and more worried about *fragmentation.* When your game needs to run continuously for days without crashing in order to get certified, an increasingly fragmented heap can prevent you from shipping.

The <a href="object-pool.html" class="pattern">Object Pool</a> chapter goes into more detail about this and a common technique for avoiding it.

</aside>

In the example code above, I just used a fixed array because I'm trying to keep
things dead simple. In real implementations, the observer list is almost always
a dynamically allocated array or list that grows and shrinks as observers are
added and removed. That memory churn spooks some people.

Of course, the first thing to notice is that it only allocates memory when observers are being wired up. *Sending* a notification requires no memory allocation whatsoever: it's just a method call. If you wire up your observers at the start of the game and don't mess with them much, the allocation cost may be minimal.

If it's still a problem, though, I'll walk through a way to implement adding and removing observers without any dynamic allocation at all.

### Linked observers

In the code we've seen so far, `Subject` owns a list of pointers to each `Observer` watching it. The `Observer` class itself has no reference to this list. It's just a pure virtual interface. Interfaces are preferred over concrete, stateful classes, so that's generally a good thing.

But if we *are* willing to put a bit of state in there, we can thread the subject's list *through the observers themselves*. Instead of the subject having a separate collection of pointers, the observer objects become nodes in a linked list.

To implement this, first we'll get rid of the array in `Subject` and replace it with a pointer to the head of the list of observers:

^code linked-subject

Then we'll extend `Observer` with a pointer to the next observer in the list:

^code linked-observer

We're also making `Subject` a friend here. The subject owns the API for adding and removing observers, but the list it will be managing is now inside the `Observer` class itself. The simplest way to give it the ability to poke at that list is by making it a friend.

Adding an observer is just wiring it into the list. We'll take the easy
option and insert it at the front:

^code linked-add

The other option would be add it to the end of the linked list. Doing that adds a bit more complexity: `Subject` has to either walk the list to find the end, or keep a separate `tail_` pointer that always points to the last node.

Adding it to the front of the list is simpler, but does have one side effect. When we walk the list to send a notification to every observer, the most recently registered observer gets notified first. If you register observers A, B, and C, in that order, they will receive notifications in C, B, A order.

In theory, this doesn't matter one way or the other. It's a tenet of good observer discipline that two observers observing the same subject should have no ordering dependencies relative to each other. If the ordering *does* matter, it means those two observers have some subtle coupling that could end up biting you.

Regardless of which end of the list we add the new observer, you can see we achieved our goal here: adding an observer is just a couple of pointer assignments. Not an allocation in sight.

Let's get remove working:

<span name="remove"></span>

^code linked-remove

<aside name="remove">

Removing a node from a linked list usually requires a bit of ugly special case handling for removing the very first node, like you see here. There's a more elegant solution using a pointer to a pointer. Consider it an exercise for the reader.

</aside>

Because we have a singly linked list, we have to walk it to find the observer we're removing. We'd have to do the same thing if we were using a regular array for that matter. If we use a *doubly* linked list, where each observer has a pointer to both the observer after it and before it, we can remove an observer in constant time. If this were real code, I'd do that.

The only thing left to do is sending a notification. That's as simple as walking the list:

^code linked-notify

Not too bad, right? A subject can have as many observers as it wants, without a single whiff of dynamic memory. Registering and unregistering is as fast as it was with a simple array. There's only two quibbles with this. The first is that walking a linked list means skipping around in memory more. That can cause <span name="cache">cache misses</span>, which harm your performance. In hot code paths, that may be unacceptable.

<aside name="cache">

See the chapter on <a href="data-locality.html" class="pattern">Data Locality</a> for a ton of more information on how CPU caching affects performance and what you can do about it.

</aside>

The other problem is a more fundamental. Since we are using the observer object itself as a list node, that implies it can only be part of one subject's observer list. In other words, it can only observe a single subject at a time. In a vanilla observer implementation where each subject has its own independent list, an observer can be in more than one of them simultaneously.

### A pool of list nodes

You may be able to live with that limitation. I find it more common for a subject to have multiple observers than vice versa. If it *is* a problem for you, there is another more complex solution you can use that still doesn't require dynamic allocation. It's too long to cram into this chapter, but I'll sketch it out and let you fill in the blanks.

The basic idea is that each subject will have a linked list of observers. Unlike the design we just saw, those nodes won't be observers themselves. Instead, they'll be separate little "node" objects that contain a pointer to the observer and then a pointer to the next node in the list. This way, multiple nodes can point to the same observer.

The way you avoid dynamic allocation is simple: Since all of those nodes are the same type, you create an <a href="object-pool.html" class="pattern">object pool</a> of them. That gives you a fixed-size pile of list nodes to work with, and you can use and reuse them as you need without having to hit an actual memory allocator.

## It's Too Awesome?

By now, you shouldn't be frightened of the technical details of the pattern anymore. As you can see, it's quite simple and you can even tweak how it uses memory. But does that mean that you should observers all the time?

That's a different question. Like all design patterns the Observer pattern isn't a cure-all. Even when implemented correctly and efficiently, it may not be the right solution. The reason design patterns get a bad rap is because people apply good patterns to the wrong problem and end up making things worse.

There are two challenges with the Observer pattern, one technical and one at something more like the usability level. We'll do the techical one first because those are always easier than human factors issues.

### Destroying subjects and observers

I think the sample code we walked through is solid for what it is, but it side-steps an important issue: when happens when you delete a subject or an observer? If you just wantonly call `delete` on some observer, a subject may still have a pointer to it. That's now a dangling pointer into deallocated memory. When that subject tries to send a notification to it, well... you're not going to end up in your Happy Place.

Destroying the subject is easier since in most implementations the observer doesn't have any references to it. But even then, sending its bits straight to the memory manager's recycle bin may cause some problems. Those observers may still be expecting to receive notifications in the future, and they don't know that that will never happen now. They aren't observers at all any more, they just think they are.

You can deal with this in a couple of different ways. The simplest is to do what I did and just punt on it. It's an observer's job to unregister itself from any subjects when it gets deleted. More often than not, the observer *does* know which subjects it's observing, so it's usually just a matter of <span name="destructor">adding</span> a `removeObserver()` call to its destructor.

<aside name="destructor">

As always, the hard part isn't doing it, it's *remembering* to do it.

</aside>

If you don't want to leave observers hanging when a subject gives up the ghost, that's easy to fix. Just have the subject send one final "dying breath" notification right before it gets destroyed. That way any observer can receive that and take whatever action (mourn, send flowers, etc.) it thinks is appropriate.

People, even those of us who've spent enough time in the company of machines to have some of their precision rub off on us, are reliably terrible at being reliable. That's why we invented computers: they don't make the mistakes we so often do.

A safer answer is to make observers automatically unregister themselves from every subject when they get destroyed. If you implement the logic for that once in your base observer class, everyone using it doesn't have to remember to do it themselves. This does add some complexity, though. It means each *observer* will need a list of the *subjects* its observing. You end up with pointers going in both directions.

### It's cool, I've got a GC

All of you cool kids with your hip modern languages with garbage collectors are feeling pretty smug right now. Think you don't have to worry about this because you never explicitly delete anything? Think again!

Imagine this: you've got some UI screen that shows a bunch of stats about the player's character like their health and stuff. The player can bring up the screen and dismiss it whenever they want. You implement the screen as an observer where the subject is the main character.

Every time the character takes a punch to the face (or elsewhere, I suppose), it sends a notification. The UI screen observes that and updates the little health bar. Great. Now what happens when the player dismisses the screen, but you don't unregister the observer?

The UI won't be visible anymore, but it will still be in memory. The entire time the player is playing the game, running around, getting in fights, the character will be sending notifications. Those will get sent to the UI screen, which will then reposition a bunch of UI elements and do other utterly pointless work.

This is such a common problem in notification systems that it has a <span name="lapsed">name</span>: the *lapsed listener problem*. Even though the user may not see a problem, you're wasting memory and CPU cycles on some zombie UI. The lesson here is that you always have to be disciplined about registration.

<aside name="lapsed">

An even surer sign of its significance: it has [a Wikipedia article](http://en.wikipedia.org/wiki/Lapsed_listener_problem).

</aside>

### What's going on?

The other, deeper problem with the Observer pattern comes directly from the problem it solves. We reach for Observer because it prevents our code from being too tightly coupled. A subject can indirectly communicate with an observer without being statically bound to it.

For such a simple pattern, it accomplishes that quite well. But, if you've ever had to debug a program that uses observers pervasively and isn't doing what it's supposed to do, you've learned that it decouples things a bit *too* well.

As you're tracing your way through the codebase, there's no easy way to see which parts of the program are communicating with which other parts. Instead of a simple direct named function call that you can look up, all you see is "send a notification".

To figure out who *receives* that notification, you have to determine which set of observers happen to be bound to that subject at runtime. Instead of being able to *statically* reason about the communication structure of the program, you have to reason about its *imperative, dynamic* behavior.

The coupling is still there. After all, those pieces of code do need to communicate. It's just that now the coupling is an <span name="emergent">emergent</span> property of how the observers happened to be wired up at runtime. When you've got long chains of observers and subjects, debugging this communication network can be a nightmare.

<aside name="emergent">

I've worked with programs that treated observers like the One True Religion and figuring out why something wasn't working was about as fun as figuring out why a balled-up string of Christmas lights won't come on, except instead of a single string of them, it was a branching graph of wires full of lights that can spontaneously unplug each other.

</aside>

There is no perfect solution here. There are real valid reasons to not want to statically staple pieces of your codebase to each other. Maintaining tightly coupled code sucks: it makes it impossible to reason about parts of the program in isolation. And, when the game gets too big to fit in your head, you *need* to be able to reason about it locally. But replacing that coupling with something more dynamic does mean *globally* reasoning about your program is harder.

Don't cry yourself to sleep though. The trick with this is to use it where it helps and avoid it where it doesn't. Here's a simple guideline: *if the subject can't do its own work without having an observer, don't use this pattern*.

This pattern is a good fit when the observer cares about the subject but the subject couldn't care less about the observer. If that isn't the case, then it implies there is a real, meaningful coupling between those two objects. In that case, it's usually better to avoid this pattern and make that coupling more explicit.

The Observer pattern is a great way to layer "secondary" or "ancillary" behavior on top of a core shell of "real" work. Like in our example, the physics engine doesn't need an achievement system whatsoever. It's job is physics and it can do that just fine without anyone watching. It supports observers because that's the simplest, least coupled way we can get the achievement system working, but it doesn't do anything for the physics engine itself.

## Observers Today

Design Patterns came out in the <span name="90s">90s</span>. Back then object-oriented programming was *the* hot paradigm. Every programmer on Earth wanted to "Learn OOP in 30 Days" and middle managers paid them based on the number of classes they created. Engineers judged their mettle by the depth of their inheritance hiearchies.

<aside name="90s">

That same year, Ace of Bass had not one but three hit singles, so that may tell you something about our taste and judgment back then.

</aside>

The Observer pattern got popular during that zeitgeist, so it's no surprise that it's class heavy. But mainstream coders now are more comfortable with functional programming. Having to implement an entire interface just to receive a notification doesn't fit today's aesthetic.

It feels <span name="different">heavyweight</span> and rigid. It *is* heavyweight and rigid! It's impossible to have a single class that can use methods for observing different subjects.

<aside name="different">

This is why implementations pass the subject to the notify method. Since an observer only has a single notify method, if it's observing multiple subjects, it needs to be able to tell which one called it.

</aside>

A more modern approach is for an "observer" to just be a reference to a method or function. In languages with first class functions, and especially ones with <span name="closures">closures</span>, this is a much more common way to do observers.

<aside name="closures">

These days practically *every* language has closures. C++ overcame the challenge of closures in a language without garbage collection before Java got its act together and introduced them in JDK 8.

</aside>

For example, C# has "events" baked into the language. With those, the listener you register is just a "delegate", which is that language's term for a first-class reference to a method. In JavaScript's event system, observers can be objects supporting a special `EventListener` protocol, but they can also just be functions. The latter is almost always what people use.

If I were designing an observer system today, I'd make it <span name="function">function-based</span> instead of class-based. Even in C++, I would tend towards a system that let you register member function pointers as observers instead of instances of some `Observer` interface.

<aside name="function">

[Here's](http://molecularmusings.wordpress.com/2011/09/19/generic-type-safe-delegates-and-events-in-c/
) an interesting blog post on one way to implement this in C++.

</aside>

## Observers Tomorrow

Event systems and other observer-like patterns are incredibly common these days. They're a well-worn path. But if you write a few large apps using them, you start to notice something. A lot of the code in your observers is pretty dumb boilerplate. It's usually something like:

  1. Get notified that some state has changed.
  2. Imperatively modify some chunk of UI to reflect the new state.

It's all, "Oh, the hero health is 7 now? Let me set the width of the health bar to 70 pixels." After a while, that gets pretty tedious. Computer science academics and software engineers have been trying to eliminate that tedium for a *long* time. Their attempts have gone under a number of different names: "dataflow programming", "functional reactive programming", etc.

While there have been some successes, usually in limited domains like audio processing or chip design, the holy grail still hasn't been found. In the meantime, a humbler step in that direction has started gaining traction. Many recent application frameworks now use "data binding".

Unlike more ambitious approaches, data binding doesn't try to entirely eliminate imperative code and doesn't try to architect your entire application around a giant declarative dataflow graph. What it does do is automate a bunch of obvious busywork where you're tweaking a UI element to reflect a change to some value.

Like other declarative systems, data binding is probably a bit too slow and complex to fit inside the core of a game engine. But I would be surprised if I didn't see it start making in-roads into less critical areas of the game like UI.

In the meantime, the good old Observer pattern will still be there waiting for us. Sure, it's not as exciting as some hot technique that manages to cram both "functional" and "reactive" in its name, but it's dead simple and it works. To me, those are often the two most important criteria for a solution.
