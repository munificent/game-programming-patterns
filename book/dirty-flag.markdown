^title Dirty Flag
^section Optimizing Patterns

## Intent

*Avoid unnecessary work by deferring it until the result is needed.*

## Motivation

"Flag" and "bit" are synonymous in programming: they both mean a single micron of data that can be in one of two states. With bits we call them "true" and "false". For flags, you sometimes hear "set" and "cleared". I'll use all of these interchangeably. "Dirty bit" is an equally <span name="specific">common</span> name for this pattern, but I figured I'd stick with the name that didn't seem as prurient.

<aside name="specific">

Wikipedia's editors don't have my level of self-control and went with [dirty bit](http://en.wikipedia.org/wiki/Dirty_bit).

</aside>

### Locating a ship at sea

Most games have something called a *scene graph*. This is a big data structure that contains all of the objects in the world. The rendering engine uses this to determine where on screen to draw stuff.

At its simplest, a scene graph is just a flat list of objects. Each object has a model or some other graphic primitive, and a <span name="transform">*transform*</span>. The transform describes the object's position, rotation, and scale in the world. To move or turn an object, we just change its transform.

<aside name="transform">

The mechanics of *how* this transform is stored and manipulated is unfortunately out of scope here. The comically abbreviated summary is that it's a 4x4 matrix. You can make a single transform that combines two transforms -- for example translating and then rotating an object -- by multiplying the two matrices.

How and why that works is left as an exercise for the reader.

</aside>

When the renderer draws an object, it takes the object's model, applies the transform to it, and then renders it there in the world. If we just had a scene *bag* and not a scene *graph* that would be it and life would be simple.

However, most scene graphs are <span name="hierarchical">*hierarchical*</span>. An object in the graph may have a parent object that it is anchored to. In that case, its transform is relative to the *parent's* position, and isn't its absolute position in the world.

For example, imagine our game world has a pirate ship at sea. Atop the ship's mast is a crow's nest. Hunched in that crow's nest is a pirate. Clutching the pirate's shoulder is a parrot. The ship's local transform will position it in the sea. The crow's nest's transform positions it on the ship, and so on.

**TODO illustrate**

This way, when a parent object moves, its children move with it automatically. If we change the local transform of the ship, the crow's nest, pirate, and parrot go along for the ride. It would be a total <span name="slide">headache</span> if, when the ship moved, we had to manually adjust the transforms of everything on it to keep them from sliding off.

<aside name="slide">

To be honest, when you are at sea you *do* have to keep manually adjusting your position to keep from sliding off. Maybe I should have chosen a drier example.

</aside>

But to actually draw the parrot on screen, we need to know its absolute position in the world. I'll call the parent-relative transform the object's *local transform*. To render an object, we need to know its *world transform*.

### Local and world transforms

Calculating an object's world transform is pretty straightforward: you just walk its parent chain starting at the root all the way down to the object, combining transforms as you go. In other worlds, the parrot's world transform is:

<span name="degenerate"></span>

    ship local transform * nest local transform * pirate local transform * parrot local transform

<aside name="degenerate">

In the degenerate case where the object has no parent, its local and world transforms are equivalent.

</aside>

We need the world transform for every object in the world every frame, so even though it's just a handful of matrix multiplications per model, it's on the hot code path where performance is critical. Keeping them up-to-date is tricky because when a parent object moves, that affects the world transform of itself and all of its children, recursively.

The simplest approach is to just calculate transforms on the fly while rendering. Each frame, we recursively traverse the scene graph starting at the top of the hierarchy. For each object, we calculate its world transform right then and draw it.

But this is terribly wasteful of our precious CPU juice! Many objects in the world are *not* moving every frame. Think of all of the static geometry that makes up the level. Recalculating their world transforms each frame even though they haven't changed is a waste.

### Cached world transforms

The obvious answer is to *cache* it. In each object, we store its local transform and its derived world transform. When we render, we just use the precalculated world transform. If the object never moves, that's always up to date and everything's happy.

When an object *does* move, the simple approach is to refresh its world transform right then. But don't forget the hierarchy! When a parent moves, we have to recalculate its world transform *and all of its children's, recursively*.

Imagine some busy gameplay. In a single frame, the ship gets tossed on the ocean, the crow's nest rocks in the wind, the pirate leans to the edge, and the parrot hops onto his head. We changed four local transforms. If we recalculate world transforms eagerly whenever a local transform changes, what ends up happening?

    1. update ship local transform
    2. calculate ship world transform
    3. calculate nest world transform
    4. calculate pirate world transform
    5. calculate parrot world transform
    6. update nest local transform
    7. calculate nest world transform
    8. calculate pirate world transform
    9. calculate parrot world transform
    10. update pirate local transform
    11. calculate pirate world transform
    12. calculate parrot world transform
    13. update parrot local transform
    14. calculate parrot world transform

We only moved four objects, but we did *ten* world transform calculations. That's six pointless calculations that get thrown out before they are ever used by the renderer. We calculated the parrot's world transform *four* times, but it only got rendered once.

The problem is that a world transform may depend on several local transforms. Since we recalculate immediately each time *one* of those changes, we end up recalculating the same transform multiple times when more than one of the local transforms it depends on changes in the same frame.

### Deferred recalculation

We'll solve this by <span name="decoupling">decoupling</span> changing local transforms from updating the world transforms. This lets us change a bunch of local transforms in a single batch and *then* recalculate the affected world transform once after all of those modifications are done, right before we need it to render.

<aside name="decoupling">

It's interesting how much of software architecture is just intentionally engineering a little slippage.

</aside>

To do this, we add a flag to each object in the graph. When the local transform changes, we set it. When we need the object's world transform, we check the flag. If it's set, we calculate the world transform then and clear the flag. The flag represents, "Is the world transform out of date?" For reasons that aren't entirely clear, the traditional name for this "out-of-date-ness" is "dirty". Hence: *a dirty flag*.

If we apply this pattern and then move all of the objects in our previous example, the game ends up doing:

    1. update ship local transform
    2. update nest local transform
    3. update pirate local transform
    4. update parrot local transform
    5. calculate ship world transform
    6. calculate nest world transform
    7. calculate pirate world transform
    8. calculate parrot world transform

That's the best you could hope to do: the world transform for each affected object is calculated exactly once. With just a single bit of data, this pattern does a few things for us:

1. It collapses modifications to multiple local transforms along an object's parent chain into a single recalculation on the object.
2. It avoids recalculation on objects that didn't move.
3. And a minor bonus: if an object gets removed before it's rendered, it doesn't calculate its world transform at all.

## The Pattern

A set of **primary data** changes over time. A set of **derived data** is determined from this using some **expensive process**. A **"dirty" flag** tracks when the derived data is out of sync with the primary data. It is **set when the primary data changes**. When the derived data is requested, if the flag is set **the processing is done then and the flag is cleared.** Otherwise, the previous **cached derived data** is used.

## When to Use It

Compared to some other patterns in this book, this one solves a pretty specific problem. Also, like most optimizations, you should only reach for it when you have a performance problem big enough to justify the added code complexity.

Dirty flags are typically used to solve two problems: *calculation* and *synchronization*. In both cases, the process of going from the primary data to the derived data is time-consuming or otherwise costly.

In our scene graph example, the process is slow because of the amount of math to perform. When using this pattern for synchronization on the other hand, it's more often that the derived data is *somewhere else* -- either on disk or over the network on another machine -- and simply getting it from point A to point B is what's expensive.

There are a couple of other requirements too:

 *  **The primary data has to change more often than the derived data is used.**
    This pattern gets its benefit from collapsing multiple changes to the
    primary data into a single refresh of the derived data. That's possible
    because those changes happen in a batch before the derived data is used.
    If you find yourself always needing that derived data after every single
    modification to the primary data, this pattern can't help.

 *  **It should be hard to incrementally update the derived data.** Let's say the
    pirate ship in our game can only carry so much booty. We need to
    know the total weight of everything in the hold. We
    *could* use this pattern and have a dirty flag for the total weight. Every
    time we add or remove some loot, we set the flag. When we need the
    total, we add up all of the booty and clear the flag.

    But a simpler solution is to just *keep a running total*. When we add or
    remove an item, just add or remove its weight from the current total. If
    we can "pay as we go" like this and keep the derived data updated, then
    that's often a better choice than using this pattern.

All of this makes it sound like dirty flags are never actually useful, but
you'll eventually find a place here or there where they help.
<span name="hacks">Grepping</span> your
average game codebase for the word "dirty" will often
turn up uses of this pattern.

<aside name="hacks">

From my research, it also turns up a lot of comments apologizing for "dirty" hacks.

</aside>

## Keep in Mind

Even after you've convinced yourself this pattern is a good fit, there are a few wrinkles that can cause you some discomfort.

### There is a cost to deferring too long

This pattern defers some slow work until the result is actually needed, but when is, it's often needed *right now*. But the reason we're using this pattern to begin with is because calculating that result is slow!

This isn't a problem in our example because we can still calculate world coordinates fast enough to fit within a frame, but you can imagine other cases where the work you're doing is a big chunk that takes noticeable time to chew through. If the game doesn't *start* chewing until right when the player expects to see the result, that can cause an unpleasant visible <span name="gc">pause</span>.

Another problem with deferring is that if something goes wrong, you may never do the work at all. This can be particularly problematic when you're using this pattern to save some state to a more persistent form.

**TODO: Illustration of title bar**

For example, text editors track if your document has "unsaved changes", which is a picture-perfect example of this pattern. That little bullet or star in your file's title bar is literally the dirty flag visualized. The primary data is the open document in memory, and the derived data is the file on disk.

Many programs don't save to disk until either the document is closed or the application is exited. That's fine most of the time, but if you accidentally kick the power cable out, there goes your masterpiece.

We've talked about two times you can calculate or synchronize the derived data -- the second the primary data changes or right before the derived data is needed -- but there's actually a continuum between those points. There are a range of options where you defer the work *somewhat* but still kick it off before the result is needed.

<aside name="gc">

This is mirrors the different garbage collection strategies in systems that automatically manage memory. Reference counting systems free memory the second its no longer needed, but burn CPU time updating ref counts eagerly every time references are changed.

Simple garbage collectors defer reclaiming memory until it's really needed, but the cost is the dreaded "GC pause" that can freeze your entire app until the collector is done scouring the heap.

In between the two are more complex systems like deferred ref-counting and incremental GC that reclaim memory less eagerly than pure ref-counting but more eagerly than stop-the-world collectors.

</aside>

When your fancy text editor auto-saves a backup in the background every few minutes, that's basically the trade-off it's making. The frequency that it auto-saves is the point it chose on the continuum between not losing too much work when a crash occurs and not thrashing the file system too much by saving all the time.

### You have to make sure to set the flag *every* time the state changes

Since the derived data is calculated from the primary data, it's essentially a cache. Whenever you have cached data in memory, the trickiest aspect of it is <span name="cache">*cache invalidation*</span> -- correctly noting when the cache is out of sync with its source data. In this pattern, that means correctly setting the dirty flag when *any* primary data changes.

<aside name="cache">

Phil Karlton famously said, "There are only two hard things in Computer Science: cache invalidation and naming things."

</aside>

Miss it in one place, and your program will incorrectly use stale derived data. This leads to confused players and very hard to track down bugs. When you use this pattern, you'll have to be very careful that any code that modifies the primary state also sets the dirty flag. In our scene graph, for example, we have to remember to invalidate not just the object's world transform when its local transform changes, but all of its children too since they depend on it.

One way to mitigate this is by encapsulating modifications to the primary data behind some interface. If anything that can change the state goes a single narrow API, you can set the dirty bit there and rest assurred that it won't go out of sync.

### You have to keep the previous derived data in memory

<span name="sync"></span>

When the derived data is needed and the dirty flag *isn't* set, it uses the previously calculated data. This is obvious, but that does imply that you have to keep that derived data around in memory in case you end up needing it later.

<aside name="sync">

This isn't much of an issue when you're using this pattern for synchronization. There, the derived data isn't usually in memory at all.

</aside>

If you weren't using this pattern, you could calculate the derived data on the fly whenever you needed it, then discard it when you were done. That avoids the expense of keeping it cached in memory, at the cost of having to do that calculation every time you need the result.

Like many optimizations, then, this pattern <span name="trade">trades</span> memory for space. In return for keeping the previously calculated data in memory, you avoid having to recalculate it when it hasn't changed. This trade-off makes sense when the calculation is slow and memory is cheap. When you've got more time than memory on your hands, it's better to just calculate it as needed.

<aside name="trade">

Conversely, compression algorithms make the opposite trade-off: they optimize *space* at the expense of the processing time needed to decompress.

</aside>

## Sample Code

Let's assume we've met the surprisingly long list of requirements and see how the pattern looks in code. As I mentioned before, the actual math behind transform matrices is beyond the humble aims of this book, so I'll just encapsulate that in a class whose implementation you can presume exists somewhere out in the ether:

^code transform

The only operation we need here is `combine()` so that we can get an object's world transform by combining all of the local transforms along its parent chain. It also has a method to get an "origin" transform: basically an identity matrix that means no translation, rotation, or scaling at all.

Next, we'll sketch out the class for an object in the scene graph. This is the bare minimum its needs *before* applying this pattern:

^code graph-node

Each node has a local transform which describes where it is relative to its parent. It has a mesh which is the actual graphic object that's used to render it. (We'll allow `_mesh` to be `NULL` too to handle non-visual nodes that are used just to group their children.) Finally, each node has a possibly empty collection of child nodes.

With this, a "scene graph" object is really just a single root `GraphNode` object whose children (and grandchildren, etc.) are all of the objects in the world:

^code scene-graph

In order to render a scene graph, all we need to do is traverse that tree of nodes starting at the root and call this function for each node's mesh with its correct world transform:

^code render

We won't implement this here, but if we did, it would do whatever magic the renderer needs to draw that mesh at the given location in the world. If we can call that correctly and efficiently on every node in the scene graph, we're happy.

### An unoptimized traversal

To get our hands dirty, let's throw together a basic traversal for rendering the scene graph by calculating the world positions on the fly. It won't be optimal, but it will be simple. We'll add a new method to `GraphNode`:

^code render-on-fly

We pass the world transform of the node's parent into this in `parentWorld`. With that, all that's left to get the correct world transform of *this* node is to combine it with its own local transform. We don't have to walk *up* the parent chain to calculate this because we calculate as we go while walking *down* the chain.

We calculate the node's world transform and store it in `world`, then we render the mesh if we have one. Finally, we recurse into the child nodes, passing in *this* node's world transform. All in all, it's nice tight, simple recursive method.

We kick off the process by rendering the root node:

^code render-root

### Let's get dirty

So this code does the right thing -- renders all the meshes in the right place -- but it doesn't do it efficiently. It's calling `_local.combine(parentWorld)` on every node in the graph, every frame. Let's see how this pattern fixes that. First, we need to add two fields to `GraphNode`:

^code dirty-graph-node

The `_world` field caches the previously-calculated world transform, and `_dirty`, of course, is the dirty flag. Note that the flag starts out `true`. When we create a new node, we haven't calculated it's world transform yet, so at birth it's already out of sync with the local transform.

The only reason we need this pattern is because objects can *move*, so let's add support for that:

^code set-transform

The important part here is that it sets the dirty flag too. Are we forgetting anything? Right: the child nodes!

When a parent node moves, all of its children's world coordinates are invalidated too. But here we aren't setting their dirty flags. We *could* do that, but that's recursive and slow. Instead we'll do something clever when we go to render. Let's see:

^code dirty-render

This is similar to the original naïve implementation. The key changes are that we check to see if the node is dirty before calculating the world transform, and we store it in a field instead of a local variable. When the node is clean, we skip the `combine()` completely and use the old but still correct `_world` value.

The clever bit is that `dirty` parameter. That will be `true` if any node above this node in the parent chain was dirty. In much the same way that `parentWorld` updates the world transform incrementally as we traverse down the hierarchy, `dirty` tracks the dirtiness of the parent chain.

This lets us avoid having to actually recursively set each child's `_dirty` flag in `setTransform()`. Instead, we'll just pass the parent's dirty flag down to its children when we render and look at that too to see if we need to recalculate the world transform.

The end result here is exactly what we want: changing a node's local transform is just a couple of assignments, and rendering the world calculates the exact minimum number of world transforms that have changed since the last frame.

## Design Decisions

This pattern is pretty specific, so there are only a couple of knobs to twiddle on it:

### When is the dirty flag cleaned?

* **If you defer it until the result is needed:**

    * *It avoids doing calculation entirely if the result is never used.* For
        primary data that changes frequently and where the derived data is
        rarely accessed, this can be a big win.

    * *If the calculation is time-consuming, it can cause a noticeable pause.*
        Postponing the work until the end-user is waiting to see the result can
        affect their gameplay experience. Often, it's fast enough that this
        isn't a problem, but if it is, you'll have to do the work earlier.

* **At well-defined checkpoints:**

    Sometimes there is a point in time or the progression of the game where it's
    natural to do the deferred synchronization or calculation. For example,
    we may want to save the game only when the pirate sails into port. Or the
    sync point may not part of the game mechanics. We may just want to hide the
    work behind a loading screen or a cut scene.

    * *You can ensure the time spent doing the work doesn't impact the user
        experience.* Unlike the above option, you can often give something to
        distract the player while the game is busy on other things.

    * *You lose control of when the work actually happens.* This is sort of the
        opposite of the above point. You have micro-scale control over when the
        work happens, and can make sure the game handles it gracefully.

        What you *can't* do is ensure the player actually makes it to the
        checkpoint or meets whatever criteria you've defined. If they get lost
        or the game gets in a weird state, you can end up deferring the work
        longer than you expected.

* **In the background:**

    Like your text editor that auto-saves a backup every few minutes, you can
    do the work after some time interval. Usually, you'll start the <span name="hysteresis">timer</span>
    on the first modification and then process all of the changes that happened
    between then and when the timer fires.

    <aside name="hysteresis">

    The term in human-computer interaction for in intentional delay between
    when a program receives user input and when it responds is [*hysteresis*](http://en.wikipedia.org/wiki/Hysteresis).

    </aside>

    * *You can tune how often the work is performed.* Since the time
        when we clean the dirty state and do the work isn't dependent on
        the player requesting some data or reaching some checkpoint, we can
        ensure it happens as frequently (or infrequently) as we want.

    * *You can do more redundant work.* If the primary state only changes a
        tiny amount during the timer's run and our dirty flags are
        coarse-grained, we'll process a bunch of data that hasn't
        changed. When changes trickle in, that timer will constantly be
        triggering work over data that hasn't changed much again and again.

    * *You'll need some support for doing work asynchronously.*
        Processing the data "in the background" implies that the player can
        keep doing whatever it is that they're doing at the same time. That
        means we'll likely need threading or some other kind of concurrency
        support so that the work we're doing can happen while the game is still
        responsive and being played.

        Since the player is likely interacting with
        the same primary state that you're processing, you'll need to think
        about making that safe for concurrent modification too.

### How fine-grained is your dirty tracking?

Imagine our pirate game lets players build and customize their pirate ship. Ships are automatically saved online so the player can resume where they left off. We're using dirty flags to determine which decks of the ship have been fitted and need to be sent to the server. Each chunk of data we send to the server contains some modified ship data and a bit of metadata describing where on the ship this modification occurred.

* **If it's more fine-grained:**

    Say we slap a dirty flag on each tiny plank of each deck.

    * *You only process data that actually changed.* We'll send exactly the
        facets of the ship that were modified to the server.

    * *When a lot of data changes, a larger number of dirty flags also need to
        be set.*

* **If it's more coarse-grained:**

    Alternatively, we could associate a dirty bit with each deck.
    Changing anything on that deck marks the entire deck <span name="swab">dirty</span>.

    <aside name="swab">

    I could make some terrible joke about it needing to be swabbed here, but
    I'll refrain.

    </aside>

    * *You end up processing unchanged data.* Add a single barrel to a deck
        and we'll have to send the whole thing to the server.

    * *Less memory is used for storing dirty flags.*

    * *Less time is spent on fixed overhead.* When processing some changed data,
       there's often a bit of fixed work you have to do on top of handling the
       data itself. In the example here, that's the metadata required to
       identify where on the ship the changed data is. The bigger your
       processing chunks, the fewer of them there are, which means the less
       fixed overhead you have.

## See Also

*   This pattern is increasingly common outside of games in client-side MVC
    web frameworks like [Angular](http://angularjs.org/) which use dirty
    flags to track which data has been changed in the browser and needs to
    be pushed up to the server.

* Physics engines keep track of which objects are resting and which are in motion. Since a resting body won't move until an impulse is applied to it, they don't need any processing until they get touched. This "is resting" bit is essentially a dirty flag to track which objects have had forces applied and need to have their physics resolved.
