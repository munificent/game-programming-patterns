^title Update Method
^section Sequencing Patterns

## Motivation

- hero trying to enter crypt. have undead guard patrolling entrace, back and forth. not very smart, but very loyal.

- do straight imperative code:

        while true
          for x = 0 to 100
            set pos
            render

          for x = 100 to 0
            set pos
            render

- code is monopolizing cpu. no other entities on level get to do anything
  no input handling. no game loop.

- want to add statue enemy that fires at player every few frames

        frame = 0
        while true
          for x = 0 to 100
            entity1.set pos
            if frame++ % FIRE == 0 then entity2.fire
            render

          for x = 100 to 0
            entity1.set pos
            if frame++ % FIRE == 0 then entity2.fire
            render

- nasty, all code for entities is mixed together. imagine we want to add
  another patrolling entity that moves at a different speed.
- want to be able to mix and match entities on levels
- adding entities shouldn't affect others
- means want each entities behavior to be self-contained

- solution is to split out behavior for each entity with entity
- instead of code that does behavior over multiple frames without returning,
  only simulates one frame
- everywhere it used to render frame, now returns/yields
- can then handle multiple entities by interleaving these calls
- call it once each from on every entity
- now entities run concurrently and independently

## Pattern

???

## When to Use It

- most real-time game uses this, called directly from game loop
- turn-based games may use it too, but decoupled from game loop: only call when
  entity has turn
- if game entities/pieces are particularly simple and don't do much (chess,
  checkers), may not be needed
- use when:

    - have number of game entities or systems
    - entities have own independent behavior
    - need to move over time

## Keep in Mind

- splitting up behavior into one frame at a time makes code more complex

    - almost always need to do this anyway to play nice with game loop
    - very hard to control framerate with straight line imperative code

- have to remember where you left off each frame

    - lot of temporary state has to be moved into fields on the entity so that
      it persists across calls

    - state pattern can help

    - vm pattern too

- behavior is more concurrent but still turn-based

    - each entity gets slice of time, but order that they are processed in
      still matters since later entities will see updated state of earlier
      ones

    - is mostly a good thing. to do real simultaneous sim, would need double
      buffer.

        - lots of memory and complexity.
        - bad behavior: two entities see square is empty and both try to move
          into it in parallel. what do?
        - sequential is almost always better.
        - TODO: how do physics systems handle this? research

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


