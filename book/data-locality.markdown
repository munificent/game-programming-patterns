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

Most of my programming background is higher level: I've done gameplay, a bit of AI, and tons of UI, tools, and shared library kind of stuff. But I'm generally not one of those people who spends three weeks squeezing another 3 FPS out of the rendering engine. I try to keep myself out of the hot path of the game loop most of the time.

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

Modern CPUs have **caches to speed up memory access.** These can access memory **adjacent to recently accessed memory much quicker**. Take advantage of that to improve performance by organizing data in **contiguous memory in the order that you process it.**

## When to Use It

have code that's performance critical

touches a lot of data

accessing main memory is slow

burning lots of cycles on data cache misses. profile profile profile!

in other words, make sure you have a real problem and you're certain problem
is coming from cache. as we'll see, this pattern can be major architectural
surgery. don't want to operate unless patient is really sick.

(other way to think about it is good to keep this pattern in mind throughout
project so don't have to make sweeping changes later for it.)

## Keep in Mind

many optimizations sacrifice abstraction for speed. abstraction is about
building interfaces to obscure what's going on. that decoupling makes it
easier to change things without affecting other stuff. abstraction about
generalization, about being flexible by not making assumptions.

perf is often about the concrete. opt often starts with "assuming we only need
x" then takes advantage of that. thrives on specifics.

### Anti-OOP?

this pattern in particular fights against encapsulation. in c++ going through
interfaces implies pointers and references. but this pattern is about putting
object in memory *here*, not *pointer to it*.

[virtual methods are other half of encapsulation. those cause similar caching
problems with i cache.]

to give cpu lots of data to chew, often means large collections of objects.
to have contiguous, usually need to be same size. means it works best with
objects of same type so they are same size. goes against subclassing where
objects may be different sizes.

### Anti-object but pro-class?

oop is about treating object as independent actor that owns state and behavior.
naturally optimized for thinking of objects as owners of their own destiny
and singular.

this pattern really about aggregates. associates behavior with *collection*
of objects.

may feel unintuitive.

think we'll see that most of oop principles like data hiding are still there.
more about hiding data in *type*, not *instance of type*, which is actually
how priv in c++ works anyway.

**todo: above para may not pan out**

**todo: subheader?**

pattern often means moving stuff around in memory to keep stuff contiguous.
when moving stuff, have to be careful about pointers to things you're moving.
this is *not* easy pattern to apply.

## Sample Code

confession time. wanted to show a single example here that was skeleton of game
loop processing a bunch of components. was going to show complete worst case
example with lots of pointer chasing, stuff random in memory. then show
optimized version so you could run yourself and see perf difference.

ended up being lesson in how tricky this opt. optimizing for cache *highly*
context-dependent. used to thinking that code composes. if you have piece of
code exhibits a and other exhibits b, slap together exhibits a and b.

but caching relies on big hidden complex chunk of mutable state (cpus caches
and the mapping it uses from memory to cache, prefetching logic etc.) means
two unrelated pieces of code can have wildly different cache effects when put
in program together.

good reminder of golden rule of programming: profile in the context of your
actual program.

managed to get some benchmarks that showed drastic effects on my computer, but
every time tried to put them into form that made sense interleaved with prose
in chapter, effects dimished. (not disappear! still could manifest 5x worse
perf.)

instead of trying to shoehorn both tight benchmark and good
teaching example into one program, doing something ultimately better. instead of single monolithic example, do a few
different smaller examples showing some of various techniques commonly applied
to make code more cache friendly.

### contiguous arrays

typical game loop using component pattern. have bunch of actors. each actor has
ai, physics, and render component. here's what *needs* to happen (in order):

1. update all of ai components.
2. update all of physics components.
3. render all render components.

lot of game engines look like:

    naive pointer chasing

we are murdering cache here. after running this code, cache actively hates us.
spitting in coffee. here's what doing wrong.

1. iterating over list of actors. list of pointers to actors.
2. for each actor, traverse pointer. CACHE MISS.
3. now look up physics component in actor.
4. that's pointer too. traverse CACHE MISS.
5. update physics component.
6. go all the way back to list of actor pointers and start again.

since actors and components are pointers, could be scattered all over memory.
especially since actors created destroyed during game, often are arranged
randomly.

observe: only reason we look up actor is to get to component. actor itself has
no interesting data. also, order we update components doesn't usually matter.
sure, can make difference in game, but not usually one that matters to player.

instead of huge tree of actors and components spread over memory, simplify:
one array of components for each type. big array of ai, physics components, etc.
not array of pointers, array of actual objects. not linked list. ARRAY.

game loop tracks arrays directly. when it updates ai, just walks ai component
array and updates each one in turn. no pointer chasing. no skipping around
memory. pumping delicious bytes of data directly into cpu's hungry maw. happy
cpu.

do this for each component type. then done. in little synthetic benchmark,
made 50x diff.

interestingly, note that this doesn't break encapsulation. each component still
owns its data and behavior. data is still private. just changed way its used.

actor can even maintain pointers to its components. if you have actor and need
to get to component from it, can still have that capability. important part is
that hot processing loop can sidestep and go straight to data.

### packed data

ok, so you reorganize codebase around above pattern. makes lots of sense when
you have fixed set of actors, but actors come and go. maybe actors far from
player are deactivated and don't get processed.

end up doing

    iterate over components and check is active

two problems. first is now we have to look up actor and check flag. constantly
switching between reading memory from actor and the memory from component.
thrash cache.

second problem is when actor is inactive, end up skipping over component data.
with lots of inactive actors, constantly blowing cache skipping through memory
looking for something that actually needs updating.

solution is instead of *checking* active flag, *sort* by it. keep all active
actors in front of list. then track how many there are. update loop is simple

    loop active components

trick is keeping sorted. actors change active state all time. to keep memory
packed, can do this:

    1. when actor is activated, swap memory for its component with first
    inactive component in list, the inc number of active actors.
    2. when deactivated, swap memory for its component with last active component
    then dec num

lots of programmers allergic to moving stuff in memory. feels heavyweight.
but move is just read and write. not much slower than just reading it, which
do every frame. if active states change less often than you update, then
probably quicker to take hit of moving things to keep sorted if it keeps update
loop fast. profile!

### hot/cold splitting

this is a bit like a finer-grained manifestation of previous example. say we've
got ai component for actor. has a few fields for basic pathfinding, and which
target actor is heading towards. stuff it uses every single frame.

but also some ai data for less common scenarios. maybe has some data to store
what item it drops when killed. that state will only come in to play once.

if updating bunch of actors ai, walking through nicely packed components, but
still wasting a bunch of time skipping over state we aren't using every frame.
all of data for drop is getting loaded into cache for no reason. end up getting
more cache misses since data we do care about is more spread out.

solution is called hot/cold splitting. slice ai component into two structures.
first holds "hot data": fields we need every frame. second is cold data.

hot component holds pointer to cold one. when we need cold data, can get to it
through that. other wise, only have to skip over pointer when iterating over
components. (if using parallel arrays like first example, could even possible
ditch pointer.)

can see how this starts to get fuzzy. deciding which data is hot and cold
depends a lot on game and how it gets used. more art than science. maybe not
even art. crapshoot?

## Design Decisions

pattern is really about a mindset: getting you to think about where stuff is in
memory as a critical piece of performance story. actual concrete design space
is wide open. can affect every corner of codebase and architect deeply around
this ("data-oriented design"). or maybe just local opt you do for some key stuff
like render loop and particles.

### what is granularity of classes?

lot of oop teaches us to define small fine-grained classes with has-a references to other objects. get bit trees or graphs of tiny objects scattered in memory. nice for separation of concerns and reuse. not nice for keeping things contiguous.

don't have to give up encapsulation, but sometimes makes more sense to think of class as representing collection of homogeneous objects. instead of particle class, have particle *system* class. often what's going on when you see "manager", "system", or "pool": single instance of class that represents large number of things.

**class = individual object:**

  * *how we've been trained to do object modeling.* lets you bring into play all of powerful abstraction and encapsulation tools oop gives us. makes it easier to reason about behavior and state of small individual entities in isolation. while not perfect, reason this is most successful paradigm in world.

  * *perfectly adequate for most of codebase.* opt triggers some weird flaw in brain where once start worrying about perf, hard to *stop* wanting to optimize everything. get obsessed.
   but opt isn't without cost. get too sucked into perf mindset and can end up sacrificing all manner of things at that altar. like maintainability and flexibility. takes time too. dumb to burn time optimizing 80% of codebase that is not perf critical. your menu screens probably don't need to worry about this.

**manager objects:**

  * *for lightweight "dumb" objects, don't lose much by having object that represents group of them.* things like particles where behavior of every particle is conceptually same are just as easy to reason about in class that updates a pile of them as they are in class that only deals with one.

  * *lets manager completely control memory for objects.* weird that with most objects, they don't encapsulate their own memory management. [operator new] memory has to be given to them before they come to life. to keep stuff in right place in memory, you need some manager *anyway*, so it may as well manage the objects as well as their memory.

  * *lets you control access to objects*. by hiding individual objects behind manager, you can prevent outside code from getting raw pointer to them. very important if you are sorting, packing or otherwise moving in memory: doing so would break any existing pointers.

**both:**

final option is to do both: have class for manager and class for individual objects. if doing manager, will end up doing this in some way anyway. at very least, will have dumb struct for individual object. once have that, have option of putting some methods there too for things that only operate on single object.

classes will likely end up being very closely tied to each other. friend in c++.

  * *gets you back some of encapsulation of individual objects.* at least some of behavior can be scope to single object.

  * *most complex.* have two full classes. have to decide which has which parts of responsibility.

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
