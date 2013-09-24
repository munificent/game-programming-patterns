^title Flyweight
^section Design Patterns Revisited

The fog lifts, revealing a majestic old growth forest. Huge trees, countless in number, tower over the player forming a cathedral of greenery. The stained glass canopy of leaves fragments the sunlight into golden shafts of glimmering dust. Between giant trunks, he can make out the massive forest receding into the distance.

This is the kind of otherworldly setting we dream of as game developers, and scenes like these are often enabled by a pattern whose name couldn't possibly be more modest: the humble Flyweight.

## Forest for the Trees

I can describe an enormous forest with just a few sentences, but actually *implementing* it in a realtime game is enough to make a seasoned graphics programmer blanch. When you've got an entire forest of individual trees filling the screen, all they see is the millions of polygons they'll have to somehow shove onto the GPU in less than a sixtieth of a second.

You've got thousands of trees, each with detailed geometry containing thousands of polygons. Even though you might have enough *memory* to describe that forest, in order to render it, that has to get pushed from the CPU over the bus to the GPU.

Each tree has a bunch of data associated with it:

* A mesh of polygons that define the shape of the trunk, branches, and greenery.
* A few textures for the bark and leaves.
* The location and orientation in the forest.
* Tuning parameters like size and tint so that each tree looks different.

If you were to sketch it out in code, you'd have something like this:

^code heavy-tree

Many of those fields are themselves pretty large objects, in particular the mesh and textures. An entire forest of these large objects would be too much to throw at the GPU in one frame. Fortunately, there's a time-honored trick to handling this.

The key observation is that even though there may be thousands of trees in the forest, they mostly look similar. They will likely all use the <span name="same">same</span> mesh and textures. That means most of the data in that class is the *same* between all of those objects.

<aside name="same">

You'd have to be crazy or a billionaire (or both) to budget for the artists to individually model each tree in the entire forest.

</aside>

We can model that explicitly by splitting our class in half. First, we take the data that all trees have <span name="type">in common</span> with each other and move that into a separate class:

^code tree-model

The game only needs a single one of these, since there's no reason to have the same meshes and textures in memory a thousand times. Then, each *instance* of a tree in the world has a *reference* to that shared model. It then adds the state that is instance-specific: its position, color, and other parameters.

^code split-tree

<aside name="type">

This looks a lot like the <a href="type-object.html" class="pattern">Type Object</a> pattern. Both involve delegating part of an object's state to some other object shared between a number of instances. However, the intent behind the patterns differs.

With a Type Object, the goal is to be able to define many different types of objects without having to define different *classes*. Even though a single instance of the type object will be shared across multiple objects, there are still *other* type objects used by other instances. Type objects make it easier to have many different types.

This pattern is purely about sharing resources: it is often used with only a single instance of the shared state.

</aside>

This is all well and good for storing stuff in main memory, but that doesn't help rendering. Before the forest gets onscreen, it's got to work its way over to the GPU's memory. We need a way to express this data sharing that the graphics card understands.

## A Thousand Instances

To minimize the amount of data we have to push to the GPU, we want to send the shared data, the `TreeModel`, just *once*. Separately, we push over every tree instance's unique data, its position, color, and scale. Then we tell the GPU "use that one model to render each of these instances".

Fortunately, today's graphics APIs and <span name="hardware">cards</span> support exactly that. The details are fiddly, and out of the scope of this book, but both Direct3D and OpenGL have support for *instanced rendering*. In both APIs, you provide two streams of data. The first is the blob of common data that will be rendered multiple times: the mesh and textures. The second is the sequence of instances and their parameters that will be used to vary that first chunk of data each time it's drawn.

<aside name="hardware">

This graphics card API means that the Flyweight pattern may be the only Gang of Four design pattern to have actual hardware support.

</aside>

## The Flyweight Pattern

Now that we've got one concrete example under our belts, I can walk you through the general pattern. Flyweight, like its name implies, comes into play when you have objects that you need to be more lightweight, generally because you have too many of them.

With instanced rendering, it's not so much that they take up too much memory as it is they take too much *time* to push each separate tree over the bus to the GPU. But the basic idea is the same.

The pattern solves that issue by separating out an object's data into two kinds: The first kind of data is the stuff that's not specific to a single *instance* of that object. It's the stuff that's shared across all objects. The Gang of Four calls this the *extrinsic* state, but I like to think of it as the "context-free" stuff. In the example here, this is the geometry and textures for the tree.

The rest of the data is the *intrinsic* state, the stuff that is unique to that instance. In this case, that is each tree's position, scale, and color.

Just like in the chunk of sample code up there, this pattern saves memory by moving the extrinsic state *out* of the main object and into something shared. It separates an object from *the context where it is used*.

From what we've seen so far, this just seems like basic resource sharing, and hardly worth being called a pattern. That's partially because in this example here, we could come up with a clear separate *identity* for the shared state: the `TreeModel`.

I find this pattern to be less obvious (and thus more clever) when used in cases where there isn't a real well-defined identity for the shared object. In those cases, it feels a little more like an object is magically in multiple places at the same time. Let me show you another example.

## A Place To Put Down Roots

The ground these trees are growing on needs to be represented in our game too. There can be patches of grass, dirt, hills, lakes, rivers, and whatever other terrain you can dream up. We'll make the ground *tile-based*: the surface of the world will be a huge grid of tiny tiles. Each tile can be one of a few different terrain types: grass, dirt, hill, water, you get the idea.

Each type of terrain has a number of properties that affect gameplay:

* A movement cost that determines how quickly players can move through it.
* A flag for whether it's a watery terrain that can be crossed by boats.
* A texture used to render it.

Because we game programmers are paranoid about efficiency, there's no way we'd store all of that state in each tile in the world. Instead, a common approach is to use an enum for tile terrain types:

^code terrain-enum

Then the world maintains a huge grid of those:

<span name="grid"></aside>

^code enum-world

<aside name="grid">

The `WIDTH * HEIGHT` means we're storing a 2D grid in a 1D array. All that requires is storing each row one after the other in the array (or each column, if you're in [column-major order](http://en.wikipedia.org/wiki/Row-major_order)). I'm doing this here just to keep it simple. In real code, use a nice 2D grid data structure to hide this implementation detail.

</aside>

To actually get the useful data about a tile, we'd do something like:

^code enum-data

You get the idea. This works, but I find it a bit ugly. You've got all of the
data for a terrain type smeared across a bunch of methods. It would be really nice to keep all of that encapsulated together. After all, that's what objects are designed for.

It would be great if we could have an actual terrain *class*, like:

<span name="const"></span>

^code terrain-class

<aside name="const">

You'll notice that everything in this class is `const`. That's no coincidence. Flyweight objects are almost always *immutable* -- their state doesn't change once created. Since the same object is used in multiple contexts, if you were to modify it, the changes would appear in multiple places simultaneously, exposing the object sharing.

</aside>

But we don't want to pay the cost of having an instance of that for each tile in the world. If you look at that class, notice that there's actually *nothing* in there that's specific to *where* that tile is in the world. Given that, there's no reason to have more than one of each terrain type. Every grass tile on the ground is identical to every other one.

Instead of having the world be a grid of enums, or Terrain objects, it will be a grid of *pointers* to `Terrain` objects. Each tile that uses the same terrain will point to the same terrain instance.

Since the terrain instances are used in multiple places, their lifetime is a bit more complex to manage if you were to dynamically allocate it. Instead, we'll just store them directly in the world:

^code world-terrain

Then we could use those to lay out the world something like:

<span name="generate"></span>

^code generate

<aside name="generate">

I'll admit this isn't the world's greatest procedural terrain generation algorithm.

</aside>

Now instead of methods on `World` for accessing the terrain properties, we can just expose that directly:

^code get-tile

And if you want some property of the tile, you can get it right from that object:

^code use-get-tile

We're back to the pleasant API of working with real objects, but with almost none of the memory overhead.

## What About Performance?

I say "almost" here because the performance bean counters will rightfully want to know how this compares to the perf of using an enum. Storing a pointer to the object implies an indirect lookup: to get to some terrain data like the movement cost, you first have to follow the pointer in the grid to find the terrain object, and then find the movement cost there. Chasing a pointer like this can cause a cache miss, which can slow things down.

As always, the golden rule of optimization is *profile first*. Modern computer hardware is too complex for performance to be a game of pure reason anymore. In my tests while writing this chapter, there was no noticeable difference between using an enum or a flyweight object. If anything, the latter was a bit faster. But that's entirely dependent on how other stuff was laid out in memory.

What I *am* confident is that using flyweight objects shouldn't be dismissed out of hand here. I think they often give you the advantages of an object-oriented style -- mainly encapsulation -- without the expense of tons of objects. If you find yourself creating an enum and doing lots of switches on it, it's worth trying this pattern instead. If it turns out you need to sacrifice that for speed, at least do the tests yourself to make sure there actually is a speed cost first.

## See Also

 *  In the tile example, we just eagerly created an instance for each terrain type and stored it in `World`. That made it easy to find and reuse the one shared instance. In many cases, though, you won't want to create all of the flyweights up front.

    If you don't know which ones you'll actually need, it's better to create them as needed. To get the advantage of sharing, when you go to "create" one, you'll first see if you've already created an identical one. If so, you just return that instance.

    This usually means that you'll have to encapsulate construction behind some interface that can first look for an existing object. Hiding a constructor like this is an example of the <a href="http://en.wikipedia.org/wiki/Factory_method_pattern" class="gof-pattern">Factory Method</a> pattern.

    In order to return a previously created flyweight, you'll have to keep track of the pool of them that you've already instantiated. As the name implies, that means that an <a href="object-pool.html" class="pattern">Object Pool</a> might be a helpful place to store them.

 *  When you're using the <a class="pattern" href="state.html">State</a>, you sometimes have state objects that don't have any, uh, state, that's specific to the machine that the state is being used in. The state's identity alone is enough to be useful. In that case, you can apply this pattern and reuse that same state instance in multiple state machines at the same time without any problems.

 *  This pattern requires the flyweight object not have any state specific to the context in which it appears. Sometimes that's easy, but other times there's a *little* bit of context that it would be useful for a method in the flyweight object to have access to. Since it can't *store* that data, the only option is to pass it into the method. When you do, that's the <a href="context-parameter.html" class="pattern">Context Parameter</a> pattern.
