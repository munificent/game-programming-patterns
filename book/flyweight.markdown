^title Flyweight
^section Design Patterns Revisited

A horn rings out a battle call in the distance. Starting with just a few tiny specks, a giant army crests over a hill, hordes of soldiers rushing down to crash into the waiting battlefield below. This is the kind of epic scenarios we dream of as game developers, and scenes like this are often enabled by a pattern whose name couldn't possibly be more modest: the humble Flyweight.

## An Army of Soldiers

I can describe a scene like the above with just a few sentences, but actually *implementing* it in a realtime game is enough to make a seasoned graphics programmer blanch. When you've got huge armies filling the screen, all they see is the millions of polygons they'll have to somehow cram onto the GPU in less than a sixtieth of a second.

You've got hundreds, maybe thousands of soldiers, each with a detailed mesh contains thousands of polygons. Even though you might have enough *memory* to hold that battlefield, in order to render it, that has to get pushed from the CPU over the bus to the GPU.

Fortunately, there's a time-honored trick for handling this. Each warrior on the ground has a bunch of data associated with it:

* A mesh of polygons that define the shape and details of his skin
* A few textures painted onto that skin
* A skeleton of vertices used to animate that mesh
* The soldier's location on the battlefield
* The positions of all of the joints in their skeleton: their current pose.
* Maybe some other tuning parameters like a scale or tint to add some visual
  variety to the army.

The key observation here is that even though there may be thousands of soldiers on the battlefield, they mostly look the same. They will likely all use the <span name="same">same</span> mesh, skeleton, and textures. That means most of the data in that list is the *same* between all of those units.

<aside name="same">

You'd have to be crazy or a billionaire (or both) to budget for the artists to individually model each lowly grunt in the military.

</aside>

If you were to sketch this out in code, you'd have something like this:

    class Soldier {
    private:
      Mesh* mesh;
      Skeleton* skeleton;
      Texture* texture;
      Vector position;
      double height;
      Color skinTone;
    };

Each instance of `Soldier` would have its own `position`, `height`, and `skinTone`, but those pointers to `mesh`, `skeleton`, and `texture` would all point to the same objects. There's no reason to have the same mesh in memory a thousand times.

This is all well and good for storing stuff in main memory, but that doesn't help rendering. Before the army gets onscreen, it's got to work it's way over to the GPU's memory. We need a way to model this that the graphics card understands. The answer is *instanced rendering*.

## A Thousand Instances

To minimize the amount of data we have to push to the GPU, we'd like to send the shared data -- the mesh, skeleton, and textures -- *once* and then push over each soldier instance's unique data -- its position, height, and color. Then we'd tell the GPU "use that shared data to render each of these instances".

Fortunately, today's graphics APIs support exactly that.

**TODO: Show D3D and GL APIs.**

## The Flyweight Pattern

Now that we've got one concrete example under our belts, I can walk you through the general pattern. Flyweight, like its name implies, comes into play when you have objects that you need to be more lightweight: generally because you have too many of them.

With instanced rendering, it's not so much that they take up too much memory as it is they take too much *time* to push each separate soldier over the bus to the GPU. But the basic idea is the same.

The pattern solves that issue by separating out the object's data into two kinds: The first kind of data is the stuff that's not specific to a single *instance* of that object. It's the stuff that's shared across all objects. The Gang of Four calls this the *extrinsic* state, but I like to think of it as the "context-free" stuff. In the example here, this is the geometry and textures for the soldier.

The rest of the data is the *intrinsic* state, the stuff that is unique to that instance. In this case, that's the soldier's position, height, and color.

Just like in the chunk of sample code up there, this pattern saves memory by moving the extrinsic state *out* of the main object and into something shared. It separates out an object from *the context where it is used*.

From what we've seen so far, this just seems like basic resource sharing, and hardly worth being called a pattern. That's because the soldier example didn't have any behavior. To get a better feel for how this pattern works, let's switch to a different example.

## The Field of Battle

The battlefield these soldiers are marching around on needs to be represented in our game too. There can be patches of grass, dirty, hills, and whatever other terrain you can dream up. To make things retro-cool, let's decide to make the world *tile-based*: the ground will be a huge grid of tiny tiles. Each tile can be one of a few different terrain types: grass, dirt, hill, water, you get the idea.



** TODO: talk about perf**

## The Flyweight Pattern

**NOTES, delete when done:**

http://sourcemaking.com/design_patterns/flyweight

"Extrinsic state is stored or computed by client objects, and passed to the Flyweight when its operations are invoked."

- in other words, can make flyweight operations context-free by passing in context.

- some overlap with type object. both patterns delegate some of object's definition
  to other shared object.

http://javapapers.com/design-patterns/flyweight-design-pattern/
"Flyweight is used when there is a need to create high number of objects of almost similar nature. High number of objects consumes high memory and flyweight design pattern gives a solution to reduce the load on memory by sharing objects. It is achieved by segregating object properties into two types intrinsic and extrinsic."


- pattern is sort of about saving memory, but memory is cheap
  - however, cache is not


gof:
"Facilitates the reuse of many fine grained objects, making the utilization of large numbers of objects more efficient."


- got refers to "instrinsic" and "extrinsic" state. prefer "contextual" and "context-independent".

- common place where pattern is used in games
  - explain instanced rendering
  - key concepts: separate out data specific to single object in world (transform)
    from data it has in common with others (mesh)
  - then reuse the latter
  - talk about how rendering api supports instanced rendering directly: hardware
    understands flyweight pattern.
- then say, that's general idea, here's other application
- tiles or other places where you'd use an enum and lots of switches
- can instead do a pointer
- talk about perf of pointer chasing, field lookup, virtual method invoke
- discuss using flyweight both for data and for behavior

- flyweights almost always immutable: if you change it, everything that uses it
  will change

- create them upfront or as needed? if latter, think about threading

- compare to string interning
