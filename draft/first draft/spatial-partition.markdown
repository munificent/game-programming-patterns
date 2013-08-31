^title Spatial Partition
^section Optimizing Patterns

## Intent

*Quickly find objects near a certain location by storing them in a data
structure organized by position.*

## Motivation

We play games in part because they let us visit other worlds different from our
own. But those worlds typically aren't *so* different. They often share the same
basic physics and tangibility of our world. This is why they can feel so real.

One simple facet of that that we'll be focusing on here is *location*. Game
worlds usually have a sense of *space* and objects are somewhere in that space.
This manifests itself in a bunch of ways. The obvious one is physics: objects
move, collide, and interact. But the audio engine may take into account where
sound sources are relative to the player. Online chat may be restricted to other
players close to yourself.

Space, and *spatialness* permeates your game engine. Often, it boils down to
needing answer to the question, "what objects are near this location". If you
find yourself asking this frequently enough each frame, and it can start to be
a performance bottleneck for your game.

### Units on the field of battle

Say we're making a real-time strategy game. Opposing armies with hundreds of units will clash together on the field of battle. Warriors need to know who is nearby to know who to avoid and who to swing their blade at.

The naïve way to determine this is to look at every pair of units and see how close they are to each other:

^code pairwise

Here we have a doubly-nested loop where each loop is walking <span name="all">all</span> of the units on the battlefield. That means the number of pairwise tests we have to perform each frame increases with the *square* of the number of units. Each additional unit you add has to be compared to *all* of the previous ones. With a large number of units, that can spiral out of control.

<aside name="all">

The inner loop doesn't actually walk all of the units. It walks all of the ones the outer loop hasn't already visited. This is to avoid comparing each pair of units *twice*, once in each order. If you've already handled a collision between A and B, you don't need to check it again for B and A.

In Big-O terms, though, this is still *O(n&sup2;)*.

</aside>

### Drawing battlelines

Before we see how this pattern tackles our big optimization problem, let's see how we'd solve a smaller one first. Imagine if our battlefield was simplified such that any unit could only be in one of a few different locations. Instead of a 2D battle*field*, imagine it's a 1D battle*line*. Not only that, but only *integer* positions are allowed. A unit's position is just an int within some known small range.

Again, we're trying to find which units are battling, which means which units have the same position. But now that there are only a few positions in question, we can adjust our data structure to make that explicit. Instead of a single array of units, we'll have an <span name="pigeonhole">array</span> of units *at each position*:

^code pigeonhole-array

<aside name="pigeonhole">

This is essentially the same as a sorting algorithm called a *pigeonhole sort*. To sort a bunch of numbers within some range, you create second array of "pigeonholes", one for each value in the range. Then you walk the original array and put each number in its hole. Then you walk through the holes in order and copy the contents of the non-empty ones back to the array.

Because it uses memory based on the *range* of values, it isn't a good fit for many problems, but for the ones where it does work, it's hard to beat -- it has *O(n)* worst-case performance. Snazzy!

</aside>

Since we don't have a single array of units, we'll have to look at each position to see who is fighting:

^code pigeonhole-outer

Then within that position, we look at each pair of units:

^code pigeonhole

This effectively means that instead of a *doubly*-nested loop, now it's *triply*-nested. It seems we've made things worse!

There's a big difference, though: each of those inner arrays will be *much* shorter. In fact, with this artificially simple problem we've set up for ourselves, each array will *only* contain units that actually are in combat since they will have the exact same position.

For example, consider a battleline with 100 distinct positions and 200 units randomly distributed on them. Our first brute-force solution will have to compare 199,000 pairs of units to find around 200 actual hits.

With a separate array for each position, we don't have to look at *any* pairs of units that aren't actually hitting each other. Instead of 199,000 "do you have the same position?" checks, we go straight to the 200 known hits, a factor of 100 improvement.

The lesson is pretty obvious: if you store your objects in a data structure organized by their location, <span name="obvious">you can find them much more quickly</span>. Our artificially sample program with just a couple of fixed positions makes that trivial to do. This pattern is about applying the same technique to harder problems: ones where objects are in two or three dimensions, where their position can vary continuously, or where you might not even know the bounds they have to fit within.

<aside name="obvious">

This sounds stupidly obvious when I put it this way. But obvious *after* the fact doesn't always mean obvious *before* you learn it. It took eighty years after the invention of canning before someone came up with the can opener.

</aside>

## The Pattern

A set of **objects** each have a **position in space**. Store them in a **spatial data structure** that organizes them by that position. This data structure lets you **efficiently query for objects at or near a location**. When an object's position changes, **update the spatial data structure** so that it can continue to find it.

## When to Use It

This is a very common pattern for storing both live, moving game objects and also often the more static art and geometry of the level. Sophisticated games will often have multiple different spatial partitions for different kinds of content.

However, like many patterns, it does have some complexity overheard. Advanced spatial partitioning techniques require real engineering work to get right. Don't pull this out of the toolbox unless you know its worth the effort.

The basic requirements for this pattern are that you have a set of objects that each have some kind of position and that you frequently need to query that set to find objects near each other or near some location.

## Keep in Mind

Spatial partitions exists mainly to knock an *O(n&sup2;)* query down to something more manageable. The *more* objects you have, the more valuable that becomes. Conversely, if your *n* is small enough, it may not be worth the bother.

Since this pattern involves organizing objects by their position, objects that *change* their position are harder to deal with. You'll have to <span name="hash-change">reorganize</span> the data structure to keep track of the object at its new location, and that adds code complexity *and* burns CPU cycles. Make sure the trade-off is worth it.

<aside name="hash-change">

Imagine a hash table where the keys of the hashed objects can change spontaneously, and you'll have a good feel for why it's tricky.

</aside>

Spatial partitions also take up a chunk of additional memory for the bookkeeping data structures. Like many optimizations, they trade memory for speed. If memory is tight, that may be a losing proposition.

## Sample Code

Like most patterns, spatial partitions come in a lot of different flavors. Unlike some other patterns, though, these specific flavors are quite well-documented and specified. It's a lot easier to get a paper published that says "this data structure improves your performance by a factor of four and here's a proof" than "this kinda makes your code a bit easier to maintain, I think, not that I have any numbers to back it up."

A number of spatial partitioning strategies have been codified. *[BSPs][]*, *[k-d trees][]*, and *[quadtrees][]* (and *octtrees*, their 3D cousins) are the most common ones in games but there are more exotic structures floating around in academia. Because what I care about is the general *idea* of the pattern, I'm going to show you the simplest possible spatial partition that's still useful for real-world games: *a fixed grid.*

[bsps]: http://en.wikipedia.org/wiki/Binary_space_partitioning
[k-d trees]: http://en.wikipedia.org/wiki/K-d_tree
[quadtrees]: http://en.wikipedia.org/wiki/Quad_tree

### A sheet of graph paper

Here's the basic idea: imagine the entire field of battle. Now superimpose a grid of squares onto it, like a sheet of graph paper. We pick how big these squares are and then tile them across the entire game area.

Instead of a single array of units, we store them in the cells of this grid. For each cell, we store the list of units whose position falls within that cell's bounding box. When we go to handle combat, we only need to consider units within the same cell. Instead of comparing every unit in the game with every other one, we've *partitioned* the battlefield into a bunch of smaller mini-battlefields, each with a much smaller number of units to consider. (In fact, many will be empty.)

This is so <span name="simple">simple</span> you'd probably invent it yourself if you'd never heard of it, but even so it's still surprisingly effective. Enough talk, time for code!

<aside name="simple">

This is true of many design patterns. We don't document them because they are *novel*, we document them so that after we've all reinvented them ourselves, we can at least later agree on some terminology so we can talk about them effectively.

</aside>

### A grid of linked units

First let's get some prep work. Here's our basic unit class:

^code unit-simple

Each unit has a position (in 2D). And it has a pointer to the `Grid` that it lives on. This will be a bit like our array of positions in the earlier trivial example. We make `Grid` a `friend` class because, as we'll see, when a unit's position changes, it has to do an intricate dance with the grid to make sure everything is updated correctly.

Here's a sketch of the grid:

^code grid-simple

In the earlier example, we used a simple <span name="stl">array</span> to store the units at each position. We're going to do something a bit more flexible this time. Here, each "cell" is just a pointer to a unit. This works because we extend `Unit` with `next` and `prev` pointers:

^code unit-linked

This organize units in a doubly-linked list, instead of a array. Each cell in the grid then just points to the first unit in the linked list of units within that cell and each unit has pointers to the ones before and after it in the list. We'll see why soon.

<aside name="stl">

Here and throughout the book, I've deliberately avoided using any of the built-in collection types in the C++ standard library. I want to require as little external knowledge as possible to understand the code. Also, like a magician's "nothing up my sleeve", I want to make it clear *exactly* what's going on in the code. Especially with performance-related patterns this is important.

But this is my choice for *explaining* patterns. If you're just *using* them in real code, save yourself the headaches and use some of the fine collections built into pretty much every widely-used programming language today. Life's too short to code linked lists from scratch every time.

</aside>

### Entering the field of battle

The first thing we need to do is make sure new units are actually placed into the grid when they are created. We'll make unit handle this in its constructor:

^code unit-ctor

This `add()` method is defined like so:

^code add

It's a little finicky, like linked list code always is, but the basic idea is pretty simple. We find the cell the unit is sitting in and then add it to the front of that list. If there is already a list of units there, link it in after the new unit.

### A clash of swords

Once all of the units are nestled in their cells, we can let them start hacking at each other. With this new grid, the main method for handling combat look like this:

^code grid-melee

Similar to the earlier example, it just walks each cell and then calls `handleCell` on it. As you can see, we really have partitioned the battlefield into little isolated skirmishes. Each cell then handles its combat like so:

^code handle-cell

Aside from the pointer shenanigans to deal with walking a linked list, you'll note that this is exactly like our original naïve method for handling combat: it compares each pairs of units to see if they're in the same position.

The only difference is that we no longer have to compare *all* of the units to each other. That's where all of the performance benefit comes from.

### Charging forward

We've solved our performance problem, but we've created a new problem in its stead. Units now are confined to their cell. If you move a unit outside of the cell that contains it, units in the old cells won't see it anymore, but nor will anyone else. Our battlefield is a little *too* partitioned.

To fix that, we'll need to do a little work each time a unit moves. If it crosses a cell's boundary lines, we need to remove it from that cell and add it to the new one. First, we'll give `Unit` a method for changing its position:

^code unit-move

Presumably, this gets called by the AI code for computer-controlled units and by the user input code for the player's. All it does is hand-off control to the grid, which then does:

^code grid-move

That's a mouthful of code, but it's pretty straightforward. The first bit checks to see if we've crossed a cell boundary at all. If not, we can just update the unit's position and we're done. There's nothing else to do.

If it *has* left its current cell, we remove it from that cell's linked list and then add it back to the grid. Just like with a new unit, that will then insert it in the linked list for the cell that contains its new position.

This is why we're using a doubly-linked list here: we can very quickly add and remove units from lists just by setting a few pointers. With lots of units moving around each time, that can be important.

### At arms' length

This seems pretty simple, but I have cheated in one way. In the example, I've been showing, units only interact when they have the *exact same* position. That's true for checkers and chess, but less true for more realistic games. Those usually have attack *distances* to take into account.

This pattern still works fine. Instead of just checking for an exact location match, you'll do something more like:

^code handle-distance

Or probably something more advanced and based on your game's mechanics. When your game works this way, there's a corner case you need to consider: units in different cells may still be close enough to interact.

    +--------+--------+
    |       _|        |
    |      / |\       |
    |     | A|B|      |
    |      \_|/       |
    |        |        |
    +--------+--------+

Here, B is within A's attack radius even through their centerpoints are in different cells. To handle this, you will need to compare not just units in the same cell, but in neighboring cells too. To do this, first we'll split the inner loop out of `handleCell()`:

^code handle-unit

Now we have a function that will take a single unit and a list of other units and see if there are any hits. Then we'll make `handleCell()` use that:

^code handle-cell-unit

Note also that we pass in the coordinates of the cell now and not just its unit list. Right now, this doesn't do anything different than the previous example, but we'll expand it slightly:

^code handle-neighbor

Those additional `handleUnit()` calls look for hits between the current unit and <span name="neighbor">four</span> of the eight neighboring cells:

<aside name="neighbor">

If `U` is the cell with the unit, it looks at these nearby cells:

    +---+---+---+
    | X | X |   |
    +---+---+---+
    | X | U |   |
    +---+---+---+
    | X |   |   |
    +---+---+---+

</aside>

If any unit in those neighboring cells is close enough to the edge to be within the unit's attack radius, it will find the hit. We only look at *half* of the neighbors for the same reason that the inner loop starts *after* the current unit: to avoid comparing each pair of units twice.

Consider what would happen if we did check all eight neighboring cells. Let's say we have two units in adjacent cells, close enough to hit each other, like this:

    +---+---+
    |  A|B  |
    +---+---+

If, for each unit, we looked at all eight cells surrounding it, here's what would happen:

1. When we are finding hits for A, we would look at its neighbor on the right and find B. So we'd register an attack between A and B.
2. Then, when we are finding hits for B, we would look at its neighbor on the *left* and find A. So we'd register a *second* attack between A and B.

Only looking at half of the neighboring cells fixes that. *Which* half doesn't matter at all.

There's another corner case you may need to consider too. Here, I'm assuming the maximum attack distance is smaller than a cell. If you have very small cells and large attack distances, you may need to scan a bunch of neighboring cells, several rows out.

## Design Decisions

### Is the partition hierarchical or flat?

You can roughly organize spatial partition strategies into two buckets. Some break up space into a single collection of partitions, like the cells in our grid example. Others partition recursively: each partition may itself be broken into sub-partitions.

Hierarchical spatial partitions work by breaking the entire space into just a couple (usually two, four or eight) of regions. Then, if a region contains a large enough number of objects, it's recursively subdivided. This process continues until any given leaf partition only has a certain maximum number of objects in it.

Most of the big time serious spatial partition data structures you read about like BSPs and quadtrees are hierarchical, but simpler partitions like grids can be useful too.

* **If it's a single level of partitioning:**

    * *It's <span name="simpler">simpler</span>.* Flat data structures are easier to reason about, and simpler to implement.

    <aside name="simpler">

    This is a design point I mention in almost every chapter, and for good reason: whenever you can, take the simpler option. Much of programming is fighting against complexity and we have to avoid it wherever we can.

    </aside>

    * *It can be faster to update when objects change their position.* When an object moves, the data structure needs to be updated to find the object in its new location. With a hierarchical spatial partition, this can mean adjusting several layers of the hierarchy. If lots of objects are moving all the time, that can be a noticeable time sink.

* **If it's hierarchical:**

    * *It handles empty space more efficiently.* One reason hierarchical spatial partitions are used is that they handle large empty spaces more efficiently. Imagine in our earlier example if one whole side of the battlefield was empty. With our simple grid, that would lead to a large number of empty cells. We'd still walk all of those cells each frame.

    Since hierarchical space partitions don't subdivide sparse regions, a large empty space will remain a single partition. Instead of lots of little partitions to walk, there would just be the one big empty one.

    * *It handles densely populated areas more efficiently.* This is the other side of the coin: if you have a bunch of objects all clumped together, a non-hierarchical partition can be ineffective. You'll end up with one partition that has so many objects in it, you may as well not be partitioning at all. A hierarchical partition will usually adaptively subdivide that into smaller partitions and get you back to having only a few objects to consider at a time.

### Does the partitioning depend on the set of objects?

In our simple grid example, the size of the grid cells was fixed beforehand and then we just slotted units into them. Other partitioning schemes are more adaptable: they will pick the partition boundaries based on the actual set of objects and where they are in the world.

The goal is have a *balanced* partitioning, where each subset of the world has roughly the same number of objects. Keeping the partitions balanced helps performance. Consider in our grid example if all of the units were clustered in one corner of the battlefield. They'd all be in the same cell, and our code for finding attacks would regress right back to the original *O(n&sup2;)* problem that we were trying to solve.

* **If the partitioning is object-independent:**

    Our grid example is one partition that works this way: the cell size is selected up front.

    * *The space can be filled incrementally.* Adding new objects generally just means finding the right partition to add it to. You can do this one at a time without any real performance issues. If the partitions are based on the set of objects, you don't even know how things are partitioned until you have a set of objects to play with. In those cases, it's more efficient to be able to partition the entire set of objects at once.

    * *Objects can be moved quickly.* With our grid example, moving a unit just means removing it from one cell and adding it to another. If the partition boundaries themselves change based on the set of objects, then moving one can cause a boundary to move, which can in turn cause lots of other objects to need to be moved to different partitions if the boundary has moved past them.

        This is directly analagous to sorted binary search trees like red-black trees or AVL trees: when you add a single item, you may end up needing to resort the entire tree and shuffle a number of nodes around.

    * *The partitions can be imbalanced.* Of course the downside of this rigidity is that you have less control over your partitions being imbalanced. If the set of objects starts clustering together, you can degrade performance.

* **If the partitioning adapts to the set of objects:**

    Spatial partitions like BSPs and k-d trees split the world recursively so that each half contains about the same number of objects. This means that choosing the planes that you divide along requires counting how many objects are on each side.

    Bounding volume hierarchies are another spatial partition that optimize for the specific set of objects in the world.

    * *You can ensure the partitions are balanced.* This gives not just good performance, but *consistent* performance: if each partition has the same number of objects, you ensure that all queries in the world will take about the same amount of time. With games that need to maintain a consistent frame-rate, sometimes this consistency is more important than being as fast as possible in some cases.

* **If the partitioning is object-independent, but *hierarchy* is object-dependent:**

    One spatial partition deserves special mention because it has some of the best characteristics of both fixed partitions and adaptable ones: quadtrees. A quadtree partitions the world into four cells:

        +-------+-------+
        | *  *  |       |
        |       |       |
        |     * | *     |
        +-------+-------+
        |       |      *|
        |       |   *   |
        |       |       |
        +-------+-------+

    Then, for each of those squares, if the number of objects in it is large enough, it is subdivided:

        +-------+-------+
        | * |*  |       |
        |---+---|       |
        |   | * | *     |
        +-------+-------+
        |       |      *|
        |       |   *   |
        |       |       |
        +-------+-------+

    This process continues recursively until every square has fewer than some maximum number of objects. This means that the partitions don't *move* based on the set of objects, but the number of *subdivisions* is object-dependent.

    * *The space can be filled incrementally.* Adding a new object means just finding the right square and adding it. If that bumps that square above the maximum count, it gets subdivided. The other objects in that square get pushed down into the new smaller squares. This requires a little work, but it's a *fixed* amount of effort: the number of objects you have to push into the smaller squares will always be less than that maximum object count.

        Removing objects is equally simple. You remove the object from its square and if the parent square's total count is now below the threshhold, you can collapse those subdivisions.

    * *Objects can be moved quickly.* This, of course, follows from the above. "Moving" an object is just an add and a remove and both of those are pretty quick with quadtrees.

    * *The partitions are balanced.* Since any given square will have less than some fixed maximum number of objects, comparing objects withing a square can never go pathological if a bunch of objects are clustered together.

### Are objects only stored in the partition?

You can treat your spatial partition as *the* place where the objects in your game live, or you can consider it just a secondary cache to make look-up faster, while there is also a second primary collection that holds the objects.

* **If it is the only place objects are stored:**

    * *It avoids the memory overhead and complexity of two collections.* Of course, it's always cheaper to store something once instead of twice. Also, if you have two collections, you have to make sure to keep them in sync. Every time an object is created or destroyed, it has to be added or removed from both.

* **If there is another collection for the objects:**

    * *Traversing all objects is faster.* If the objects in question are "live" and have some processing they need to do, you may find yourself frequently needing to visit every object regardless of its location. Imagine if, in our earlier example, most of the cells were empty. Having to walk the full grid of them just to find the non-empty ones can be a waste of time.

    A second collection that just stores the objects gives you a way to walk all of the objects. You effectively have two data structures, one optimized for each access pattern.

## See Also

* I've tried to not discuss specific spatial partitioning structures in detail here to keep the chapter high-level (and not too long!) but your next step from here should be to learn a few of the common ones. Despite their scary names, they are all surprisingly straightforward. The common ones are:

    * [Grid](http://en.wikipedia.org/wiki/Grid_(spatial_index))
    * [Quadtree](http://en.wikipedia.org/wiki/Quad_tree)
    * [BSP](http://en.wikipedia.org/wiki/Binary_space_partitioning)
    * [k-d tree](http://en.wikipedia.org/wiki/Kd-tree)
    * [Bounding volume hierarchy](http://en.wikipedia.org/wiki/Bounding_volume_hierarchy)

* Each of these spatial data structures, is basically extending an existing well-known data structure from 1D into more dimensions. Knowing the performance characteristics of their 1D cousins will help you tell if they are a good fit for your problem:

    * A grid is effectively a [bucket sort](http://en.wikipedia.org/wiki/Bucket_sort) in multiple dimensions.
    * BSPs, k-d trees, and bounding volume hierarchies are [binary search trees](http://en.wikipedia.org/wiki/Binary_search_tree).
    * Quadtrees and octrees are [tries](http://en.wikipedia.org/wiki/Trie).
