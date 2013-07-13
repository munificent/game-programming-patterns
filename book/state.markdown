^title State
^section Design Patterns Revisited

- this chapter is a two-fer
- the "state pattern" in design patterns is one way to implement a certain
  higher-level pattern. it's a "design pattern" but in many ways its more
  an implementation pattern.
- the higher level pattern is "finite state machines"
- weird corner of cs that's familiar to game developers (especially ai) compiler writers, and almost no one else
- we'll cover both here and a bit about some variations and limitations

## We've all been there

- start coding input handling your player-controlled character
- left and right button presses make run left and right
- charge makes it start charging powerup
- need to keep track of how long you've been charging, so add chargeTime
- releasing charge cancels
- unless its full in which case it uses power
- wait, can't move left and right if charging, so need to check chargeTime there
- jump button makes it start jumping
- can't double jump, so add isJumping
- cancels charge if you jump while charging
- can move left and right while jumping, but not charge while jumping
- then you add ducking, and attacking, and getting hit
- you end up with a slew of flags, many combinations of which are invalid
  (what does it mean if you're ducking and jumping at the same time?)
- you keep finding bugs where you didn't consider what happens when some button is pressed in some weird state
- add longer and deeper chains of ifs
- in exasperation, you get out a notebook and scribble a flowchart
- ta-da, you just invented finite state machines

## Finite state machines to the rescue!

- after getting overwhelmed by a sea of booleans and huge branches, you cleared your head and realized
- character can be in one of a few set of states: standing, running, charging, jumping, ducking
- set of states is *finite*: you can list them out
- each state is what character "is doing". can only be in one at a time
- the way a button press is handled is determined completely by the state
- a state can handle a button press by doing a couple of things:
  - executing some chunk of code like firing the powerup, or changing velocity to jump
  - transitioning to another state
- you grab a notebook and draw boxes for each state
- you draw lines between each box and label them with buttons
  - these are the button presses that cause a transition to that state
  - you also jot down notes about behavior that happens when changing state

- (like old text adventure: state is where you are, transitions are "go north", etc.)

- this is a fsm: you have a set of states
  - when inputs come in:
    - they can cause some behavior
    - and may cause a state transition
  - otherwise, the input is ignored
- but how to code?

## Switch

- go back to code. realize first problem is that you have a bunch of bools, when you really just need an enum
- if ever have handful of bools, only one of which can be true at a time, sign you need enum
- make enum for states
- give character field that is enum value of current state
- then instead of ifs, you can switch
- changing state just means setting field to new enum value
- congrats, this the is the simplest way to implement a state machine.
- it's legit, despite what gof may have you believe.
- not perfect, though
  - big switches can get hairy, especially if you keep adding states
  - smears code for one state across class
    - you have all of the input handling code for all states in one place and updating code in another, and the rendering code in another...
    - might be easier if you had all of the code for one state together: input handling, updating, and rendering
  - character has this "chargeTime" field, but it's only meaningful in charging state
    - in other state, it has no value and isn't well-defined

## GoF State pattern

- brings us to the gof pattern
- hardcore oop guys will never use a conditional or a switch when they can use virtual dispatch
- [gof comes from smalltalk where conditionals *are* virtual dispatch]
- you can def go too far, but there's sense there
- the state pattern says, take those switch cases and replace them with classes
- define an abstract class that represents the current state the character is in
- each state (standing, running, charging, etc.) is a subclass
- each switch, each place where behavior varied by state, becomes a virtual method

- pros
  - each state is nicely encapsulated
  - can have state-specific... state
    - can move that chargetime field out of character into charging state
      - when not in that state, field doesn't exist
      - nice and neat
  - easier to maintain when you have lots of states
- cons
  - lot more complex
    - especially in languages like c++ where it takes a good bit of code to
      define a class, lot of boilerplate

- sounds a lot like strategy or type object
  - mechanics are same (delegate some part of self to object)
  - intent is different
  - strategy is given from outside
    - about decoupling class from behavior
    - doesn't usually change after creation
    - class referring to strategy may not be aware of strategy implementations
  - state subclasses are usually relatively well coupled to main class
    - focus is on *changing* state during lifetime of obj
  - type object is about delegating attributes as much as behavior
  - state = "what am i doing right now"
  - strategy = "how do i do it"
  - type object = "what kind of thing am i"

## Where do the states come from?

- have to figure out where to store instances of state
  - static ones
    - if states don't have any additional state (like charge time), no need
      to make more than one instance of each state
    - all you really need is a vtable
    - can just make since static instance
    - this is flyweight pattern
    - for love of god, don't bother with a singleton
  - if state does have state, may need one for each stateful entity
    - then have to decide if you instantiate every time change state, or
      reuse
    - instantiating can churn memory and be slow
    - reuse means hanging onto them somehow
    - more frequently state changes, more you want to avoid allocating on
      each state change
  - personally, prefer static ones
  - if do need per-instance ones, good candidate for obj pool

- talk about other implementations using tables or function pointers?

## Or don't code it at all

- one reason fsms got popular in games is because the boxes-and-lines way of
  modeling them is straightforward enough non-programmers can build them
- there are tools out there that will let you build fsms visually
- worked on games where designers drew flowcharts in program to create behavior

## Too good to be true

- this is a really cool little concept. if you've ever gone from spaghetti code
  to drawing out the clean flowchart and coding it up, it really does feel like
  a breath of fresh air.
- like an informercial for ginzu knives, makes it seem like everything is just
  so easy!
- alas, reality isn't as nice as commercials

- first, has weird problem kind of like commercial.
  - know how every commercial starts with hapless consumer struggling without
    product?
  - fsms are like that, always seem to muddle through without them before
    realizing you need one
  - but then you've got a good amount of code to rewrite
  - but if you just use state pattern up front, it can be overkill
  - gof state pattern is a big jump in code complexity over a basic switch
  - totally overkill if you just have a few states and transitions
  - just get used to idea you may need to refactor
    - when fsm works well, it's often so much clearer its worth the effort

- appeal of fsms is that they are very rigid and constrainted: once you draw
  those boxes and lines, you've instantly got a clear picture of how your
  program works
  - easy to reason about
  - easy to debug

- problem though, is it is actually quite constrained, often too constrained
- basically have to reduce all of your state to a single number
  - not even turing complete
- can lead to combinatorial explosion
- because of this, game programmers go beyond restricted cs formal defn of fsm
  in a bunch of ways
- walk through some problems and show common ways to solve them

## Concurrent machines: Buy one, get one free

- character can equip gun
- gun can be fired while running or jumping
- after firing, it has to recharge
- if you try to mix into main state machine, have lots of dupe states:
  jumping, jumping with charging gun, jumping with charged gun, etc.
- have set of states for character, and set of states for gun
- to represent all possible states in one fsm, have to have every combination
  (pair) of states
- better is to have two independent state machines running in parallel
- gun state machine is just charging -> charged
- some amount of coordination between them
- jump and run states will need to check gun state to see if character can fire
- not theoretically perfect, but works: just like game devs do

## Hierarchical state machines: inheritance for states

- lets say we break out running states a bit in order to have better animations:
  can be standing, walking, running, sliding
- in any of those states, pressing jump button starts a jump
- fire button shoots
- would be nice to not have to copy that code for each state
- one solution is hierarchical state machine (hsm)
- states can be substates and have superstates
- like subclass superclass relationship
- when an event comes in, if the substate doesn't handle it, it rolls up to super
- in other words, just like inherited methods that aren't overridden
- can also handle events in substate to override
- so we'll define a "on ground" superstate
- standing, walking, running, and sliding will all be substates
- on ground has code for jumping and others inherit it
- note: has many same problems as inheritance: substates tightly coupled to super

## Pushdown automata: A stack of states

- another problems with fsms is they have no memory
- know what state *are* in, but not what states you *were* in
- as soon as you leave a state, forget all about it

- say we want to be able to pause the game
- add new state for paused
- when that state is entered, stop game simulation, but keep listening for input
- so game can be unpaused
- can pause at any point, so character can be in any state when going to pause
  state (good place to consider hsm)
- when you unpause, needs to go back to state character was in
- if we do strict fsm, would have to have state for each one: paused-when-running, paused-when jumping, etc.
- awful
- instead, we'll store *stack* of states
- current state is state on top of stack
- can transition from one state to another by replacing top state like normal
- but can also *push* a state
  - transitions to new state
  - but leaves previous state under it on stack
- to go back, can pop state
- pausing *pushes* pause state
- resuming just pops it

- can be used for ai where you want to interrupt what an npc is doing like it hears a sound while patrolling, and then have it go back to what it was doing afterwards
- also good for ui where menu screens are often naturally stack-like with forward and back


## But wait, there's more

- above refinements are very common
- actually pretty rare to see a totally vanilla fsm
- but they still aren't a silver bullet
- even with those extensions, fsms are still pretty rigid and verbose
- good fit for thing like network protocol, input handling, simple ai
- but for game ai, which was where fsms first became popular, industry is moving away from them towards more flexible systems like behavior trees or planning systems
- good to understand fsms as a way to decompose and think about problems
- occassionally useful to actually implement, but often more just a mental tool

**misc notes:**

"Allow an object to alter its behavior when its internal state changes. The object will appear to change its class."

- state machines are related to update steps
  - often need to split behavior across a series of update calls
  - need to remember what you were doing
  - that's its "state"
- ditto for networking where stuff happens asynchronously
- generally good for systems where you need to interrupt and resume them where you left off

- compare flyweight states versus per-instance states
- consider raw function pointers as states

- can be too deterministic and lead to predictable behavior
  - can fix this by making them non-deterministic
    - transition to one of a couple of randomly chosen states

- http://www.richardlord.net/blog/finite-state-machines-for-ai-in-actionscript
- http://en.wikipedia.org/wiki/UML_state_machine#Hierarchically_nested_states
- http://sourcemaking.com/design_patterns/state
- http://en.wikipedia.org/wiki/Pushdown_automaton
- http://aigamedev.com/open/article/fsm-age-is-over
- http://www.skorks.com/2011/09/why-developers-never-use-state-machines/
- http://jessewarden.com/2012/07/finite-state-machines-in-game-development.html
- https://code.google.com/p/flash-state-engine/
- http://gamedev.stackexchange.com/questions/6005/finite-state-machine-in-c
- https://sites.google.com/site/behaviortreeeditor/thesis
- http://www.state-machine.com/psicc/PSiCC_excerpts.pdf
