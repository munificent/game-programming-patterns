^title Flyweight
^section Design Patterns Revisited

A horn rings out a battle call in the distance. Starting with just a few tiny specks, a giant army crests over a hill, hordes of soldiers rushing down to crash into the waiting battlefield below. This is the kind of epic scenarios we dream of as game developers, and scenes like this are often enabled by a pattern whose name couldn't possibly be more modest: the humble Flyweight.

## An Army of Soldiers

I can describe a scene like the above with just a few sentences, but actually *implementing* it in a realtime game is enough to make a seasoned graphics programmer blanch. When you've got huge armies filling the screen, all they see is the millions of polygons they'll have to somehow cram onto the GPU in less than a sixtieth of a second.

You've got hundreds, maybe thousands of soldiers, each with detailed geometry containing thousands of polygons. Even though you might have enough *memory* to hold that battlefield, in order to render it, that has to get pushed from the CPU over the bus to the GPU.

Each warrior on the ground has a bunch of data associated with it:

* A mesh of polygons that define the shape and details of his skin
* A few textures painted onto that skin
* A skeleton of vertices used to animate that mesh
* The soldier's location on the battlefield
* Their current pose -- the positions of all of the joints in their skeleton
* Maybe some other tuning parameters like a scale or tint to add some visual
  variety to the army.

If you were to sketch it out in code, you'd have something like this:

^code heavy-soldier

Many of those fields are themselves pretty large objects: the `mesh` and `texture` in particular are a handful. Fortunately, there's a time-honored trick to handling this.

The key observation is that even though there may be thousands of soldiers on the battlefield, they mostly look the same. They will likely all use the <span name="same">same</span> mesh, skeleton, and textures. That means most of the data in that list is the *same* between all of those units.

<aside name="same">

You'd have to be crazy or a billionaire (or both) to budget for the artists to individually model each lowly grunt in the military.

</aside>

We can show that explicitly be tweaking our object model. We'll split our class in half. First, we take the data that all soldiers have <span name="type">in common</span> and move that into a separate class:

^code soldier-model

The game only needs a single one of these, since there's no reason to have the same mesh in memory a thousand times. Then, each *instance* of a soldier in the world has a *reference* to that since shared model. It then adds the state that is instance-specific: its `pose`, `position`, and other parameters.

^code split-soldier

<aside name="type">

This looks a lot like the <a href="type-object.html" class="pattern">Type Object</a> pattern. Both involve delegating part of an object's state to some other object shared between a number of instances. However, the intent behind the patterns differs.

With a Type Object, the goal is to be able to define many different types of objects without having to define different *classes*. Even though a single instance of the type object will be shared across multiple objects, there are still *other* type objects used by other instances. Type objects make it easier to have many different types.

This pattern is purely about sharing resources: it is most often used with only a single instance of the shared state.

</aside>

This is all well and good for storing stuff in main memory, but that doesn't help rendering. Before the army gets onscreen, it's got to work it's way over to the GPU's memory. We need a way to model this that the graphics card understands. The answer is *instanced rendering*.

## A Thousand Instances

To minimize the amount of data we have to push to the GPU, we'd like to send the shared data -- the `SoldierModel` -- *once* and then push over each soldier instance's unique data -- its position, pose, height, and color. Then we'd tell the GPU "use that one model to render each of these instances".

Fortunately, today's graphics APIs and <span name="hardware">cards</span> support exactly that. The details are fiddly, and out of the scope of this book, but both Direct3D and OpenGL have support for instanced rendering. In both cases, you provide two streams of data. The first is the blob of data that will be rendered multiple times: the mesh, texture and other details. The second is the sequence of instances and their parameters that will be used to tweak that first chunk of data each time it's drawn.

<aside name="hardware">

This fact means that the Flyweight pattern may be the only Gang of Four design pattern to have actual hardware support.

</aside>

## The Flyweight Pattern

Now that we've got one concrete example under our belts, I can walk you through the general pattern. Flyweight, like its name implies, comes into play when you have objects that you need to be more lightweight: generally because you have too many of them.

With instanced rendering, it's not so much that they take up too much memory as it is they take too much *time* to push each separate soldier over the bus to the GPU. But the basic idea is the same.

The pattern solves that issue by separating out the object's data into two kinds: The first kind of data is the stuff that's not specific to a single *instance* of that object. It's the stuff that's shared across all objects. The Gang of Four calls this the *extrinsic* state, but I like to think of it as the "context-free" stuff. In the example here, this is the geometry and textures for the soldier.

The rest of the data is the *intrinsic* state, the stuff that is unique to that instance. In this case, that's the soldier's position, height, and color.

Just like in the chunk of sample code up there, this pattern saves memory by moving the extrinsic state *out* of the main object and into something shared. It separates out an object from *the context where it is used*.

From what we've seen so far, this just seems like basic resource sharing, and hardly worth being called a pattern. That's partially because in this example here, we could come up with a clear separate *identity* for the shared state: the `SoldierModel`.

I find this pattern to be less obvious (and thus more clever) when used in cases where there isn't a real well-defined identity for the shared object. In those cases, it feels a little more like an object is magically in multiple places at the same time. Let me show you another example.

## The Field of Battle

The battlefield these soldiers are marching around on needs to be represented in our game too. There can be patches of grass, dirt, hills, lakes, rivers, and whatever other terrain you can dream up. To make things retro-cool, let's decide to make the world *tile-based*: the ground will be a huge grid of tiny tiles. Each tile can be one of a few different terrain types: grass, dirt, hill, water, you get the idea.

Each type of terrain has a number of properties that affect gameplay:

* A movement cost that determines how quickly soldiers can move through it.
* A flag for whether it's a watery terrain that can be crossed by boats.
* How opaque the tile is: a number indicating how much it reduces the visibility radius.
* A texture used to render it.

Because we game programmers are paranoid about efficiency, there's no way we'd store all of that state in each tile on the battlefield. Instead, a common approach is to use an enum for tile terrain types:

^code terrain-enum

Then the battlefield maintains a huge grid of those:

<span name="grid"></aside>

^code enum-battlefield

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

But we don't want to pay the cost of having an instance of that for each tile on the battlefield. If you look at that class, notice that there's actually *nothing* in there that's specific to *where* that tile is in the world. Given that, there's no reason to have more than one of each terrain type. Every grass tile in the world is identical to every other one.

Instead of having the battlefield be a grid of enums, or Terrain objects, it will be a grid of *pointers* to `Terrain` objects. Each tile that uses the same terrain will point to the same terrain instance.

Since the terrain instances are used in multiple places, their lifetime is a bit more complex to manage if you were to dynamically allocate it. Instead, we'll just store them directly in the battlefield:

^code battlefield-terrain

Then we could use those to lay out the battlefield something like:

<span name="generate"></span>

^code generate

<aside name="generate">

I'll admit this isn't the world's greatest procedural terrain generation algorithm.

</aside>

Now instead of methods on `Battlefield` for accessing the terrain properties, we can just expose that directly:

^code get-tile

And if you want some property of the tile, you can get it right from that object:

^code use-get-tile

We're back to the pleasant API of working with real objects, but with almost none of the memory overhead.

## What About Performance?

I say "almost" here because the performance bean counters will rightfully want to know how this compares to the perf of using an enum. Storing a pointer to the object implies an indirect lookup: to get to some terrain data like the movement cost, you first have to follow the pointer in the grid to find the terrain object, and then find the movement cost there. Chasing a pointer like this can cause a cache miss, which can slow things down.

As always, the golden rule of optimization is *profile first*. Modern computer hardware is too complex for performance to be a game or pure reason anymore. In my tests, there was no noticeable difference between using an enum or a flyweight object. If anything, the latter was a bit faster. But that's entirely dependent on how other stuff was laid out in memory.

What I *am* confident is that using flyweight objects shouldn't be dismissed out of hand here. I think they often give you the advantages of an object-oriented style -- mainly encapsulation -- without the expense of tons of objects. If you find yourself creating an enum and doing lots of switches on it, it's worth trying this pattern instead. If it turns out you need to sacrifice that for speed, at least do the tests yourself to make sure there actually is a speed cost first.
