^title Singleton

If I had to reduce this chapter to one word, it would be: *don't*.
The Singleton pattern is the most-used design pattern in games, and
the least needed. In most cases, using it actually make things worse.
Before I try to convince you why it's a bad idea and show you some
alternatives, I suppose I'm obligated to at least explain what the
pattern is.

## The Singleton Pattern

*Design Patterns* summarizes Singleton like this:

> Ensure a class has one instance, and provide a global point of
> access to it.

Let's split at the "and" and consider each part separately.

### Ensuring a Class Has One Instance

There are times when a class can only perform correctly only if
there is a single instance of it. This usually happens when your class
wraps or communicates with an external system that maintains its own
global state.

For example, let's say you're writing a class that wraps the
platform's underlying file system. Because file system operations can
be slow, your class handles most operations asynchronously. These
operations need to be coordinated with each other. If you start one
asynchronous operation to create a file, and another one to delete
that same file, you want to make certain that those two operations are
coordinated.

If users can freely create instances of your file system wrapper,
those instances have no way of coordinating with each other. Enter the
singleton: a pattern for letting a class itself ensure that it will
have only a single instance.

### Providing a Global Points of Access

Lots of places in our codebase may need to read and
write files. If they can't create their own instances of our file
system wrapper, how do they do that? This is the other half of the pattern. In addition to
creating the single instance, we also provide a globally-available
method to get it. This way, anyone anywhere can get their paws on our blessed instance.

Solving both of those, the canonical singleton implementation looks like this:

^code 1

The static `sInstance` member will hold an instance of
the class. The private constructor ensures that no other code can
create their own FileSystems, so there can be no more than one. The
public static `Instance()` method lets any place in the codebase
access the instance. It is also responsible for creating the instance
using lazy initialization. It will instantiate it the first time
someone asks for it.

## The Good Points

Right off, our implementation has a few good things going for it.

*   **It's convenient.** This the big win, and the reason it's used
    so much. With this simple implementation, it's easy as pie for
    anywhere in our codebase to get at the file system and do what it
    needs. It's the drive-thru fast food joint of design patterns.

*   **It doesn't allocate the Singleton if no one uses it.** Saving
    memory and cycles is always good. Since the instance is lazy
    initialized, if it's *never* accessed, it will never get created
    at all.

*   **You can subclass the Singleton.** This is a great but often
    overlooked feature. It isn't necessary for the `Instance()`
    function to actually allocate an instance of `FileSystem`.
    Instead, we can create FileSystem as an abstract file system
    interface, and defer the implementation to a concrete subclass.
    First, let's define a file system interface, and two
    platform-specific implementations:

    ^code 2

    Now we turn `IFileSystem` into a Singleton:

    ^code 3

    The fun part is how the instance is initialized. Instead of just
    creating an instance of its own class, it now chooses an
    appropriate platform-specific derived class:

    ^code 4

    The end result is that our entire codebase can access the file
    system using `IFileSystem::Instance()`, but only the
    implementation file for the `IFileSystem` class itself is coupled
    to any of the platform-specific code.

## Then the Trouble Starts

I said Singleton is the fast food of the design pattern world, and like a greasy burger, it goes down easy but after a while you start
to regret eating them for every meal. Unfortunately, while the benefits of it are immediately apparent, the problems it causes may
not be obvious for a while:

### It's a glorified global variable.

Smart programmers who came before us learned the hard way how much
pain global state can cause. They've passed on that legacy by beating
into our heads "globals = bad". However, doing clean object-oriented
design *without* any global state is surprisingly hard.

Many game programmers come from a low-level procedural background.
When you're coding in assembly or C, globals and statics are a common
way to solve problems. Moving to C++ and being told not to do that
anymore feels like a major tool in the toolbox has been taken away.

The Singleton pattern feels like a guilt-free way to get that tool
back. Sadly, it's a sham. While the pattern wraps the global state up
into something more OOP-friendly, it doesn't really solve the problems
global state causes any more than pouring low-fat dressing on a
quarter-pounder with cheese makes it healthier.

If you only learned to avoid globals because it's the Right Thing To
Do, here's a quick review on the trouble they cause:

<aside>

Computer scientists call functions that don't access or modify global
state "pure" functions. Pure functions are easier to reason about,
easier for the compiler to optimize, and let you do neat things like
memoization where you cache and reuse the results from previous calls
to the function.

While there are challenges to using purity exclusively, the benefits
are strong enough that languages like Haskell have been developed that
*only* allow pure functions.

</aside>

*   **It makes it harder to reason about code.** Let's say you're
    reading a function someone else wrote, trying to figure out what's
    causing a bug. If that function doesn't touch any global state,
    then all you need to hold in your head is the arguments being
    passed in, and the code for the function itself.

    Now imagine right in the middle of that function is a call to
    `SomeClass::GetSomeGlobalData()`. Now, to understand that piece of
    code, you have to hunt through the entire codebase and see what
    could be changing that global data. You don't really hate globals
    until you've had to grep an entire game's codebase at three in the
    morning and look at every single callsite trying to find the one
    errant call that's putting some global module in an unexpected
    state.

*   **It encourages coupling.** The new coder on your team isn't
    familiar with your game's beautiful decoupled architecture, but
    he's just been given his first task: make boulders play sounds
    when they fall onto the ground. You and I know we don't want the
    physics code to be coupled to *audio* of all things, but he's just
    trying to get his task done. Unfortunately for us, the instance of
    our AudioPlayer is globally visible. So, one little `#include`,
    and our new guy's now added a coupling between modules that will
    cause maintenance headaches for years.

    If there hadn't been a global instance of the audio player, even
    if he *did* `#include` the audio header, he still wouldn't be able
    to get at what he needs. That difficulty would encourage him to
    do things the right way. When you control access to instances,
    you control coupling.

*   **It isn't concurrency-friendly.** The days of games running on a
    simple single-threaded CPU are pretty much over. Code today must
    at the very least work in a multi-threaded way even if it doesn't
    take full advantage of concurrency. When you make something
    global, you've created a chunk of memory that every thread can
    see and poke at, without realizing what other threads may be
    doing to it. It's all too easy to run into deadlocks or other
    hideous synchronization issues.

You'll notice that the Singleton pattern doesn't dodge *any* of those
bullets. A Singleton is still a global, just with a fancy name.

### It Solves Two Problems, but You Probably Just Have One.

That "and" in the Gang of Four's description of the pattern is a bit
odd. The two parts honestly don't seem that related. Ensuring a single
instance sounds pretty useful, but who says you want to let everyone
poke at it? Global access to an instance is definitely convenient,
but that's convenient even for classes where multiple instances really
wouldn't cause any problems.

The latter of the two is almost always why programmers use the
Singleton pattern. Let's say your game has a VFX (visual effects)
engine for little bits of graphical dazzle like smoke, explosions,
and other flashes of light and color. A bunch of places will need to
touch that: your AI code may need to put on a light show when our
hero levels up, the physics code shoots out a shower of sparks when swords collide. We need a single point of access so that all of those
systems can get to our VFX one.

So we reach for the Singleton pattern, but we get this other odd little rule comes along for the ride. All of the sudden, we can no
longer instantiate more than one VFX system, even though there's
nothing in its code that would be harmed by doing so.

At first, it seems like a harmless addition. There probably is only
one VFX system in the game anyway, so who cares if it won't let you
make more. The problem is that what's right today may not be right
tomorrow. You may later decide to run separate VFX systems in parallel
so that you can allocate more memory to one for more important effects, and restrict the other for secondary effects.

This is even more painful when the code you've arbitrarily forced to
only allow a single instance is in a library shared across multiple
teams. Now your library isn't only providing a service, it's also
dictating exactly how every client must use that service, regardless
of if that's the best way for each game.

### Lazy Initialization is Risky.

In the PC world of infinite virtual memory and loose performance
requirements, using lazy initialization is a pretty neat trick. It
doesn't translate so well to games, though. If creating your Singleton
audio system takes a few hundred milliseconds, initializing it will
cause visibly dropped frames and the game will stutter. You really
don't want to wait until the player is in the middle of a level and
causes the first sound to play when that happens.

Likewise, games generally need to closely control how memory is laid
out in the heap to avoid memory fragmentation (see Object Pool for
more on fragmentation). If your Singleton allocates a chunk of heap
when it initializes, you probably want to know exactly when that's
going to occur.

The get around this, most codebases I've seen implement the Singleton
pattern like this:

^code 5

<aside>

OK, that's not entirely fair. It does one nice thing for you: it
prevents you from re-assigning to your global variable.

</aside>

That solves the lazy initialization problem, but at the expense of
discarding everything about Singleton that *does* make it better than
a raw global variable. With the above, you can no longer use polymorphism, and the class must be constructible at static initialization time. All you've done is taken a global variable and
put a nice bow around it.

## Where Do We Go From Here?

If I've done my job, you'll think twice before you pull Singleton out
of your toolbox the next time you have a problem. But you still have
your problem. What tool *should* you pull out? In truth, sometimes
Singleton is the right choice, but based on what you're specifically
trying to address, I may have some other options for you.

### See If You Need The Class at All

<aside>

If you find yourself naming a class with "Manager", try try to come up
with a more descriptive name. We use metaphors to illuminate code by
describing it in terms of something concrete and familiar. If you name
a class "Manager" you're saying "this class does something like what a
manager does". What does that mean? It periodically checks on objects
to see if they're on schedule? It brings donuts to meetings?

Ask yourself what the class actually does and try to incorporate that
into the name. Does it *create* objects? Call it a "factory". Does it
hold a reusable pool of them? Call it a "pool". If you can't come up
with a concise description of what the class actually does, then you
may have worse problems than just a weak name.

</aside>

Before we go about solving your problem, let's make sure we can' just
erase it completely. More often than I'd like, when I see someone
using Singleton, it's to implement a "manager": that nebulous class
that babysits other objects. I've seen codebases where it seems like
*every* class had a manager: Monster, MonsterManager, Particle,
ParticleManager, Sound, SoundManager. Sometimes, for variety, they'll
throw a "System" or "Engine" in there.

While caretaker classes like that are sometimes useful, often they
just reflect unfamiliarity with OOP. Consider these two contrived
classes:

^code 8

Maybe this example is a bit dumb, but I've seen plenty of code that
after scraping away the crusty details reduces down to something like
this. If you look at this code, it's natural to think, "Well,
BulletManager needs to be a Singleton." After all, anything that has a
Bullet will need the manager too, and how many instances of
BulletManager do you need?

The answer is *zero*, actually. In OOP, objects are
supposed to take care of themselves. So the way to solve the "singleton" problem for our manager class is like this:

^code 9

There we go. No manager, no problem. Of course, this won't work in
every case. There are plenty of times where you do need a class that
"manages" others. A common example is an <a class="pattern" href="object-pool.html">Object Pool</a>. But, just for my peace of
mind, if you find yourself creating a "manager" type class, take a
moment to see if you need it at all or if your objects can get a
promotion and manage themselves.

### To Limit a Class to a Single Instance

This is the first half, and only the first half of what Singleton
solves. If you're writing a class that wraps an external API, it can
be critical to ensure there's only a single instance of that wrapper.

However, that doesn't mean it's your class's job to provide *access*
to that instance. It may be that the game wants to restrict access to
the single instance to certain areas of the code, or maybe even make
it entirely private to a single class (they may be wrapping your
wrapper in one of their own).

There are a couple of ways to accomplish this. Here's one:

^code 6

This class allows anyone to construct it, but will fail if you try to
construct more than one instance. This sidesteps the problems with
Singleton's lazy initialization, and also lets the calling code control where the instance lives and when it is created. It also lets
the calling code destroy the instance and then create another one
later.

The downside, and it may be a big one for you, is that the
check to prevent multiple instantiation is only done at *runtime*. The
Singleton pattern, in contrast, guarantees a single instance at
compile time, by the very nature of the class's structure.

We can get back our compile-time guarantee of single instantation by
going back to Singleton but splitting it into two classes:

^code 7

Our FileSystem class uses a private constructor to prevent arbitrary
code from creating instances of itself, but it doesn't actually create
any instances of its own. Instead, it declares a friend class, and
grants that special class the freedom to instantiate it.

The problem here, of course, is the our FileSystem class is still
coupled to the class that has an instance of it, but at least we're
no longer having the FileSystem grant access to an instance of itself
to *everyone*. In our example where we want to wrap the FileSystem
class in another wrapper, it encapsulates the one instance of
FileSystem completely, so that no other code can get at the unwrapped
FileSystem.

### To Provide Convenient Access to An Instance

This is the reason we reach for Singletons, they make it easy to get
our hands on an object we need to use in a lot of different places.
That ease comes at a cost, though: it's also easy to get our hands on
the object in places where we *don't* it being used.

In general, you want things to be as narrowly scoped as possible while
still getting the job done. The smaller the scope an object has, the
fewer places you need to keep in your head while you're working on it.
So, before we take the nuclear option of Singleton and give an object
*global* scope, let's see if there are any other smaller scopes that
will still reach all the places where we need our object.

Before we get into specifics, we need an example. Our Singleton
candidate will be a class for playing sounds. Since lots of places in
a game need to cause sound to play, we're going to need to make it
easy to get to. Here's the class:

^code 10

The question now is, how can we take advantage of the architecture of
the rest our game to give access to that without making it completely global? Consider a few different cases:

*   **If every place that needs it is derived from the same class**
    then we can let the base class hold the instance and give the
    derived classes access to it through a protected member. Like so:

    ^code 11

    If this looks like it could work for you, check out the
    <a class="pattern" href="sandbox-method.html">Sandbox Method</a>
    Pattern.

*   **If every place that needs it implements the same interface**
    then you can simply pass the instance in to the methods in that
    interface:

    ^code 12

    This is similar to the previous case, but is useful when the base
    class is a pure interface and you don't want it to hold any state.
    If this fits your design, see <a class="pattern" href="context-parameter.html">Context Parameter</a>.

*   Finally, **if it's needed in classes that aren't related, but
    those classes only care that the instance implements a certain
    interface** we can use a pattern called <a class="pattern"
    href="service.html">Service</a>. Briefly, it looks like:

    ^code 13

    The `IAudioPlayer` interface abstracts away the concrete
    `AudioPlayer` class. Any code that needs to play audio can get an
    instance of it through the static `Provider` class. Unlike
    Singleton which instantiates *itself*, the Provider class requires
    some external code somewhere to actually register the intance with
    it.

    This is still a bit undesirable because the Provider makes our
    audio player globally available, but it at least decouples the
    calling code from the concrete audio player class, and it lets
    our code control when the concrete audio player class is
    instantiated. If I can't reduce scope by actually passing around a
    reference to the instance I need, I reach for this pattern before
    I go for Singleton.

## What's Left?

<aside>

I do a lot of tool programming in C#, and I do find myself using C#'s
equivalent of the Singleton pattern pretty frequently there: static
constructors. A static constructor in C# will be called once for a
class, right before any static field is used. That basically is the
Singleton pattern, enshrined in the language.

</aside>

The question remains, where *should* you use the actual full Singleton
pattern? In my experience, the answer is *never* in a game. There are
other simpler patterns for guaranteeing single instantiation (often
simply using a static class is effective) and providing convenient
access to an instance. Many of those patterns are described here in
Section 3. The remaining feature of Singleton, lazy initialization,
is useful for PC software, but not something I've ever found myself
wanting in a game codebase.
