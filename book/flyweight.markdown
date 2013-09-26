^title Flyweight
^section Design Patterns Revisited

Fog lifts, revealing a majestic old growth forest. Ancient hemlocks, countless in number, tower over the player forming a cathedral of greenery. The stained glass canopy of leaves fragments the sunlight into golden shafts of mist. Between giant trunks, the player can make out the massive forest receding into the distance.

This is the kind of otherworldly setting we dream of as game developers, and scenes like these are often enabled by a pattern whose name couldn't possibly be more modest: the humble Flyweight.

## Forest for the Trees

I can describe an sprawling woodland with just a few sentences, but actually *implementing* it in a realtime game is another story. When you've got an entire forest of individual trees filling the screen, all a graphics programmer sees is the millions of polygons they'll have to somehow shovel onto the GPU every sixtieth of a second.

You've got thousands of trees, each with detailed geometry containing thousands of polygons. Even though you might have enough *memory* to describe that forest, in order to render it, that data has to make its way over the bus from the CPU to the GPU.

Each tree has a bunch of data associated with it:

* A mesh of polygons that define the shape of the trunk, branches, and greenery.
* A few textures for the bark and leaves.
* Its location and orientation in the forest.
* Tuning parameters like size and tint so that each tree looks different.

If you were to sketch it out in code, you'd have something like this:

^code heavy-tree

That's a lot of data and the mesh and textures are particularly large. An entire forest of these objects is too much to throw at the GPU in one frame. Fortunately, there's a time-honored trick to handling this.

The key observation is that even though there may be thousands of trees in the forest, they mostly look similar. They will likely all use the <span name="same">same</span> mesh and textures. That means most of the fields in these objects are the *same* between all of those instances.

<aside name="same">

You'd have to be crazy or a billionaire (or both: a "crazillionaire") to budget for the artists to individually model each tree in an entire forest.

</aside>

We can model that explicitly by splitting our class in half. First, we pull out the data that all trees have <span name="type">in common</span> and move it into a separate class:

^code tree-model

The game only needs a single one of these, since there's no reason to have the same meshes and textures in memory a thousand times. Then, each *instance* of a tree in the world has a *reference* to that shared `TreeModel`. What's left in `Tree` is the state that is instance-specific: its position, color, and other parameters:

^code split-tree

<aside name="type">

This looks a lot like the <a href="type-object.html" class="pattern">Type Object</a> pattern. Both involve delegating part of an object's state to some other object shared between a number of instances. However, the intent behind the patterns differs.

With a Type Object, the goal is to minimize the number of classes you have to define by lifting "types" into your own object model. Any memory sharing you get from that is an extra bonus. This pattern is purely about efficiency.

</aside>

This is all well and good for storing stuff in main memory, but that doesn't help rendering. Before the forest gets onscreen, it has to work its way over to the GPU's memory. We need a way to express resource sharing that the graphics card understands.

## A Thousand Instances

To minimize the amount of data we have to push to the GPU, we want to send the shared data, the `TreeModel`, just *once*. Then, separately, we push over every tree instance's unique data -- its position, color, and scale. Then we tell the GPU "use that one model to render each of these instances".

Fortunately, today's graphics APIs and <span name="hardware">cards</span> support exactly that. The details are fiddly, and out of the scope of this book, but both Direct3D and OpenGL have support for [*instanced rendering*](http://en.wikipedia.org/wiki/Geometry_instancing).

In both APIs, you provide two streams of data. The first is the blob of common data that will be rendered multiple times -- the mesh and textures in our arboreal example. The second is the list of instances and their parameters that will be used to vary that first chunk of data each time it's drawn. With a single draw call, an entire forest appears.

<aside name="hardware">

This graphics card API means that the Flyweight pattern may be the only Gang of Four design pattern to have actual hardware support.

</aside>

## The Flyweight Pattern

Now that we've got one concrete example under our belts, I can walk you through the general pattern. Flyweight, like its name implies, comes into play when you have objects that you need to be more lightweight, generally because you have too many of them.

With instanced rendering, it's not so much that they take up too much memory as it is they take too much *time* to push each separate tree over the bus to the GPU, but the basic idea is the same.

The pattern solves that by separating out an object's data into two kinds: The first kind of data is the stuff that's not specific to a single *instance* of that object. It's the stuff that's shared across all objects. The Gang of Four calls this the *extrinsic* state, but I like to think of it as the "context-free" stuff. In the example here, this is the geometry and textures for the tree.

The rest of the data is the *intrinsic* state, the stuff that is unique to that instance. In this case, that is each tree's position, scale, and color.

Just like in the chunk of sample code up there, this pattern saves memory by sharing one copy of the extrinsic state across every place where an object appears. It separates an object from *the context where it is used*.

From what we've seen so far, this just seems like basic resource sharing, and hardly worth being called a pattern. That's partially because in this example here, we could come up with a clear separate *identity* for the shared state: the `TreeModel`.

I find this pattern to be less obvious (and thus more clever) when used in cases where there isn't a real well-defined identity for the shared object. In those cases, it feels a little more like an object is magically in multiple places at the same time. Let me show you another example.

## A Place To Put Down Roots

The ground these trees are growing on needs to be represented in our game too. There can be patches of grass, dirt, hills, lakes, rivers, and whatever other terrain you can dream up. We'll make the ground *tile-based*: the surface of the world is a huge grid of tiny tiles. Each tile is covered in one kind of terrain.

Each type of terrain has a number of properties that affect gameplay:

* A movement cost that determines how quickly players can move through it.
* A flag for whether it's a watery terrain that can be crossed by boats.
* A texture used to render it.

Because we game programmers are paranoid about efficiency, there's no way we'd store all of that state in <span name="learned">each</span> tile in the world. Instead, a common approach is to use an enum for terrain types:

<aside name="learned">

After all, we already learned our lesson with those trees.

</aside>

^code terrain-enum

Then the world maintains a huge grid of those:

<span name="grid"></aside>

^code enum-world

<aside name="grid">

The `WIDTH * HEIGHT` means we're storing a 2D grid in a 1D array. To do that, we just store each row one after the other in the array (or each column, if you're in [column-major order](http://en.wikipedia.org/wiki/Row-major_order)). I'm doing this here just to keep it simple. In real code, use a nice 2D grid data structure to hide this implementation detail.

</aside>

To actually get the useful data about a tile, we do something like:

^code enum-data

You get the idea. This works, but I find it ugly. I think of movement cost and wetness as *data* about a terrain, but here that's embedded in code. Worse, that data for a single terrain type is smeared across a bunch of methods. It would be really nice to keep all of that encapsulated together. After all, that's what objects are designed for.

It would be great if we could have an actual terrain *class*, like:

<span name="const"></span>

^code terrain-class

<aside name="const">

You'll notice that everything in this class is `const`. That's no coincidence. Since the same object is used in multiple contexts, if you were to modify it, the changes would appear in multiple places simultaneously.

That's probably not what you want. Because of this, Flyweight objects are almost always *immutable*.

</aside>

But we don't want to pay the cost of having an instance of that for each tile in the world. If you look at that class, you'll notice that there's actually *nothing* in there that's specific to *where* that tile is in the world. In flyweight terms, *all* of a terrain's state is "extrinsic" or "context-free".

Given that, there's no reason to have more than one of each terrain type. Every grass tile on the ground is identical to every other one. Instead of having the world be a grid of enums, or Terrain objects, it will be a grid of *pointers* to `Terrain` objects. Each tile that uses the same terrain will point to the same terrain instance.

Since the terrain instances are used in multiple places, their lifetimes are a little more complex to manage if you were to dynamically allocate them. Instead, we'll just store them directly in the world:

^code world-terrain

Then we could use those to lay out the world something like:

<span name="generate"></span>

^code generate

<aside name="generate">

I'll admit this isn't the world's greatest procedural terrain generation algorithm.

</aside>

Now instead of methods on `World` for accessing the terrain properties, we can just expose the `Terrain` object directly:

^code get-tile

And if you want some property of the tile, you can get it right from that object:

^code use-get-tile

We're back to the pleasant API of working with real objects. Also, `World` is now no longer coupled to all sorts of details of terrains. And we did this with almost no overhead: a pointer is often just as large as an enum.

## What About Performance?

I say "almost" here because the performance bean counters will rightfully want to know how this compares to the perf of using an enum. Storing a pointer to the object implies an indirect lookup: to get to some terrain data like the movement cost, you first have to follow the pointer in the grid to find the terrain object, and then find the movement cost there. Chasing a pointer like this can cause a cache miss, which can slow things down.

As always, the golden rule of optimization is *profile first*. Modern computer hardware is too complex for performance to be a game of pure reason anymore. In my tests while writing this chapter, there was no noticeable difference between using an enum or a flyweight object. Flyweights were actually noticeably faster. But that's entirely dependent on how other stuff was laid out in memory.

What I *am* confident of is that using flyweight objects shouldn't be dismissed out of hand. They give you the advantages of an object-oriented style without the expense of tons of objects. If you find yourself creating an enum and doing lots of switches on it, consider this pattern instead. If it turns out you need to sacrifice that for speed, at least do the tests yourself to make sure there actually is a speed cost first.

## See Also

 *  In the tile example, we just eagerly created an instance for each terrain type and stored it in `World`. That made it easy to find and reuse the one shared instance. In many cases, though, you won't want to create all of the flyweights up front.

    If you can't predict which ones you actually need, it's better to create them on demand. To get the advantage of sharing, when you go to "create" one, you first see if you've already created an identical one. If so, you just return that instance.

    This usually means that you have to encapsulate construction behind some interface that can first look for an existing object. Hiding a constructor like this is an example of the <a href="http://en.wikipedia.org/wiki/Factory_method_pattern" class="gof-pattern">Factory Method</a> pattern.

    In order to return a previously created flyweight, you'll have to keep track of the pool of ones that you've already instantiated. As the name implies, that means that an <a href="object-pool.html" class="pattern">Object Pool</a> might be a helpful place to store them.

 *  When you're using the <a class="pattern" href="state.html">State</a>, you often have "state" objects that don't have state that's specific to the machine that the state is being used in. The state's identity and methods are enough to be useful. In that case, you can apply this pattern and reuse that same state instance in multiple state machines at the same time without any problems.

 *  This pattern requires the flyweight object not have any state specific to the context in which it appears. Sometimes there's a *little* bit of context that a method in the flyweight needs access to. Since it can't *store* that data, the only option is to pass it in. That's the <a href="context-parameter.html" class="pattern">Context Parameter</a> pattern.
