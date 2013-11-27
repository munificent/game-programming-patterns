^title Data Locality
^section Optimizing Patterns

## Intent

*Optimize processor cache utilization by arranging data in memory in the order that it is used.*

## Motivation

We've all been lied to. For years, we've been shown charts where CPU speed keeps going up and to the right. Moore's Law isn't just some historical observation, it's some kind of moral imperative. We software folks shouldn't have to lift a finger to see our programs magically speed up every year.

CPUs have been getting faster (though even that's plateauing now), but the hardware heads failed to mention a little something. Sure, we can process data faster than ever. But we can't *get* that data faster.

**TODO: graph of processor and memory speeds**

### A data warehouse

Before your super fast CPU can blow through a ream of calculations, it actually needs to get the data its working on out of main memory and into registers. It turns out that RAM hasn't been keeping up with increasing CPU speeds. Not even close.

With today's typical hardware, it can take *hundreds* of cycles to fetch a byte of data from <span name="ram">RAM</span>. If each instruction needs some data, and it takes hundreds of cycles just to get a byte of data for that instruction, how is that our CPUs aren't just sitting idle 99% of the time waiting for data?

Actually, they *are* stuck waiting on memory an astonishingly large fraction of time these days, but it's not as bad as it could be. To explain how, we're going to take a trip to the Land of Overly Long Analogies...

<aside name="ram">

It's called "Random access memory" because, unlike disc drives, you can theoretically access any piece of it as quick as any other. You don't have to worry about reading things consecutively like you do a disc.

Or, at least, you *didn't*. As we'll see, RAM isn't so random access anymore either.

</aside>

Imagine you're an accountant in a tiny little office. Your job is to request a box of papers and go through and do some <span name="accountant">accountant</span>-y stuff with them. Add up a bunch of numbers or something. You do this for specific labeled boxes according to some arcane logic that only makes sense to other accountants.

<aside name="accountant">

I probably shouldn't have used a job I know absolutely nothing about in this analogy.

</aside>

You've been working this job for a while and you've gotten pretty awesome at it. You can now finish off an entire box of paper in, say, a minute. Impressive, right? On a good day without too many coffee breaks, you should be able to get through about 480 boxes.

There's a little problem though. All of those boxes are stored in a warehouse in a separate building. To get a box, you have to ask the warehouse guy to bring it to you. He goes and gets a forklift and drives around the aisles until he finds the box you want.

It takes him, seriously, an entire day to do this. Not exactly a go-getter. He's the boss's son, so no one's gonna do anything about it. This means that no matter how fast you are, you only get one box a day. The rest of the time, you just sit there and question the life decisions that led to this soul-sucking job instead of following your passion for videogames.

One day, a couple of industrial designers show up. Their job is to improve the efficiency of operations. They make assembly lines and stuff go faster. The boss has noticed how few boxes get processed and he wants some improvement, so he brought in the experts.

After watching you work for a few days, they notice a few things:

* Pretty often, when you're done with one box, the next box you request is right
  <span name="next">next</span> to it on the same shelf in the warehouse.

* Using a forklift to carry a single box of papers is pretty dumb.

* There's actually a little bit of spare room in the corner of your office.

<aside name="next">

The technical term for often using stuff nearby the thing you just used is *locality of reference*.

</aside>

They come up with a clever fix. Whenever the warehouse guy gets a box, he'll
actually grab an entire pallet of them. He gets the box you requested, and then
a bunch of boxes that are next to it. He doesn't know if you want those (and, given his work ethic, clearly doesn't care). He just grabs them.

He loads the whole pallet and brings it to you. Ignoring issues of workplace safety, he drives the forklift right in and drops the pallet in the corner of your office.

When you need a new box, now, the first thing you do is see if it's already on the pallet in your office. If it is, great! It just takes you a second to grab it and you're back to crunching numbers. If a pallet holds fifty boxes and you got lucky and all of the boxes you need happen to be on it, you can churn through fifty times more work than you could before.

But, if you need a box that's *not* on the pallet, you're back to square one. You can only fit one pallet in your office, so your warehouse friend will have to come take that one back and then bring you entirely new one. Tomorrow.

### A pallet for your CPU

That's a pretty close <span name="analogy">analogy</span> for how CPUs in modern computers work. In case it isn't obvious, you play the role of the CPU. Your desk is the CPU's registers, and the box of papers is the data you can fit in them. The warehouse is your machine's RAM, and that annoying warehouse guy is the bus that pulls data from main memory into registers.

<aside name="analogy">

Why would it be in this chapter if it wasn't a good analogy?

</aside>

If I were writing this chapter thirty years ago, the analogy would stop there. But as chips got faster and RAM... didn't... the hardware guys started looking for solutions. What they came up with was *CPU caching*. Modern chips have a <span name="caches">little chunk</span> of memory right inside the chip. It's small because it has to fit in the chip. The CPU can pull data from this much faster than it can main memory in part because it's physically closer to the registers. The electrons have a shorter distance to travel.

<aside name="caches">

Modern hardware actually has multiple levels of caching, which is what they mean when you hear "L1", "L2", "L3", etc. Each level is larger but slower than the previous. For this chapter, we won't worry about the fact that memory is actually a hierarchy, but it's important to know.

</aside>

This little chunk of memory is called a *cache* (in particular, the chunk on the chip is your *L1 cache*) and in my belabored analogy, its part was played by the pallet of boxes. Whenever your chip reads a byte of data from RAM, it automatically grabs the following chunk of contiguous memory -- usually around 64 to 128 bytes -- too and puts it all in the cache. This strip of contiguous memory is called a *cache line*.

If the next byte of data you need happens to be in that chunk, the CPU reads it straight from the cache, which is *much* faster than hitting RAM. When it looks for a bit of data and finds it, that's called a *cache hit*. If it can't find it in the cache and has to go to main memory, that's a *cache miss*.

There's one important detail I glossed over in the analogy. In the accountant's office, there was only room for one pallet, or one cache line. In a real cache, it has room for a number of cache lines. It's a relatively smaller, fixed number, but it's more than *one*. The details about how those work is unfortunately out of scope here, but Google "cache associativity" if you want to feed your brain.

So, whenever the CPU needs some data, it looks to see if a cache line containing it is already in the cache. When that fails, and a cache miss occurs, the CPU *stalls*: it can't process the next instruction because needs data. So it just sits there, spinning its cycles all bored and lonely, for a few hundred cycles until the fetch completes.

Our mission for the rest of this chapter is to figure out how to minimize that happening. Imagine you're trying to optimize some performance critical piece of game code and it looks like this:

^code do-nothing

What's the first change you're going to make to that code? Right. Take out that pointless function, expensive call. That functional call is equivalent to the performance hit of a cache miss. Every time you bounce to main memory, it's like you put a `sleep()` call in your code.

### Wait, data is performance?

Now, I've known about CPU caching and optimizing for it for a long time. It's one of those ideas that I sort of absorbed through osmosis just by being around other programmers. But I didn't have any first-hand experience with it.

Most of my programming background is higher level: I've done gameplay, a bit of AI, and tons of UI, tools, and shared library kind of stuff. But I'm generally not one of those people who spends <span name="weeks">three weeks</span> squeezing another 3 FPS out of the rendering engine. I try to keep myself out of the hot path of the game loop most of the time.

<aside name="weeks">

I have a ton of respect for them, though!

</aside>

Don't get me wrong, I do care about performance too. So when I started to work on this chapter, I spend some time putting together little mini-game-like programs to try to trigger best case and worst case cache usage. I wanted to write benchmarks that would thrash the cache so I could see first-hand how much bloodshed it causes.

The first thing I learned is that it's surprisingly hard to get a clear window into what your cache is doing. Many basic profilers won't show it and since it's just memory access, it doesn't stick out in the profile. It just looks like every line of code is kind of slow.

When I finally got some stuff working, though, I was surprised. Even after hearing of how big a deal it is, there's nothing quite like seeing it with your own eyes. <span name="ymmv">I got a few programs</span> that did the *exact same* computation. The only difference was how many cache misses and memory thrashing they caused. The worst case was fifty *times* slower than the best case.

<aside name="ymmv">

There's a lot of caveats here. In particular, different computers have different cache setups so my machine may be different from yours, and dedicated game consoles are very different from PCs which are quite different from mobile devices.

Your mileage will vary.

</aside>

This was a real eye-opener to me. I'm used to thinking of performance being an aspect of *code* not *data*. A byte isn't slow or fast, it's just a static thing sitting there. But, because of caching, *the way you organize things directly impacts performance.*

The challenge for me now is to wrap that concept up into something that fits into a chapter here. This book tries to be about simple, concrete patterns. It's a recipe book for code.

But optimization for cache usage is a huge topic. I haven't even touched on *instruction caching*. Remember, code is in memory too and has to be loaded onto the CPU before it can be executed. Someone more versed on the subject could write an entire book on it.

Until you get your hands on that, though, I do think there are a few basic techniques that I can fit in here that will get you started along the path of thinking about how your data structures impact your performance.

What it all boils down to is something pretty simple: Whenever the chip reads some memory, it gets a whole cache line. The more you can use stuff in that cache line, the faster you go. So the goal then is to *organize your data structures so that the things you're processing are next to each other in memory*.

If your code is crunching on A then B then C, you want them laid out in memory like this:

    +---+---+---+
    | a | b | c |
    +---+---+---+

Note, these aren't *pointers* to A, B, and C. It's the data for them, right there, all lined up next to each other. As soon as the CPU reads in A, it will start to get B and C too (depending on how big they and how big a cache line is). Since you're working on those next, you're chip is happy and you're happy.

## The Pattern

Modern CPUs have **caches to speed up memory access.** These can access memory **adjacent to recently accessed memory much quicker**. Take advantage of that to improve performance by **increasing data locality** -- keeping data in **contiguous memory in the order that you process it.**

## When to Use It

Like most optimizations, the first guideline for using it is *when you have a performance problem.* There's no point wasting time applying this in code that's way off to the side of your core game loop. In fact, doing so has *negative* value. It makes your code more complex and less flexible.

With this pattern specifically, you'll also want to ensure you reach for it because you have performance problems *caused by cache misses*. If your code is slow for other reasons, this won't help. There are three golden rules for optimization:

1. Profile.
2. Profile.
3. Profile.

You'll need something a little more sophisticated than manually instrumenting your code with some profile hooks that just see how much time has elapsed. You really need to see how many cache misses are occurring and where. Fortunately, CPUs expose this information and there are profilers out that there can access it. Also, tools like cachegrind can run your code in a way that simulates the cache and report cache misses.

It's worth spending the time to get one of these working and make sure you understand the (surprisingly complex) numbers it gives you in return. You really don't want to be fumbling around in the dark here. This pattern may cause you to do some major surgery on your data structures. You need a reliable heart monitor so you can ensure you've actually helped your patient.

That being said, designing your data structures to be cache friendly can have speeding affects on your program. Cache misses *will* affect the performance of your game, so it's good to be thinking about how cache-friendly your program is throughout the design process.

## Keep in Mind

Many optimizations sacrifice flexibility for speed. We use things like interfaces to insulate parts of the codebase from each other. That makes it easier to change one of these parts without affecting the others. The cost is that code on either side of the interface can make fewer assumptions about what the other side is doing. That's the *point*: assumptions are coupling.

Performance is often about making things *concrete*. Many optimization start with "if we assume X then we can...". Optimizations thrive on specifics. This pattern in particular fights against our desire for abstraction. In C++, using interfaces implies accessing objects through <span name="virtual">pointers or references</span> to objects. But going through a pointer often means hopping across memory, which causes the exact cache misses this pattern tries to avoid. You'll be sacrificing some of your precious interfaces to please this pattern.

<aside name="virtual">

The other half of interfaces is *virtual method calls*. Those require the CPU to look up an object's vtable, and then find the pointer to the actual method to call there. So, again, you're chasing a pointer and often causing a cache miss, though this time it's an *instruction* cache miss, since you're loading code onto the CPU.

</aside>

Some people feel focusing on data locality also goes against the grain of object-orientation. In their minds, OOP is about each *object* taking care of itself. As we'll see, this pattern thrives on dealing with *collections* of homogenous objects.

Personally, I think OOP is more about each *class* taking care of its instances, so I believe the nice things we get from OOP like data hiding still apply here. There may be times where you need to open up an object's internal structure more than you would like to please outside code wanting to get at its data directly in a certain order, but I'm not enough of an OOP purist to be really bothered like that. As always, there are challenging trade-offs. That's what makes it fun.

## Sample Code

OK, it's time for a confession. My goal for this part of the chapter was to show you a little skeleton of a game loop that was processing a bunch of game objects. I was going to show a complete example that demonstrated worst case cache usage so you could run it and see for yourself. Then we'd optimize so you could see the difference.

What I got instead was a lesson in how tricky controlling cache usage is. Every time I tried to rearrange my sample program into pieces that made sense in the context of the chapter, the cache usage changed dramatically. Cache usage is *highly* context dependent. Even the order that functions are written in your source file can affect things. This means that a piece of code can have different caching behavior just by putting it in a program with other code.

Instead of trying to shoehorn a teaching sample and a tight benchmark into one single example program, I'm going to do something I think is ultimately more useful. Instead of a monolithic example, we'll walk through a few small examples each showing one common technique used to make code more cache friendly.

### Contiguous arrays

Let's start with the quintessential example of re-organizing a game architecture to be more cache friendly. We'll start with a typical <a href="game-loop.html" class="pattern">Game Loop</a> that processes a bunch of game entities that are organizing using the <a href="component.html" class="pattern">Component</a> pattern.

Each game entity has components for AI, physics, and rendering. So you have a game entity something like this:

^code game-entity

Each component will have a relatively small amount of state, maybe little more than a few vectors or a matrix, and then a method to <span name="update">update</span> it. The details aren't important here, but imagine something roughly like:

<aside name="update">

Like the name implies, these are a few examples of the <a href="update-method.html" class="pattern">Update Method</a> pattern. Even `render()` is this pattern, just by another name.

</aside>

^code components

The game maintains a big list of pointers to the entities in the world. Each spin of the game loop, this needs to happen:

1. Update the AI components for all of the entities.
2. Update the physics components for them.
3. Render them using their render components.

Lots of game engines implement that something like this:

^code game-loop

Before you ever heard of a CPU cache, this looked totally innocuous. But by now you've got an inkling that something isn't right here. This code hates the cache. It is spitting in the cache's coffee. Here's what it's doing:

1. We've got a list of *pointers* to game entities. For every game entity in the world, we have to traverse that pointer. That's a cache miss.

2. Then the game entity has a pointer to the component. Another cache miss.

3. Then we update the component.

4. Now we go back to step one for *every component of every entity in the game*.

The scary part here is we have no idea how any of this stuff is laid out in memory. We're completely at the mercy of the memory manager and the order that things happened to be allocated, which, after the game has been running for a while, is anything but clear cut.

**TODO: illustration**

If our goal here was to take a whirlwhind tour around the game's address space, sort of a 256MB of RAM in 4 Nights!" cheap European vacation package, it would be a fantastic deal. But our goal is to run the game quickly, and <span name="chase">traipsing</span> all over main memory is *not* the most effective way to do it. Remember that `doAbsolutelyNothingFor500Cycles()` function? Well this code is calling that *all the time*.

<aside name="chase">

The term for burning cycles traversing pointers is *pointer chasing*, which isn't quite as fun as it sounds.

</aside>

Let's do something better. Our first observation is that the only reason we even follow a pointer to get to the game entity is so we can get to a component. `GameEntity` itself has no interesting state and no useful methods. The components are what the game loop cares about. Game entities are just a means to that end.

So lets cut out the middle man. Instead of a huge tree of game entities and components scattered like stars across the inky darkess of address space, we're going to get back down to Earth. We'll have a big array for each type of component. A flat array of AI components, another for physics, and another for rendering.

Like this:

^code component-arrays

Let me just stress that these are arrays of *components* and not *pointers* to them. The data is all there, nicely lined up. The game loop will then walk these directly:

^code game-loop-arrays

We've ditched all of that <span name="arrow">pointer chasing</span>. Instead of skipping around in memory, we're doing a straight crawl straight through contiguous arrays. We're pumping a solid stream of bytes right into the hungry maw of the CPU. In my little synthetic benchmark programs, this made the update loop fifty *times* faster than the previous example.

**TODO: illustration**

<aside name="arrow">

One hint that we're doing better here is how few `->` operators there are in the new code.

</aside>

Interestingly, we haven't lost much encapsulation here. Sure, the game loop is going straight to the components instead of getting them from game entities. But it was doing that before. It needed to to ensure things were updated in the right order. But each component itself is still nicely encapsulated. It owns its own data and methods. We just changed the way it's used.

This doesn't mean we need to get rid of `GameEntity` either. We can leave it just as it is with pointers to its components. They'll just point into those arrays. This can still be useful for other parts of the codebase where you want to pass around a conceptual "game entity" and everything that goes with it. The important part is that the performance critical game loop sidesteps that and goes straight to the data.

### Packed data

Say we're doing a particle system. Following the advice of the previous section, we've got all of our particles in a nice big contiguous array. Let's wrap it in a little <span name="pool">manager class</span> too:

<aside name="pool">

This is a great example of an <a href="object-pool.html" class="pattern">Object Pool</a> custom built for a single type of object.

</aside>

^code particle-system

A rudimentary update method for the system just looks like this:

^code update-particle-system

But it turns out that we don't actually need to process *all* of them all the time. Sometimes particles are disabled, deactivated, culled, offscreen, or otherwise temporarily out of commission. The easy answer is something like this:

^code particles-is-active

For every particle, we have to check that flag before we update it. (We could move the check inside `update()` but that doesn't actually make a difference.) That probably loads the whole particle into the cache. If the particle *isn't* active, then we just skip over it to the next particle. So loading that particle into the cache was a waste of time.

The more inactive particles there are, the more we're <span name="branch">skipping across memory</span>. The faster we do that, the more cache misses there are between actually doing useful work updating active particles. If that array is large and has *lots* of inactive particles in it, we're back to just thrashing the cache again.

Having objects in a contiguous array doesn't solve everything if the objects we're actually processing aren't contiguous in it. If it's a foam of inactive objects we have to skip past, we're right back to the original problem.

<aside name="branch">

Savvy low-level coders are probably aware of another problem here. Doing an `if` check for every particle can cause a *branch misprediction* and a *pipeline stall*. In modern CPUs a single "instruction" actually takes several clock cycles. To keep the CPU busy, instructions are pipelined such that the next instructions start processing before the previous one finishes.

To do that, the CPU has to guess which instructions it will be executing next. In straight line code, that's easy, but with flow control, it gets harder. While it's executing the jump instruction for the `if`, does it guess that the particle is active and start executing the code for the `update()` call, or does it guess that it isn't?

To handle this, chips do *branch prediction*: they see which branches your code tends to take and guess that it will do that again. But when the loop is constantly toggling between particles that are and aren't active, that prediction will fail.

Every time it does, it has to ditch the instructions it had started speculatively processing and start over after the first jump instruction is done. The performance hit of this varies widely by machine, but this is why you'll see some coders avoid flow control in hot code.

</aside>

Given the subtitle you just read a minute ago, you can probably infer the solution. Instead of *checking* the active flag, we'll *sort* by it. We'll keep all of the active particles in the front of the list. We can also easily keep track of how many active particles there are. Then our update loop is beautiful:

^code update-particles

Now we aren't skipping over any data. Every byte that gets sucked into the cache is a piece of an active particle that we actually need to process.

Of course, I'm not saying you should actually quicksort the entire collection of particles every frame. That would more than eliminate the gains here. What we want to do is *keep* the array sorted.

Obviously, the only time it can become less than perfectly sorted is when a particle has been activated or deactivated. We can handle those two cases pretty easily. When a particle gets activated, we move it up to the end of the active particles by swapping it with the first *in*active one:

^code activate-particle

To deactivate a particle, of course, we just do the opposite:

^code deactivate-particle

Lots of programmers (myself included) have developed allergies to moving things around in memory. Schlepping a bunch of bytes around *feels* heavyweight in some sense. Pointers feel lightweight in comparison. But when you add in the cost of *traversing* that pointer, it turns out that your (well, at least my) intuition isn't right on modern hardware any more. In <span name="profile">many cases</span>, it's cheaper to actually move things around in memory so that you can keep the cache full.

<aside name="profile">

This is your friendly reminder to *profile* when making these kinds of decisions.

</aside>

There's a neat consequence of keeping the particles *sorted* by their active state. We no longer need to *store* the active flag at all. It can be determined entirely by its position in the array and the `numActiveParticles_` counter. That's good: it makes our particle objects smaller, which means we can pack more in our cache lines. And that makes them even faster.

It's not all rosy, though. As you can see from the API, we've lost a bit of OOP flavor here. You can no longer just call some `activate()` method on the `Particle` itself since it doesn't know it's index. Instead, the particle *system* has this responsibility.

In this case, I'm OK with `ParticleSystem` and `Particle` being tightly tied like this. I think of them as a single *concept* spread across two physical *classes*. It just means accepting the idea that particles are *only* meaningful in the context of some particle system.

### Hot/cold splitting

OK, this will be the last example of a simple technique for making your cache happier. It's a bit like a finer-grained manifestion of the previous idea. Say we've got an AI component for some game entity. It has some state in it: the animation it's currently playing, a goal position its heading towards, energy level, etc.. Stuff it checks and tweaks every single frame. Something like:

^code ai-component

But it also has some state for rarer eventualities. It may need to store some data describing what loot it drops when it gets 86'd. That state will only ever be used once by the game entity's lifetime, right at the bitter end.

^code loot-drop

Assuming we've already followed the earlier patterns, when we update these AI components, we'll be walking through a nice packed array of data. But that data includes all of the loot drop information. That makes each component bigger, which reduces the number of them we can fit in a cache line. We get more cache misses because the total memory we walk over is larger. That drop data gets pulled into the cache for every component, every frame, even though we aren't even touching it.

One solution for this is called "hot/cold splitting". The idea is to break our data structure into two separate pieces. The first holds the "hot" data: the state we need to touch every frame. The other piece is the "cold" data: everything else that gets used less frequently.

The hot piece is the *main* AI component. It's the one we need to use the most, so we don't want to hide it behind an indirection. The cold component can be off to the side, but we still need to get to it, so we give the hot component a pointer to it, like so:

^code hot-cold

Now when we're walking the AI components every frame, the only data that gets loaded into the cache is what we really, with the <span name="parallel">exception</span> of that one little pointer to the cold data. But that's still a real improvement over having all of that cold data right there in the way.

<aside name="parallel">

We could conceivably ditch the pointer too by having parallel arrays for the hot and cold components. Then we can find the cold AI data for an actor as long as we know its index since both pieces will be at the same index in their respective arrays.

</aside>

You can see how this starts to get fuzzy though. In my example here, it's pretty obvious which data should be hot and cold, but it's rarely so clear cut. What if you have fields that are used when an entity is in a certain mode but not in others? What if entities use a certain chunk of data only when they're in certain parts of the level.

Doing this kind of optimization is somewhere between a black art and a rathole. It's easy to get sucked in and spend endless time pushing data around to see what speed difference it makes. It will take some practice to get a handle on when and where it's worth it to focus on this stuff.

## Design Decisions

This pattern is really about a mindset: it's getting you to think about where your data is in memory as a critical piece of your game's performance story. The actual concrete design space is wide open. You can let <span name="dod">data locality</span> affect your whole design philosophy, or maybe it's just a localized pattern you apply to a few critical data structures.

The biggest question you'll need to answer is when and where you apply this pattern. But here's a few more concrete ones that may come up, especially if you're using this on your main game entity type.

<aside name="dod">

Noel Llopis' [famous article](http://gamesfromwithin.com/data-oriented-design) that got a lot more people thinking about designing games around data locality and cache-friendliness calls this "data-oriented design".

</aside>

### How do you handle polymorphism?

One thing I've dodged in this chapter so far is subclassing and virtual methods. We've been assuming we've got nice packed arrays of *homogenous* objects. That way, we know they're all the exact same size. But polymorphism and dynamic dispatch are useful tools, too. How do we merge them?

**Don't:**

The simplest answer is to just avoid subclassing, or at least avoid it in places where you're optimizing for cache usage. Software engineer culture is drifting away from heavy use of inheritance anyway.

  * *It's safe and easy.* You know exactly what class you're dealing with and all objects are obviously the same size. If you don't *need* polymorphism, don't use it.

  * *It's faster.* Dynamic dispatch means looking up the method in the vtable and then traversing that pointer to get to the actual code. That can be an *instruction* cache miss. While overhead for this varies widely across different hardware, there is a cost to dynamic dispatch.

  * *It's inflexible.* Of course, the reason we use dynamic dispatch is because it gives us a powerful way to make a bunch of different objects behave differently. If you want different entities in your game to have their own AI styles, their own special moves and attacks, virtual methods are a nice way to model that. Having to instead stuff all of that code into a single non-virtual method that does something like a switch on the AI type gets messy quickly.

**Use separate arrays for each type:**

We use polymorphism so that we can invoke behavior on an object whose type we don't know. In other words, we have a mixed bag of stuff and we want each object in there to do its own thing when we tell it to go.

But that just raises the question of why mix the bag to begin with? Instead, why not just maintain separate homogenous collections for each type?

  * *It keeps objects tightly packed.* Since each array only contains objects of the same type, there's no padding or other weirdness.
  * *You can statically dispatch* Once you've got objects partitioned by type, you don't actually need polymorphism at all any more. You can use regular non-virtual method calls.
  * *You have to keep track of a bunch of collections.* If you have a lot of different object types, the overhead and complexity of maintaining separate arrays for each can be a chore.

  * *You have to be aware of every type*. Since you have to maintain seprate collections for each type, you can't be decoupled from them. Part of the magic of polymorphism is that it's *open-ended*: code that works with an interface can be completely decoupled from the potentially large set of types that implement that interface.

**Use a collection of pointers:**

If you weren't worried about caching, this is the natural solution. Just have an array of pointers to some base class or interface type. All the polymorphism you could want, and objects can be whatever size they want.

  * *It's flexible.* The code that consumes the collection can work with objects of any type as long as it supports the interface you care about. It's completely open-ended.
  * *It's less cache-friendly.* Of course, the whole reason we're discussing other options here is because this means cache-unfriendly pointer indirection. But, remember, if this code isn't performance critical, that probably doesn't matter.

<!--

### What is granularity of a class?

A lot of OOP pedagogy teaches that objects that be as fine-grained and narrowly focused as possible. You hear people spouting craziness like a class shouldn't be longer than a page of code. Conceptually large objects end up being made of just a couple of smaller ones that are in turn made of smaller ones ending in some fractal cloud of tiny objects.

This is great for separation of concerns, but it's not so nice for keeping things straight in memory. We don't have to give up encapsulation, but sometimes it makes more sense to define your class as representation an *aggregation* of objects. Instead of a particle class, you have a particle *system* class.

**If a class represents an individual object:**

  * *It's how we've been trained to model things.* It lets us apply all of powerful abstraction and encapsulation tools OOP hands us. Smaller, more isolated objects are easier to reason about and modify.

  * *It's adequate for most of the codebase.* I find optimization short-circuits some weird corner of my brain. Once I start doing it, I start seeing *everything* as an optimization problem to be solved. The next thing I know, I've sorted and packed my entire sock drawer.

  The truth is, 90% of the codebase is nowhere near performance critical. Time spent optimizing that stuff is time *not* spend making a better game.

**If classes represent:**

  * *for lightweight "dumb" objects, don't lose much by having object that represents group of them.* things like particles where behavior of every particle is conceptually same are just as easy to reason about in class that updates a pile of them as they are in class that only deals with one.

  * *lets manager completely control memory for objects.* weird that with most objects, they don't encapsulate their own memory management. [operator new] memory has to be given to them before they come to life. to keep stuff in right place in memory, you need some manager *anyway*, so it may as well manage the objects as well as their memory.

  * *lets you control access to objects*. by hiding individual objects behind manager, you can prevent outside code from getting raw pointer to them. very important if you are sorting, packing or otherwise moving in memory: doing so would break any existing pointers.

**both:**

final option is to do both: have class for manager and class for individual objects. if doing manager, will end up doing this in some way anyway. at very least, will have dumb struct for individual object. once have that, have option of putting some methods there too for things that only operate on single object.

classes will likely end up being very closely tied to each other. friend in c++.

  * *gets you back some of encapsulation of individual objects.* at least some of behavior can be scope to single object.

  * *most complex.* have two full classes. have to decide which has which parts of responsibility.

-->

### how are actors defined?

if using nice contiguous arrays of components and iterating them directly,
actual actor object is less important during core game loop. still useful in other places in codebase that want to work with single logical "actor".

question is how should it be represented? how does it track its components?

**inline:**

if actor has components as actual fields and not pointers to components (or not using explicit component pattern at all), then data stored inline in actor object. in other words, not using optimizing components in memory for updating.

  *can move entire actor in memory.* since actor and its components are now all
  one contiguous obj, can move whole thing around without worrying about fragmenting it.

  *faster to access all actor data.* splitting actors into components optimizes for use case where you access all components of bunch of actors at same time. but if your access pattern is for touching all data for one actor at same time, this better. localizes actor data together.

  *simpler, smaller:* don't have to manage memory for pieces of actor separately. don't waste space storing pointers.

**pointers:**

kind of typical oop solution. actor has raw pointers to its components.

  *can store components in contiguous arrays.* since actor no longer cares where component is,
  can actuall physically be in obj pool, so iterating over components is nice to cache. at same time, actor can still easily get to its components.

  *makes moving components hard.* if sorting or packing components in memory to keep cache filled, pointers in actor will break if you're not careful to update them.

**handles or smart pointer:*

more refined solution is some kind of "handle" or custom pointer type. still references component stored elsewhere, but can include some additional tracking info. in particular, lets you keep track of all existing pointers into some component.

  *more complex*: smart pointers aren't rocket science, but aren't trivial either.
  *can handle moving components*: if have way of finding all handles to some component, can move component in memory and have handles update to new location.

      **Todo: code**

**actors as id:**

new style some engines use. imagine have contiguous arrays for each component type. now say components are stored in same order in each array. so ai component for some actor will be in third slot in ai component array, and render component for that actor will be in third slot in render component array, etc.

at this point, number 3 is all you need to find all components for an actor. so can turn actor class into just a simple id. becomes tiny! to get component for some actor, pass it (i.e. actor itself) to manager for some component. it uses id to look up index in array.

  *actors are tiny* just little ids, no pointers or anything.

  *actors are tiny*. downside is can't store other data on actor. really have to double down
  on pushing things into components.

  *works best when all actors have same set of components.* good thing about component pattern is allows mix-and-match. invisible object can just not have render component. immovable object can not have ai component. but if id describes index in parallel component arrays, those arrays need to be parallel to keep things lined up.

  if actor doesn't have render component, still have to claim a slot in array so that later actors render components are in right position. can have inactive components to handle this. but means update loop will have to skip over memory for unused components.

  *can't sort components independently.* since single id identifies component for actor in all arrays, can't sort one component array to keep things packed without sorting others in parallel. if, for example, active states for different components vary, can't keep each one sorted according to its own needs. very tighltly parallel arrays.

  *can't move components* of course, not only can't move them independently, can't move them at all. since actor id is direct index into array, moving component invalidates id. instead,
  engines keep track of unused ids. when actors are added and removed, instead of moving others around, just allows holes for unused ids in list and fills them in later.

  see object pool for more details.

## See Also

* as you can see from example, pattern goes hand in hand with components.
  since actors update one domain at a time, splitting into components also
  conveniently lets you slice an actor into pieces that you can order to be
  cache friendly.

  don't need components to use this! even code totally unrelated to main game
  objects can still benefit from cache opt.

* classic presentation that got lot of people more aware of this is: http://research.scee.net/files/presentations/gcapaustralia09/Pitfalls_of_Object_Oriented_Programming_GCAP_09.pdf

* around the same time, very influential blog post http://gamesfromwithin.com/data-oriented-design

* pattern usually takes advantage of "flat array of objects of same kind". if
  need to create and destroy them while doing that, see "object pool".

* artemis (http://gamadu.com/artemis/) is well-known framework using pure-id entities

## random notes

used to bit pack because memory was limited. then had enough memory that speed
mattered more and decoding was too expensive. now memory is effectively expensive again (have plenty, but slow to get to it), so worth it to bit pack again.

talk about how sometimes you *don't* want stuff on one cache line. if accessing on multiple threads, may be better to split.
