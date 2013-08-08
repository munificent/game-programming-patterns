^title Update Method
^section Sequencing Patterns

## Intent

*Allow a collection of entities to simulate concurrently but independently from each other by giving each entity the opportunity to update itself once per frame.*

## Motivation

The player's mighty warrior is on a quest to steal the glorious jewels from where they rest on the bones of the long-dead sorceror-queen. He walks to the entrance of her magnificent crypt to discover... nothing. No cursed statues shooting lightning at him. No undead warriors guarding the entrance. He can just walk right in that grab the loot. That won't do.

This crypt needs some protection: some enemies our brave hero can grapple with. First up, we want a re-animated skeleton guard to patrol back and forth in front of the door. (If the sorceror-queen wanted more intelligent behavior, she should have re-animated something that still had brains.)

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

It's a boon as far as the game logic is concerned too. Allowing entities to truly update independently from each other leads you to some <span name="physics">unpleasant</span> semantic corners. Imagine a game of chess where black and white moved at the same time. They both try to make a move that places a piece in the same currently empty square. How should this be resolved?

Keeping things sequential makes that much simpler: the world is in a consistent state *before* each entity takes its turn, and is consistent *after* its turn too. Instead of having to determine how a set of simultaneous moves are reconciled together, the game state just updates incrementally.

<aside name="physics">

Physics engines often *do* handle all moving entities in one frame simultaneously.

**TODO research and fill this in**

</aside>

- need to be careful about modification of entity list

    - when using update method pattern, lot of game behavior exists in the middle
      of iterating over list of entities (or components)
    - often game behavior means changing that list
    - monster drops treasure: need to add treasure
        - can usually just add it to end of list
        - keep in mind this means entity will get a chance to act in the same
          frame that it was spawned in, before it renders
        - may be good
        - may be bad because player doesn't have chance to see it
        - if update loop stores number of entities in list before it starts
          looping, can stop before new entities
        - then they don't get processed until next turn
    - you kill enemy: needs to be removed
        - if you immediately remove and enemy was next in list, loop may skip
          over an entity
        - if using something like a linked list, can end up in bad memory
        - be careful
        - may want to defer list removals until after update loop
        - add to list of "dead" entities and then remove them later

## Sample Code

- have a basic entity class

        class Entity
            int x, y

- give abstract update method

        update()

- game engine has collection of entities in game

        code...

- main game loop updates every entity once per frame

        gameloop():
            for entity in entities: entity.update()

- typically does this before physics and rendering

- then define subclasses for different kinds of entities with different behavior
- using subclassing here is heresy for many. used to be absolutely most common
  way, but components are more popular now. in practice, probably would use
  component. not here for simplicity.

- start with patroller

    class SkeletonGuard : Entity
      update()
        ...

- earlier, had local variable for position and just had for loop to walk from one side to other
- now that behavior has to be split across multiple update calls, variable has to be hoisted out of local and into field
- for loop is gone. (game loop basically drives that for us.)
- have to add other variable to remember which direction going.
- in earlier code, that was implicit based where in function we were. in first loop, going right, in second left. cpu's instruction pointer kept track of this.
- now, since we return after each frame, we have to remember that explicitly so we can resume where we left off.
- often end up with something state-machine like for behavior. see state chapter

- now do statue

    class Statue : Entity
      update()
        ...

- pretty simple

- what did we learn?

  - compared to imperative code for single entity, splitting into single-frame slices is a lot of work.
  - have to hoist local variables into fields
  - have to add some state to remember where in behavior we were
  - but compared to imperative code for *multiple* entities, much simpler. where before we had statue code mixed into patrol loops, that's all elegantly separated out.
  - code for statue is actually simpler than it was before.
  - since almost all games have multiple entities on screen, this is almost always a win.
  - now we can add and remove entities at whim. can even have *data* determine which entities level has. just drop entity on and it will do what it's supposed to.

- one other entity in game: hero!
- can just make him an entity too
- only difference is that update() looks at user input
- literally only difference
- automatically ensures player and enemies behavior is correctly synchronized

## Design Decisions

### What class does `update()` method live on?

**Main entity class**

- simplest
- no other classes
- means subclassing for each kind of entity
- industry moving away from this

**Component**

- if using component pattern already, components will have update method
- main class then just automatically calls update on all components
- if game loop updates entities which then update components, can be pointless
  pointer chasing. see structure of arrays
- probably most common pattern today

**State**

- if using fsm for entity, may have state object.
- can often put update there
- may let you reuse pieces of behavior across different entities if states are
  meaningful for more entities
- changing states gives you simple way of changing behavior
- avoids having to subclass each entity

### What order are entities updated in?

- to user, entities seem to behave simultaneously, but really are taking turns
- just that turn is 1/60 second
- since it is

**Arbitrary**

- simplest answer is just whatever order they happen to be in the list
- new entities get added to the end, stuff gets removed
- for real-time games, this usually works fine
- "turn" is only a single frame long, so order isn't really user visible
- as long as every entity only gets one update per turn, and the order is stable,
  it's fair
- as long as you don't shuffle the list, no entity will get to go twice before
  any other does

**Prioritized**

- if doing turn-based game, may actually having a timing system where some entities
  move "faster" (i.e. more frequently) than others
- explain amaranth-style energy system?
- priority queue?

### What do you iterate over?

- each frame, have to call update() methods of every object that needs updating
- when entities are added/removed, need to update this collection
- how do we do that efficiently?

**entities**

- simplest: just collection of all entities in game
- probably have this collection already
- if have lots of entities that don't need updating (dormant, inactive, etc.)
  can spend a lot of time walking over entities for no reason
- if using component model, can spend lot of time visiting entities just to
  look up component

**components**

- if using component model, update is usually handled by component, not entity
  itself
- in that case, walking entity just serves to get you to component
- can optimize by having list of just those components
- then walk that directly
- have to make sure to keep this updated when entities added/removed
- don't want zombie component
- see structure of arrays pattern

**updatable entities**

### How is collection stored?

**dynamic array**

- appending to end is fast, but removing is slow
- can use something like object pool to make removal faster: leave empty holes
  and fill later
- need to be careful because that affects update order

**linked list**

- deletion is fast
- insertion too, but usually just adding to end anyway
- can do a lot of pointer chasing and cache invalidation

TODO: does that matter? array is array of pointers anyway (since vtable) so
pointer chasing either way?

## See Also


- random notes

    - very old games used to orchestrate all enemies. that's why you had lots more
      patterns of enemies instead of them behaving independently.
    - talk about looping through all entities here. game loop doesn't cover that.
    - game loop just calls update(), so isn't coupled to details of how each
      entity/system does its thing
    - compare to using threads for each entity which means os scheduler would
      control stuff. bad!
    - have a bunch of things in the game that are all moving "simultaneously"
    - not literally at same time: even real-time games are a little more formal
      and everything actually takes its turn
    - if really tried to be at same time, would need to make sure entities only
      accessed *previous* state. would need to double buffer. makes things much
      more complex. instead, entities are sequential.
    - don't want high level code telling each entity "ok you do this, you do that"
    - instead, want each entity to own its own behavior. that way changing one
      entity in level doesn't affect others.
    - hard thing about update method is that have to get imperative code and slice
      it into tiny chunks. have to remember where you were so can pick up after
      where you left off. state pattern can help. vm pattern can too in different
      way.
    - if using component pattern, entity update will usually just delegate to
      components.
    - important that update method is fast. if one entity takes a long time, slows
      whole game.
    - don't necessarily need strict 1-entity to 1 update method system. if game
      play works on groups of entities (formations, etc.), could have update for
      entire group


