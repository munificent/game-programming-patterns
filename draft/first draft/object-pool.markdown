^title Object Pool

## Intent
*Improve performance and memory usage by creating a fixed pool of
objects up front and reusing them instead of allocating and freeing
individual objects.*

## Motivation

Let's say you're working on the visual effects for your game. When
the hero casts a spell, you want a burst of sparkles to shimmer
across the screen. This calls for a particle system, a little engine
that can spawn a bunch of individual sparkly particles and animate
them until they wink out of existence.

Since a single wave of the wand could cause hundreds of particles to
be spawned, our particle system will need to be able to create
particles very quickly. Even more importantly, we need to ensure that
creating and destroying these particles doesn't cause memory
fragmentation.

### The Curse of Fragmentation

Programming a game for a dedicated game console like the XBox 360 in
many ways has more in common with embedded programming than it does
with convention PC programming. Like embedded programming, console
games must run continuously for a very long time without crashing or
leaking memory and efficient compacting memory managers are
rarely available. This means memory fragmentation is dreaded.

Fragmentation means the free space in your heap is broken into smaller
separate regions of memory instead of one large open space. The total
amount of memory available may be large, but the largest region of
*contiguous* memory can be painfully small. When you try to allocate
a chunk of memory for your object, even though there is that much
memory scattered across the heap, it can't find a single chunk big
enough and the allocation fails. Sad faces all around.

<pre class='diagram'>
initial heap is empty
                            free
                            ...................................

allocate an object 'foo'
                            foo      free
                            [|||||||]..........................
                            
allocate an object 'bar'    
                            foo      bar            free
                            [|||||||][|||||||||||||]...........

delete the first object     
                            free     bar            free
                            .........[|||||||||||||]...........
 
the free space is now fragmented into two regions
try to allocate another bar
                            free     bar            free
                            .........[|||||||||||||]...........
                            bar      XXXXXX     XXXX        bar
won't fit in either space   [|||||||||||||]     [|||||||||||||]
</pre>

> Most console makers require games to pass "soak tests" where they
> leave the game running in demo mode for several days. If the game
> crashes, they don't allow it to ship. While soak tests can sometimes
> crash because of a rare bug, it's usually creeping fragmentation
> or memory leakage that brings the game down.

Fragmentation can occur slowly but may still gradually reduce the
heap to an unusable foam of open holes and filled bits, ultimately
hosing the game completely.

Because of this, and because allocation can be slow, most games are
very careful about when and how they manage memory. For many cases,
the simplest solution is best: grab a big chunk of memory when the
game starts and don't mess with it too much while its running.

An object pool gives us the best of both worlds: to the memory
manager, we're just allocating one big hunk of memory up front and
not freeing it while the game is playing. To the users of the particle
system, we can freely allocate and deallocate particles to our heart's
content.

## The Pattern

Define a **pool** class that maintains a collection of **reusable
objects**. Each object, in addition to whatever state it needs for its
own use, also has an **"in use" flag** to mark whether or not it is
currently "live". When the pool is initialized, it creates the entire
collection of objects up front (usually in a single contiguous
allocation) and initializes them all to not in use.

When you want a new object, you ask the pool for one. It finds the
first available object, initializes it to "in use" and gives it back
to you. When the object is no longer needed, it is set back to the
"not in use" state. This way, objects can be freely created and
destroyed without needing to allocate memory or other resources.

## When to Use It

This pattern is used widely in games for obvious things like game
entities and visual effects, but also for less visible data structures
such as currently playing sounds. Use an object pool when you need to
frequently create and destroy objects of a certain type and:

*   allocating them on the heap is slow or could lead to memory
    fragmentation.

*   the object encapsulates a resource such as a database or network
    connection that is slow to acquire and could be reused.

## Parts

### ReusableObject

*   Tracks whether or not it is in use.
    
*   Can be reinitialized to become a "new" object.
    
### ObjectPool

*   Owns a collection of ReusableObjects
    
*   Creates new ReusableObjects by reinitializing ones from its
    collection.

## Keep in Mind

### Memory for the pool will be taken regardless of use.

The pool will be babysitting its pile of objects whether or not they
are actually used. When tuning pool sizes, we're usually carefuly to
make sure they're big enough, since making them too small will often
lead to a crash. Also important is making they aren't *too* big. A
smaller pool frees up memory that could be used in other ways to make
the game better.
    
### Only a fixed number of objects can be active at any one time.

In some ways this is a good thing. By partitioning memory into
separate pools for different types of objects, you ensure that, for
example, a huge sequence of explosions won't cause your particle
system to eat all of the available memory, preventing something more
critical from being allocated.
    
Nonetheless, this also means you will need to handle the case where
you try to create a new object and it fails because the pool is full.
There are a couple of common strategies for this.
    
1.  Just don't create the object. Sounds harsh, but this makes sense
    for some things like our particle system example. If all particles
    are in use, the screen is probably full of flashing graphics. The
    user won't notice if the next explosion isn't quite as impressive
    as the ones currently going off.
    
2.  Forcibly kill an existing object. Consider a pool for managing
    currently playing sounds. Let's say we want to start a new sound
    but the pool is full. We definitely *don't* want to simply ignore
    the new sound: the user will notice if their magical wand only
    swishes dramatically half the time. A better solution is to find
    the quietest sound already playing and replace that with our new
    sound. The new sound will mask the audible cutoff of the previous
    sound. In general, if the *absence* of a new object would be more
    noticeable than the *disappearance* of an existing one, this can
    be the right choice.

3.  Increase the size of the pool. If your game does let you be a bit
    more flexible with memory, you may be able to increase the size of
    the pool. If you choose this, make sure to consider whether or not
    the pool should contract to its previous size if the additional
    capacity is no longer needed.

    A single fixed pool size may not be the best fit for all game
    states. Some levels may be effect heavy while others are sound
    heavy. In that case, it may be useful to support different pool
    sizes for different scenarios.

### Unused objects will remain in memory.

This doesn't generally matter, but if you are also using a garbage
collector, this can prevent it from freeing some memory. If your
pooled object has a reference to some other object, the garbage
collector has no way of knowing that that reference is "dead" when the
pooled object is not in use.
    
Many garbage collectors perform very well and can even compact memory
and reduce fragmentation. If you are using one, consider carefully
whether or not you even need to use an object pool for memory or
performance reasons.

### Memory size for each object is fixed.

Most object pool implementations store the objects in an array of
in-place objects. If all of your objects are of the same type, this is
fine. However, if you want to store derived objects in the pool, or
objects of different types, you need to ensure that each slot in the
pool has enough memory for the largest possible object.

> This is a common pattern for implementing speed-efficient memory
> managers. The manager has a number of pools of different block
> sizes. When you ask it to allocate a block, it finds in an open slot
> in the pool of the appropriate size and allocates from that pool.

This leads to another consideration. If your objects do vary in size,
you could end up wasting memory. Each slot will need to be big enough
to accomodate the largest type. If that type is rarely used and is
much bigger than the more common types, you're throwing away memory.
When this happens, it may be useful to split the pool into separate
pools for different sizes of object. 

### Reused objects aren't automatically cleared.

> It might be worth adding a debug feature to your object pool class
> that will clear the memory for an object to some known value when
> the object is no longer in use. That way, when that slot is used
> again later, it's easy to see which variables in the object haven't
> been initialized.
 
Most memory managers will zero out memory before use, or
initialize to some obviously wrong value, at least in debug builds.
This can help you find painful bugs caused by uninitialized variables.
Since the object pool isn't going through the memory manager every 
time it reuses an object, we won't get that safety net anymore. Worse,
the memory it's using for the "new" object previously held an object
of the exact same time, so it can be nigh impossible to tell from
looking at it if you forgot to initialize something when you created
the new object.

Because of this, pay special care that the code that initializes new
objects in the pool fully initializes the object.

## Design Decisions

At its simplest, an object pool is an almost trivial pattern: you
have an array of objects and you reinitialize them periodically. But
there are a bunch of ways you can build on top of that to make the
pool more generic, safer to use, or easier to maintain. As you
implement them in your games, you'll need to answer these questions:

### Are objects coupled to the pool?

The first question you'll run into when writing an object pool is
deciding if the objects directly defined to be used in a pool, or
whether pooling is a separate system that happens to pool those
objects. Most of the time, the answer will be that the objects are
defined to be pooled, but if you are trying to write a generic pool
class that can pool arbitrary objects, that can be an undesireable
limitation.

* **If objects are explicitly bound to the pool:**

    *   *The implementation is simpler.* You can simply put the "in
        use" flag or function in your pooled object and be done with
        it.
    
    *   *You can programmatically ensure that the objects can *only*
        by created by the pool.* A simple way to do this is to make
        the pool class a friend of the object class, and then make the
        object's constructor private.
    
        ^code 10

        This helps make sure your users don't create objects that
        aren't correctly tracked by the pool and points them towards
        the right way to use the class.

    *   *You may be able to avoid storing an explicit "in use" flag.*
        Many objects already retain some state that could be used to
        tell if it's live or not. For example, a particle may be
        available for reuse if it's current position is offscreen. If
        the object class knows it may be used in a pool, it can
        provide an `InUse()` method to query that state. This saves
        the pool from having to burn some extra memory storing a bunch
        of "in use" flags that aren't needed.

* **If objects are not coupled to the pool:**

    *   *Objects of any type can be pooled.* This is the big
        advantage. By decoupling objects from the pool, you may be
        able to implement a generic reusable pool class.
        
    *   *The "in use" state must be tracked elsewhere.* The simplest
        way to do this is by having a separate bit field.
        
        ^code 11
        
### What is responsible for initializing the reused objects?

In order to reuse an existing object, it must be reinitialized with
new state. A key question here is does this happen inside the pool
class, or outside?

*   **If the pool handles it internally:**

    *   *The pool can completely encapsulate its objects*. Depending
        on the other capabilities your objects need, you may be able
        to keep them completely internal to the pool. This can make
        sure that other code doesn't maintain references to objects
        that could be unexpectedly reused.
        
    *   *The pool is tied to how objects are initialized*. A pooled
        object may have multiple different ways it can be initialized.
        If the pool manages this, its interface needs to support all
        of those different mechanisms and forward them to the object.
    
        ^code 12

*   **If outside code initializes it:**

    *   *The pool relies on outside code to correctly initialize
        objects.* This is the most obvious implication, but an
        important one to consider. The pool won't be able to ensure
        that objects are placed in a correct state when they are
        reused.
        
    *   *The pool's interface can be simpler.* Instead of multiple
        functions for each way an object can be initialized, the
        pool can simply return a reference to the new object.
        
        ^code 13
        
        The caller can then initialize it directly using whathever is
        appropriate.
        
        ^code 14
        
    *   *Outside code may need to handle failure to create a new
        object.* The previous example code assumes that `Create()`
        will always successfully return a new object. If the pool is
        full, though, that may fail to happen. You'll need to make
        sure your calling code handles that case too, if it can occur.
        
## Sample Code

### First Pass

Real-world particle systems will often apply gravity, wind,
friction, and other physical effects. Our much simpler sample will
just move particles in a straight line for a certain number of frames
and then kill the particle. Not exactly film calibur, but it should
get the job done. First up is the little particle class.

^code 1

It provides a single default constructor that initializes it to not
in use. It relies on a later call to its `Init()` function to 
initialize it to a live state.

To animate the particles over time, an `Animate()` function is
provided. The game is expected to call this once per frame for each
live particle.

The pool will need to know which particles are available for reuse,
and this class provides that with the `InUse()` function. That
function takes advantage of the fact that particles have a limited
lifetime, and uses that to know which particles are in use, without
having to store a separate flag.

The pool class is also simple.

^code 2

It provides a `Create()` function for external code to create new
particles. `Animate()` should be called once per frame by the game,
and it simply calls the same method on each particle in the pool.

The particles themselves are simply stored in a fixed-size array in the
class. In this sample implementation, the pool size is hardcoded in
the class, but this could be tunable externally by using a dynamic
array of a given size, or using a value template parameter.

Creating a new particle is pretty straightforward.

^code 3

We simply iterate through the pool looking for the first available
particle. When we find it, we initialize it and we're done. Note that
in this implementation, if there aren't any available particles, we
simply don't create a new one.

That's pretty much all there is to a simple implementation. We can now
create a pool, and create some particles using it. The particles will
automatically free themselves up when their lifetime has expired.

> O(n) complexity, for those of us who remember our algorithms class.

For many use cases, this is good enough, but keen eyes may have
noticed that creating a new particle requires iterating through
(potentially) the entire collection until we find an open slot. If
the pool is very large and mostly full, that can get slow. Let's see
if we can improve that without sacrificing any memory.

### Speeding Up Creation

Ideally, we'd store a separate collection of pointers to all of the
free particles in the pool. Then when we need to create a new one, we
can just pull the first pointer off of our free list and insert the
new particle there.

The problem is that that would require us to
maintain an entire other array with as many pointers as there are
objects in the pool. It would be nice to not have to give up any
memory to get this speed improvement. Fortunately, there *is* some
memory laying around we can use: *the slots for the unused objects
themselves.*

When a particle isn't in use, most of it's state is
irrelevant. It's position and velocity aren't being used anyway. For
a dead particle, the only state it needs is the minimim required to
tell if it's dead. For our example, that's the `_framesLeft` member.
Everything else we can reuse. Here's a revised particle:

^code 4

We've gotten all of the member variables except for `_framesLeft` and
moved them into a struct inside union. This `live` struct will hold
the particle's state when it's in use. When it's available, the other
case of the union, the `next` member will be used. It holds a
pointer to the *next* available particle.

The idea is that we'll use the memory from all of the *dead* particles
to create a linked list that threads its way through them. Now we have
our collection of pointers to available particles, we're just
interleaving it into the particles themselves.

For this to work, we need to make sure this list is set up correctly,
and is is correctly maintained when particles are created and
destroyed. And, of course, we'll need to keep track of the list's
head:

^code 5

When a pool is first created, all of the particles are available, so
our list should thread through the entire pool. We'll make the pool
constructor set that up:

^code 6

> O(1), baby! Now we're cooking with fire!

Now, to create a new particle, we can jump directly to the first
available one:

^code 7

And finally, when a particle gives up the ghost, we'll thread it back
onto the list:

^code 8
 
## See Also

### Flyweight (GoF)

On the surface, this looks a lot like the Flyweight (GoF) pattern.
Both maintain a collection of small objects so that they can be
reused. The difference here is what is meant by "reuse". Flyweight
objects are intended to be reused by sharing the same instance between
multiple owners simultaneously. Flyweight is about avoiding duplicate
memory usage.

Pooled objects are not intended to be used that way. The objects in a
pool get reused, but only over time. "Reuse" in the context of an
object pool means reclaiming the *memory* for an object *after the
original owner is done with it*. With an object pool, there isn't
any expectation that an object will be shared within its lifetime.

<p class='note'>
circular buffer implementation?

* all objects should have same lifetime
* handles the pool overflowing by killing and replacing the oldest
  object with the new one
* useful for things like particle systems where the total lifetime
  isn't too critical, but staying within an easily adjustable memory
  footprint is
</p>
