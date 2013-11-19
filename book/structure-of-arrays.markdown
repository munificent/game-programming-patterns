^title Structure of Arrays
^section Optimizing Patterns

## Intent

*Optimize processor cache utilization by organizing data in memory in the order that its processed.*

## Motivation

been lied to

told that moore's law has been going strong for decades

every year our code all gets faster without any work

but the hardware folks left a little something out

sure, can process data much faster

but can't *get* data faster

*graph of processor and memory speeds*

## data warehouse

cpu instructions operate on data, but data is coming from memory

before can start doing work, have to pull data out of ram and into registers

turns out, ram hasn't been keeping up with chip

can now take hundreds of cycles to fetch byte of data from ram

long analogy time

imagine you have little office, job is to take box of papers and go through
and mark them up or whatever

been doing job a while and getting really fast at it

can now do entire box in like a minute, awesome

so can get through 8 * 60 boxes a day, right?

problem: boxes are all in shelves in warehouse

to get a box, have to ask warehouse guy to bring it to you

he gets forklift and goes and finds box

takes him an entire day, seriously, to bring you a box

yeah, not sure how he gets away with slacking this much

doesn't matter how fast you are, you only get one box a day, so you can only

process on box a day

industrial designers obviously not happy about this

notice a couple of things:
- pretty often, when you're done with one box, the next box you want is right
  next to it on the shelf in the warehouse
- using a forklift to carry a single box is pretty dumb

come up with clever fix

whenever warehouse guy gets box, he'll actually bring a pallet of them

gets the box you asked for and then grabs a bunch of boxes next to it

loads the whole pallet and brings it to you

just room enough in your little office for the pallet

when you finish with one box, if the next one you need is on the palette, you

can just grab it. takes you a few minutes, but nowhere near as long as getting

it from the warehouse

if pallet holds twenty boxes, you can do twenty boxes a day now. 10x better!

## ahead of the game

id folks notice one more thing

now that you are kept busy for longer while working on entire pallet of boxes,
warehouse guy spends a lot of time waiting for orders. doesn't know what to
bring you until you finish all of your boxes and ask for the next one

they suggest "pre-fetching". after he brings you pallet of boxes, immediately
starts getting the pallet of boxes next to it

if you end up asking for something else, he'll stop doing that, but if you
ask for something that is on that pallet, he'll be halfway done by the time
you ask for it

if you happen to need just the right set of boxes, you can get the whole
pipeline moving fast enough that youre almost always busy

## cpu cache

very close analogy for how cpus work today. (why would it be in chapter if
wasn't good analogy?)

warehouse is main memory. fetching data from it is slooooow.

box is amount of data that fits in registers

but cpus now have a cache. small chunk of mem faster to access than ram.
fast because on chip. literally in area of computers where physics comes
into play. faster because electrons have less distance to travel.

in analogy, cache is pallet of boxes you can fit in your office

when cpu requests byte from main memory, it automatically grabs a chunk of
contiguous memory, usually 64 or 128 bytes. called "cache line". pulls that
whole line of data into cache at once.

if you read next byte of data after one you just requested, its already in
cache (on pallet in office) and its superfast.

actually multiple levels of cache. each bigger and slower and then next: l1,
l2, etc. different chips different.

when chip needs byte, if it's in cache, gets it fast. called
cache hit.

if not already in cache, have to go to ram (or next cache level). called
cache miss.

cache misses are the enemy.

imagine you're trying to optimize some code like this:

  for (int i = 0; i < NUM_ACTORS; i++)
  {
    doAbsolutelyNothingFor500Cycles();
    actors[i]->update();
  }

what's the first thing you're going to do? right. that fn call is a cache
miss.

speed difference when reading from cache compared to memory is huge. think
100x or more. has huge impact on app perf.

## memory = perf?

heard about importance of cache before writing this, but no first-hand
experience. (most my background higher level).

did some benchmarks.

surprisingly hard to write good test program to thrash cache and compare
worst case to best.

when i did, really surprised. got little program kind of like game loop.
(will use this for example later)

worse case was 50 *times* slower than best.

doing exact same computation, exact same data, exact same results

*only* difference was caching effects

[ymmv, heavily machine dependent]

tend to think of optimization on two axes: speed and memory usage

optimizations like memoization, caching, denormalization, etc. spend extra
memory to go faster

optimizations like compression trade in speed to save memory

sort of orthogonal, can spend one to get other

but tend to think of code being about speed and data being about memory usage

but surprising lesson for me here is data is about speed too!

book tries to be about concrete patterns. simple well-defined recipes.

optimization for cache usage is big topic. haven't even touched on instruction
cache - remember code is in ram too and has to be fetched onto cpu. caching
comes into play there too!

could write entire book

hard to come up with something simple for this chapter

if problem is lots of cache misses, solution is trying to keep the stuff you
need in cache lines you've already loaded. [on same pallet in analogy]

lot of techniques for doing that, mention a couple

but main one is pretty simple: lay out data contiguously in memory in the
order you process it

if code is crunhing on a then b then c, then make your memory look like

      +---+---+---+
      | a | b | c |
      +---+---+---+

note, not *pointers* to a, b, c. actual data, right there. if we just point
to it, have to follow that pointer, and now we're off in memory that's
unlikely to be in cache ("pointer chasing")

sounds easy, but we'll see some challenges

## The Pattern

modern cpus are much slower to access memory following memory that was previously accessed. improve performance by organizing data in memory in order you process it.

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
easier to change things without affecting other stuff. abstrction about
generalization, about being flexible by not making assumptions.

perf is often about the concrete. opt often starts with "assuming we only need
x" then takes advantage of that. thrives on specifics.

### Anti-OOP?

this pattern in particular fights against encapsulation. in c++ going through
interfaces implies pointers and references. but this pattern is about putting
object in memory *here*, not *pointer to it*.

[virtual methods are other half of encapsulation. those cause similar caching
problems with i cache.]

works best with objects of same type so they are same size. pattern chews on
big contiguous homogenous arrays. goes against subclassing where objects may
be different sizes.

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

## Sample Code

sample is two-fold demonstration. one goal is to show one way of applying
pattern by going from a before program that doesn't use it to after one that
does.

but program itself is also benchmark for pattern. can run and compare before
and after versions to see impact of caching. as you'll see, both programs
do exact same processing, only diff is locality.

[benchmarking is somewhere between black art and snake oil. just because this
one does or does not show certain numbers does not mean much for your prog.
have to decide is benchmark is representative of your prog. caching is
particularly hard because based on memory layout of entire program. exact same
benchmark code may have different result just by being put in your app. even
re-ordering functions in source file can change it!]

basic idea is we're going to sketch out a sort of minimal game-like program

it will simulate a fake game as fast as it can for a bunch of frames and we'll
see how long it takes

to try to make benchmark representative, we'll use skeletal versions of game
loop and component patterns.

### before

going bottom up, so start with component. in real game, would be different
kind of components ai, physics, rendering, etc.

in real game, component has relatively small amount
state (few dozen bytes or words). small amount of processing code that modifies
that state. has to be pretty fast because lots of actors in game each with
multiple components and need to process all of them each frame.

for benchmark, just have one type of component and behavior and state is
totally artificial. basically just monkey with handful of bytes.

^code component

important bit is even though just busywork for point of benchmark, have to
ensure state is actually exposed and used by prog. compilers very good at
dead code elim. if see code doing work and result never used, aggressively
remove code. easy to write benchmark that gets compiled to nothing!

now actor, insead of having behavior and state directly in class, just has a
few of these.

actor then has few of these. even though all same type in benchmark, pretend
each one represent something different

^code actor

game will have bunch of these. keep things simple, just have array of pointers
to them.

^code actor-list

this gets wrapped up in game loop. its job is to update every component for
every actor. important bit is that all components of one type are updated
for each actor. update all ai components, then all physics, etc.

instead of looping through actors then components within loop, do opposite

means we do multiple passes over actors

^code update-world

then wrap this in loop to keep game running. in real game loop would have
stuff here to control timing so game runs at predictable speed. here, want it
to go as fast as possible, so we just loop.

^code game-loop

then bail after fixed number of frames so we can get total time. again sum and
print is to ensure processing doesn't get compiled out.

[do bunch of frames because always noise in benchmarking: other processes
taking cpu time, imprecise timers, startup time, etc. longer benchmarks
average that out]

#### set up in mem

ok, that's basic structure. before we can get it going, need to populate
world.

in real game, actors getting created and destroyed all the time. constantly
interacting with memory manager (allocator).

assuming using general purpose allocator (malloc or new) end result is actors
and components scattered all over in memory.

for simple benchmark here, won't simulate creating and destroying actors over
time. [doing so on your own isn't bad idea, though!]

instead, will simulate "middle" of game: try to set up simulation as if lots
of creation and destruction has already happened.

in other words, going to shuffle actors around in memory explicitly

#### shuffling the deck

need bunch of actors and components. will allocate on heap, but who knows
what built-in new does? to control for that, will allocate big arrays for
each.

[yes, contradicts what just said. trust me.]

need bunch of actors and bunch of components, four for each actor

^code arrays

we have array here, but game loop has earlier array of points. like most real
games. (well, in real game probably actual stl list or vector.)

here's where we shuffle things in memory, pointers will not point to
corresponding actor index in array. instead, all shuffled.

do this by making array of numbers. each slot in array maps index in actor
pointer list to index of actor in main array that it points to. so if array
element 2 has value 5, that means the 2nd actor pointer in the game loop's
array will point to the 5th actor in the big array of actor objects.

first, helper fn:

^code rand-range

just picks random number in range (half-inclusive, so min and max - 1).

using this, we'll make array of ints and fill with consecutive nums. then
rearrange them.

^code shuffle

[using fisher-yates shuffle. aka knuth shuffle. bit about fisher-yates not being in cs.]

by shuffling array instead of just picking random indices, we ensure that
one pointer points to each actor, and no actor is pointed to more or less than
once. it's strict one to one mapping, just discombobulated.

with this, make array to map actor pointers to actors

then wire them all up

      fill actor pointer array

now we'll do same for components. make big list of ints that map
component pointers in actors to indices in giant component array. need index
for each component for each actor.

      array of actor * component indices

now wire up components in each actor

      walk actors and components and wire up to array

#### keep time

ok, that's a lot of set up, but now we're ready to go. well, almost. we need
a stopwatch. that bit is platform-specific and i try to avoid that here, so
just look up your os's fn to get current time. doesn't need to be that
precise (which is why we sim a lot of frames).

fns to start and stop timer

these tell us how much time elapsed between calls.

[wall time? process time? good question. this is rough benchmark just to give
you idea. if you want to get more precise, go for it.]

ok, now can put it all together. set everything up in memory. start clock.

sim game loop. stop clock and see how long it took. on machine, it ran in
XXX seconds.

### after

this where it gets interesting. now lets not shuffle. just assign actors
and components right in order. still use pointers, though.

      unshuffled set up

everything else is same. let 'er rip!

...

talk about *how* to keep things in order! object pools

...

## Design Decisions

`para`

### Do you own the game loop, or does the platform?

`para`

* **Do blah:**

  * *Pro.* ...

  * *Pro.* ...

## See Also

* http://research.scee.net/files/presentations/gcapaustralia09/Pitfalls_of_Object_Oriented_Programming_GCAP_09.pdf

* http://gamesfromwithin.com/data-oriented-design

* goes hand in hand with component

* "flat array of objects of same kind" -> "object pool"

## random notes

making your data structures smaller (bit-packing, ordering fields to reduce padding) will fit more into a cache line, that in turn directly makes them faster

  - smaller isn't just about saving memory
  - but careful about re-ordering fields since that can interfere with access
    patterns

avoid loading entire object into cache just for one bit, like:

  every actor:
    if (actor->isActive) actor->update();

reading memory is slower than processing
- this means optimization is as much about data as code

benchmarking notes

tried a bunch of different stuff. first benchmark looked super promising:
30% faster to update components when contiguous instead of actor

apparently completely artifact of how code was organized. commenting stuff out
or moving code could nullify or reverse change

(maybe it's an icache miss issue?)

tried other smaller benchmark and cache grind

update a bunch of actors with settable amount of padding between them

as padding increases, can see cache misses go up, which is good

tops off right at around 64 bytes total for an actor which is expected since
that's the cache line size

no noticeable performance difference though

maybe need to do more fake work with data so there's most instructions that
could be processed during read if didn't have cache stall?

"A key reason why data-oriented design is so powerful is because it works very well on large groups of objects. OOP, by definition, works on a single object."

"In the 80's we had the pleasure of access to main memory being in the order of a single cycle or so - obviously the focus on design in such a system is on the instructions. Do you know what was written in the 80's? C++ (well, started in '79 but first released in '85)."

separate out "hot" fields that are used often from "cold" fields that aren't.

references

* http://seven-degrees-of-freedom.blogspot.com/2009/10/latency-elephant.html
* http://msinilo.pl/blog/?p=614
* http://research.scee.net/files/presentations/gcapaustralia09/Pitfalls_of_Object_Oriented_Programming_GCAP_09.pdf
* http://www.microsoft.com/downloads/details.aspx?FamilyId=1643D55A-D252-4717-BC3E-237C2C5295F4&displaylang=en
* http://gamesfromwithin.com/data-oriented-design
* http://dice.se/wp-content/uploads/Introduction_to_Data-Oriented_Design.pdf
* https://docs.google.com/presentation/d/17Bzle0w6jz-1ndabrvC5MXUIQ5jme0M8xBF71oz-0Js/present#slide=id.i0
* http://igoro.com/archive/gallery-of-processor-cache-effects/
