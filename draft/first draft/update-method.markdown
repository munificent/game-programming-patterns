^title Update Method
^section Sequencing Patterns

## Intent

*Allow a collection of entities to simulate concurrently but independently from each other by giving each entity the opportunity to update itself once per frame.*

## Motivation

The player's mighty warrior is on a quest to steal the glorious jewels from where they rest on the bones of the long-dead sorcerer-queen. He walks to the entrance of her magnificent crypt to discover... nothing. No cursed statues shooting lightning at him. No undead warriors guarding the entrance. He can just walk right in that grab the loot. That won't do.

This crypt needs some protection: some enemies our brave hero can grapple with. First up, we want a re-animated skeleton guard to patrol back and forth in front of the door. (If the sorcerer-queen wanted more intelligent behavior, she should have re-animated something that still had brains.)

Our actual game deals with user input, rendering, and maintaining a stable framerate. That means all of the game code is nestled within a comfy little <a href="game-loop.html" class="pattern">game loop</a>. If we ignore that for the moment and imagine the simplest possible code to move our skeleton back and forth, we'd have something like:

^code just-patrol

The problem here, of course, is that the skeleton moves back and forth, but the player never sees it. That loop doesn't exit, so this just locks the game. Not exactly a fun gameplay experience. What we actually want is for the skeleton to move one step *each frame*. We'll have to remove those loops and rely on the outer game loop for iteration. That ensures the game will keep playing while the guard is making his rounds. Like:

^code patrol-in-loop

The main thing to notice is that the code got more complex. Patrolling left and right used to be two separate simple `for` loops. We kept track of which direction the skeleton was moving implicitly by where in the code we were currently executing. Now that we have to unwind out to the main game loop and then resume where we left off each frame, we have to track that explicitly using that `patrollingLeft` variable.

But, aside from that, this more or less works, so we keep going. This skeleton is a bit too stupid to give the hero much of a challenge, so the next thing we want to do is add a couple of enchanted statues. These will fire bolts of lightning at the hero every so often. Continuing along our, "what's the simplest way to code this" style, we end up with:

^code statues

I'm pretty sure you can tell this isn't ending up like something we'd enjoy maintaining. We've got an increasingly large pile of variables and imperative code all stuffed in the game loop each handling one specific entity in the game. To get a bunch of entities all alive and moving at the same time, we've mushed their code together.

The pattern we use to fix this is so simple, you probably have it in mind now just from looking at that nasty code. What we want is entities in the game to encapsulate their behavior. This keeps the game loop uncluttered and make it easy to add and remove entities.

What we need is an *abstraction layer*, and we create that by defining an abstract `update()` method on the entity class. The game loop maintains a collection of entities, but it doesn't know their concrete types. All it knows is that they have an `update()` method. Since each entity implements `update()` itself, the behavior for entities are nicely encapsulated both from the game loop, and from each other.

Each frame, it walks the collection and calls `update()` on each entity. This gives each entity a slice of time to behave each frame. By calling it on all entities every frame, they all behave simultaneously.

The game loop just has a collection of entities, so adding and removing them from the level is easy: just add and remove them from the collection. Nothing is hardcoded anymore, and we can even populate the level using some kind of data file, which is exactly what our level designers want.

## Pattern

The **game world** maintains a **collection of objects**. Each object implements an **update method** that simulates one frame of behavior. Each frame, the game updates every entity in the collection.

## When to Use It

This pattern is the jelly to <a href="game-loop" class="pattern">Game Loop</a>'s peanut butter. Almost every realtime game out there that has living actors that the player interacts with uses this pattern is some form or other. If the game has space marines, dragons, martians, ghosts, or athletes, there's a good chance it uses this pattern.

However, if the game is more abstract and the moving pieces are less like living characters and more like pieces on a boardgame, this pattern is often the *wrong* choice. In a game like chess, you don't need to simulate all of the pieces concurrently, and you probably don't need to tell the pawns to update themselves each frame.

Even if your game features entities that do have complex, real-time behavior, if the entities needs to be very highly coordinated with each other (think aliens flying in tight formations in Galaga), this pattern may cut against the grain of your problem. Update methods work best when:

* Your game has a number of entities or systems that need to run concurrently.
* Each one has its own behavior that's relatively independent of the others.
* They need to be simulated over time.

## Keep in Mind

This pattern is pretty simple, so there aren't a lot of hidden surprises in its dark corners. Still, even a simple method has a couple of ramifications.

### Splitting behavior into one frame slices makes code more complex

The first code sample I showed for the patrolling skeleton guard used its own looping directly and ran the entire patrol without returning. Then, in the second sample, that was changed to return to the outer game loop each frame.

Taking a behavior that spans multiple frames, like patrolling back and forth and flattening it out into a piece of code that exits at the end of each frame adds a good bit of complexity. You have to move state stored in local variables into something more permanent like fields on some object.

That change is <span name="coroutine">almost</span> always necessary for the game to handle user input and all of the other fun stuff that the game loop takes care of, so the first example wasn't very practical in a real game. I showed it there, just so you can see the contrast. It's worth keeping in mind there's this sunk complexity cost.

<aside name="coroutine">

I say "almost" here because it is possible to have your cake and eat it to, to have entity behavior look like straight-line imperative code that never returns, while simultaneously having a number of them running concurrently and coordinating with the game loop.

What you need is a system that lets you have multiple "threads" of execution going on at the same time. If the code for an entity can just pause and resume in the middle of what it's doing, instead of having to *return* completely, you can write it in a more imperative form.

Actual threads are almost always too heavyweight for this to work well, but if your language supports lightweight concurrency constructs like generators in Python and C#, coroutines in Lua, or goroutines in Go, you may be able to use those. The <a href="virtual-machine.html" class="pattern">Virtual Machine pattern</a> is another option that creates threads of execution at the application level.

</aside>

### You have to store state to be able to resume where you left off each frame

In the first code sample, we didn't have any variables to indicate whether the guard was moving left or right. That was implicit based on which code was currently executing. When we transformed this to an one-frame-at-a-time form, we had to create a `movingLeft` variable to track that state. Since we return out of the code each frame, the execution position is lost each frame, and we need to explicitly track enough information to restore that on the next frame.

One pattern that often helps when doing this is the <a href="state.html" class="pattern">State pattern</a>. The fact that entity behavior is split into code that executes incrementally over many frames is one reason that state machines and similar patterns are so common for entity AI.

### Entities all simulate each frame but are not truly concurrent

The basic pattern used by most games is that each frame, they loop over every entity in the world and tell each one to update one frame. The code for updating those entities often accesses parts of the world's state, including other entities in the world.

This means that the *order* that the entities are updated *within* a frame is actually meaningful. If A updates before B, then when B updates, it will <span name="double-buffer">see</span> A's *new* position, not its previous one. Even though from the player's perspective everything is moving at the same time, the core of the game is still turn-based. It's just that a "turn" is one frame long.

<aside name="double-buffer">

If, for some reason, you decide you *don't* want your game to be sequential like this, you would need to use something like the <a href="double-buffer.html" class="pattern">Double Buffer</a> pattern. That makes the order that A and B update not matter because both of them will be seeing the *previous* frame's state for both entities.

</aside>

The fact that the game is still technically sequential is mostly a good thing. It certainly makes the code simpler and requires less memory because you don't have to store both the previous and current state for each entity.

It's a boon as far as the game logic is concerned too. Allowing entities to truly update independently from each other leads you to some unpleasant semantic corners. Imagine a game of chess where black and white moved at the same time. They both try to make a move that places a piece in the same currently empty square. How should this be resolved?

Keeping things sequential makes that much simpler: the world is in a consistent state *before* each entity takes its turn, and is consistent *after* its turn too. Instead of having to determine how a set of simultaneous moves are reconciled together, the game state just updates incrementally.

### Be careful modifying the entity list while updating

When you're using this pattern, a lot of the game's behavior ends up nestled in these update methods. That includes behavior that adds or removes entities from the game.

For example, a monster summons another of its kin to help. With new entities, you can usually just add them to the end of the list without too much trouble. You'll keep iterating over that list and eventually get to the new entity at the end and update it too. But that does mean that the new entity gets a chance to act during the frame that it was spawned, before the player has has a chance to even see it.

If you don't want that to happen, one simple fix is to store the number of entities in the list at the beginning of the update loop and only update that many before stopping:

^code skip-added

Here, `entities` is an array of the entities in the game, and `numEntities` keeps track of how many are in it. When new entities are added, that gets incremented. We cache its value in `numEntitiesThisTurn` at the beginning of the loop so that the iteration stops before we get to any new entities added in the middle of the current update loop. They'll be updated the next frame.

A slightly hairier problem is when entities are *removed* while iterating. You vanquish some foul beast and now it needs to get yanked out of the entity list. If it happens to be right after the current entity you're updating in the list, you can accidentally skip an entity:

^code skip-removed

This simple loop increments the index of entity being updated each iteration. But imagine the entity list array looks like:

    0. Foul Beast
    1. Hero
    2. Hapless Peasant

We're updating the hero, so `i` is 1. He slays the foul beast so it gets removed from the array. The hero shifts up to 0, and the hapless peasant shifts up to index 1. After updating the hero, `i` is incremented to 2, skipping right over the peasant.

This can do even nastier things if you're using something like a linked list for your entities and you happen to remove the entity currently being updated. One option is to just be careful when you remove entities and update any iteration variables to take the removal into account.

Another option is to defer actual removals until after you're done walking the list. Just mark the entity as "dead" but leave it there. If it comes up for being updated, skip it. Then, when you've updated everything, walk the list again but *only* to remove the dead entities.

## Sample Code

This pattern is one of those simple ones where the sample code almost belabors the point. That doesn't mean the pattern isn't useful. It's so useful in part *because* it's simple: it's a clean solution to a problem without a lot of ornamentation.

But just to keep things concrete, let's walk through a basic implementations. We'll start with our entity class:

^code entity-class

I stuck a few things in there, but just the bare minimum we'll need later. Presumably in real code there'd be lots of other stuff in there too like, graphics, physics, etc. We'll just ignore that for now. The important bit for this pattern is that it has a *virtual* `update()` method.

The game maintains a collection of these entities. In our sample, we'll put that in a class representing the game world:

<span name="array"></span>

^code game-world

<aside name="array">

In a real-world program, you'd probably use some real collection type, but I'm just using a vanilla array here to keep things simple.

</aside>

Now that everything is set up, the game implements the pattern by updating each entity every frame:

<span name="game-loop"></span>

^code game-loop

<aside name="game-loop">

As the name of the method implies, this is an example of the <a href="game-loop.html" class="pattern">Game Loop</a> pattern.

</aside>

### Subclassing entities?!

Now that we've got the basic skeleton up and running, we can start using the pattern to let us define some entity behaviors. Before I get started, though, I need to make a formal apology. In this example, I've hung the abstract `update()` method directly on the `Entity` class. That means implementing `update()` requires subclassing `Entity`.

Right now, the popular way to define your game entities is using the <a href="component" class="pattern">Component</a> pattern. Using that, `update()` would be on the components themselves instead of entity. That lets you avoid creating complicated class hierarchies of entities to define behaviors. Instead, you can just mix and match components.

Component systems are popular for a reason: they're much more flexible and maintainable than big rigid class hierarchies. But this also means that using inheritance here is now really *un*popular and some people think it's a moral crime for me to even use it in an example.

But this chapter isn't about components. It's about `update()` methods, and the <span name="subclass">simplest</span> way I can show them, with as few moving parts as possible, is by just putting that method right on `Entity` and making a few subclasses.

<aside name="subclass">

Just between you and me, I think the pendulum has swung a bit too far away from subclassing. Sure, avoiding it is better than the 90's OOP hysteria where the more inheritance you used the better, but being dogmatic about *not* using inheritance is as bad as being dogmatic about using it.

We shouldn't discard tools from our toolbox because they have sharp edges. We should just learn to use them carefully. Often the sharp edge is what helps it cut effectively.

</aside>

### Defining entities

OK, back to the task at hand. Our original motivation was to be able to define a patrolling skeleton guard and some lightning-bolt-unleashing magical statues. Let's start with the former. To define the patrolling behavior, now we just make a new kind of entity that implements `update()` appropriately:

^code skeleton

As you can see, we pretty much just cut that chunk of code from the game loop earlier in the chapter and pasted it into `Skeleton`'s `update()` method. The one minor difference is that `patrollingLeft_` has been made into a field instead of a local variable. That way its value sticks around between calls to `update()`.

Let's do this again with the statue:

^code statue

Again, most of the change is just moving code out of the game loop and into the class and renaming some stuff. In this case, though, we've actually made things simpler. In the original nasty imperative code, there were separate local variables for each statue's current number of frames since it last fired, and also for each statue's delay between firing.

Now that those have been moved into the `Statue` class itself, each statue takes care of that state on its own. You can create as many statues as you want and each one will have its own little timer and rate of fire independently of all of the others. And that's really the motivation behind this pattern: it's now much easier to add new entities to the game world because each one brings along everything it needs to take care of itself.

This pattern lets us separate out *populating* the game world from *implementing* it. Each entity now includes its own behavior and the game engine itself doesn't have any explicit code driving the entities in the world. This means all you need to do is instantiate and entity and add it to the world's `entities_` collection, and the entity will take it from there.

This in turn gives us the flexibility to populate the world using something like a separate data file or level editor. That's a huge win for productivity because it means non-programmers on the team can build the world and iterate on it without needing a coder in the loop. The "separation of concerns" in the code, where each entity contains its own behavior, can translate to a separation of concerns on your *team* where some people define how entities behave while others define which entities are present in each level.

## Design Decisions

With a simple pattern like this, there isn't too much variation, but there's still a couple of knobs you can turn.

### What class does `update()` method live on?

The most obvious and most important decision you'll make is what class to put `update()` on.

* **The entity class**

    This is the simplest option. If you already have an entity class, this doesn't require bringing any addition classes into play. This may work if you don't have too many kinds of entities, but the industry is generally moving away from this.

    This is because you have to subclass the entity class to define new behavior. That can be brittle and painful if you have a large number of different kinds of behavior. The entity class will often contain lots of other code besides behavior: physics, graphics, etc. You may want to subclass to vary those instead (for example, to distinguish between visible and invisible entities, or for the different kinds of physical bodies). If you need to vary entities along multiple axes simultaneously, using subclassing to express doesn't work well.

* **The component class**

    If you're already using the <a href="component.html" class="pattern">Component pattern</a>, this is a no-brainer. Your
components will invariably need to be updated each frame, so you'll pretty much
have to do this.

    It lets each component update itself independently. In the same way that the update pattern in general lets you decouple game entities from each other in the game world, this lets you decouple parts of a single entity from each other. Rendering, physics, and AI can all take care of themselves.

* **A delegate class**

    There are other patterns that involve delegating part of a class's behavior to another object. The <a href="state.html" class="pattern">State pattern</a> does this so that you can change the entity's behavior by changing the object its delegating to. The <a href="type-object.html" class="pattern">Type Object</a> pattern does this so that you can share behavior across a bunch of entities of the same "kind".

    If you're using one of those patterns, it's natural to put `update()` on that delegated class. In that case, you may still have the `update()` method on `Entity`, but it will just forward to the delegate object. Something like:

    ^code forward

    Doing this lets you define new behavior by changing out the delegated object. Like using components, it gives you the flexibility to change entity behavior without having to define an entirely new entity class.

### How are dormant objects handled?

You often have a number of objects in the world that for whatever reason don't need to be updated sometimes. It could be dead, or hidden, or disabled. If a large number of objects are in this state, it can be a waste of CPU cycles to walk over them each frame only to do nothing.

Another option is to maintain a separate collection of just the "live" objects that do need updating. When an object is disabled, it's removed from the collection. When it's re-enabled, it gets added back. This way, you only walk over items that actually have real work do to.

* **If you use a single collection containing inactive objects:**

    * *You waste CPU cycles*. For inactive objects, you'll end up either checking some "am I enabled" flag or calling a method that does nothing.

* **If you use a separate collection of only active objects:**

    * *You use extra memory to maintain the second collection.* There's still usually another master collection of all entities for cases where you need them all. In that case, this collection is overhead from a memory perspective. When speed is tighter than memory (which it often is), this can be a worthwhile trade-off.

        Another option to mitigate this is to have two collections, but have the other collection only contain the *inactive* entities instead of all of them.

    * *You have to keep the collections in sync.* When objects are created or completely destroyed (and not just made temporarily inactive), you have to remember to modify both the master collection and this one.

The metric that should guide your approach here is how many inactive objects you tend to have. The more you have, the more useful it is to have a separate collection that avoids them during your core game loop.

## See Also

* This pattern is part of a trinity with <a href="game-loop.html" class="pattern">Game Loop</a> and <a href="component.html" class="pattern">Component</a> that often form the nucleus of the game engine.

* Microsoft's XNA platform uses this pattern both in the [`Game`](http://msdn.microsoft.com/en-us/library/microsoft.xna.framework.game.update.aspx) and [`GameComponent`](http://msdn.microsoft.com/en-us/library/microsoft.xna.framework.gamecomponent.update.aspx) classes.

* The [Quintus](http://html5quintus.com/) JavaScript game engine uses this pattern on its main [`Sprite`](http://html5quintus.com/guide/sprites.md) class.