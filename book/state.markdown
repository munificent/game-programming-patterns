^title State
^section Design Patterns Revisited

Man, have I got a deal for you. This chapter is ostensibly about the State design pattern. But that pattern is really just one way to implement a more abstract pattern called a *finite state machine* (typically just "FSM"). These in turn have a few close cousin data structures: *hierarchical state machines* and *pushdown automata*.

By the end of the chapter, you should have a handle on all of those. That's a lot to cram in one chapter, though. To keep things as short as possible, the code samples here leave out a few details that you'll have to fill in on your own. I hope they're still clear enough for you to get the big picture.

State machines occupy a funny corner of computer science. They're not well known to most programmers, but <span name="two-camps">two camps</span> of coders are experienced with them: game AI developers and compiler writers. I think state machines are a good tool for other problems, too, so I'll be using a non-AI example here. First, though, let's see how we might find ourselves wanting one.

<aside name="two-camps">

In a strange way, this mirrors the early days of artificial intelligence. In the 50s and 60s, much of AI research was focused on language processing. Many of the techniques compilers now use for parsing programming languages were initially invented for parsing human languages.

</aside>

## We've All Been There

We're working on a little side-scrolling platformer. Our job is to implement the heroine that is the player's avatar in the game world. That means making her respond to user input. Push the B button and she should jump. Simple enough:

^code spaghetti-1

Spot the bug?

There's nothing to prevent our "air jumping": keep hammering B while she's in the air and she will float indefinitely. The simple <span name="landing">fix</span> is to add an `isJumping_` boolean field to `Heroine` that tracks when she's jumping and then do:

^code spaghetti-2

<aside name="landing">

There should also be code that sets `isJumping_` back to `false` when the heroine touches the ground. I've omitted that here for brevity's sake.

</aside>

Next, we want the heroine to duck if the player presses down while the she's on the ground, and stand back up when it the button is released:

^code spaghetti-3

Spot the bug this time?

With this code, the player could:

1. Press down to duck.
2. Press B to jump from a ducking position.
3. Release down while still in the air.

The heroine will switch to her standing graphic in the middle of the jump. Time for another flag...

^code spaghetti-4

It would be cool if the heroine did a dive attack if the player presses down in the middle of a jump:

^code spaghetti-5

Bug hunting time again. Find it?

We check that you can't air jump while jumping, but not while diving. Yet another field...

Something is clearly <span name="se">wrong</span> with our approach. Every time we touch this handful of conditionals and interrelated boolean fields we add bugs to it. We intend to support a variety of moves&mdash;we haven't even added *walking* yet!&mdash;but at this rate it will collapse into a heap of bugs before we're done with it.

<aside name="se">

Those coders you idolize who always seem to create flawless code aren't simply superhuman programmers. Instead, they have an intuition about which *kinds* of code are error-prone, and they steer away from that.

Complex conditional logic and mutable state&mdash;fields that change over time&mdash;are two of those error-prone kinds and this code has both.

</aside>

## Finite State Machines to the Rescue

In a fit of frustraction, you sweep everything off your desk except a pen and paper and start drawing a flowchart. You draw a box for each thing the heroine can be doing: standing, jumping, ducking, and diving. When she can respond to a button press in one of those states, you draw an arrow from that box, label it with that button, and connect it to the state she changes to.

**TODO illustrate**

Congratulations, you've just created a *finite state machine*. These came out of a branch of computer science called *automata theory* whose family of data structures also include the famous Turing machine. FSMs are the simplest member of that family.

The <span name="adventure">gist</span> is:

* You have a fixed *set of states* that the machine can be in. For our example, that's standing, jumping, ducking, and diving.

* The machine can only be in *one* state at a time. Our heroine can't be jumping and standing simultaneously. In fact, preventing that is one reason we're going to use an FSM.

* A sequence of *inputs* or *events* are sent to the machine. In our example, that's the raw button presses and releases.

* Each state has a *set of transitions*, each associated with an input and pointing to a state. When an input comes in, if it matches a transition on the current state, the machine changes to the state that transition points to.

    For example, pressing down while standing transitions to the ducking state. Pressing down while jumping transitions to diving. If no transition is defined for an input on the current state, the input is ignored.

In their pure form, that's the whole banana: states, inputs, and transitions. You can draw it out like a little flowchart. Unfortunately, the compiler doesn't recognize our scribbles, so how do we go about *implementing* one? One way is the Gang of Four State pattern, which we'll get to, but let's start simpler.

<aside name="adventure">

My favorite analogy for FSMs is an old text adventure game like Zork. You have a world of rooms that are connected to each other by exits. You explore them by entering commands like "go north".

This maps directly to a state machine: rooms are the states. The room you're in is the current state. Each room's exits are its transitions. The navigation commands are the inputs.

</aside>

## Enums and Switches

One problem our `Heroine` class has is some combinations of those boolean fields aren't valid: `isJumping_` and `isDucking_` should never both be true, for example. When you have a handful of flags where only one is `true` at a time, that's a hint that what you really want is an *enum*.

In this case, that enum is exactly the state for our FSM, so let's define that:

^code enum

Instead of those boolean fields, `Heroine` will just have one `state_` field. We also flip the order of our branching. In the previous code, we switched on input, *then* on state. This kept the code for handling one button press together, but smeared around the code for one state. We want to keep the code for each state together, so we switch on that first. That gives us:

^code state-switch

This seems trivial, but it's a real improvement over the previous code. We still have some conditional branching, but we simplified the <span name="invalid">mutable</span> state to a single field. All of the code for handling a single state is now nicely lumped together. This is the simplest way to implement a state machine and is perfect for some uses.

<aside name="invalid">

In particular, the heroine can no longer be in an *invalid* state. With the boolean flags, some sets of values were possible but incorrect. With the enum, each value it can be in is meaningful.

</aside>

Your problem may outgrow this solution, though. Say we want to add a move where if our heroine ducks for a while, she charges up and then unleashes a special attack. While she's ducking, we need to track the charge time.

We add a `chargeTime_` field to `Heroine` to store how long the attack has charged. Assume we already have an <span name="update">`update()`</span> that gets called each frame. In there, we add:

^code switch-update

<aside name="update">

If you guessed that this is the <a href="update-method.html" class="pattern">Update Method</a> pattern, you win a prize!

</aside>

We need to reset the timer when she starts ducking, so we modify `handleInput()`:

^code state-switch-reset

All in all, to add this charge attack we had to modify two methods and add a `chargeTime_` field right onto `Heroine`, even though it's only meaningful while in the ducking state. What we'd prefer is to have all of that code and data nicely wrapped up in one place. The Gang of Four has us covered.

## The State Pattern

For people deeply into the object-oriented mindset, every conditional <span name="branch">branch</span> is an opportunity to use dynamic dispatch ("virtual method call" in C++ parlance). I think you can go too far down that rabbithole. Sometimes an `if` is all you need.

<aside name="branch">

There's a historical basis for this. Many of the original object-oriented apostles like Design Patterns' Gang of Four, and Refactoring's Martin Fowler came from Smalltalk. There, `ifThen:` is just a method you invoke on the condition, which is implemented differently by the `true` and `false` objects.

</aside>

But in our example, we've reached a tipping point where something object-oriented is a better fit. That gets us to the State pattern. In the words of the Gang of Four:

> Allow an object to alter its behavior when its internal state changes. The object will appear to change its class.

That doesn't tell us much. Heck, our `switch` does that. The concrete pattern they describe, applied to our heroine looks like this:

### A state interface

First, we define an interface for the state. Every bit of behavior that is state-dependent (i.e. every place we had a `switch` before) becomes a virtual method in that interface. For us, that's `handleInput()` and `update()`:

^code heroine-state

### Classes for each state

For each state, we define a class that implements the interface. Its methods define the heroine's behavior when in that state. In other words, take each case from the earlier switches and move them into their state's class. For example:

^code ducking-state

Note that we also moved `chargeTime_` out of Heroine and into the `DuckingState` class. This is great: that piece of data is only meaningful while in that state, and now our object model reflects that explicitly.

### Delegate to the state

Next, we give the `Heroine` a pointer to its current state and then lose those big switches and delegate to the state instead:

<span name="delegate"></span>

^code gof-heroine

In order to "change state", we just need to assign `state_` to point to a different `HeroineState` object. That's the State pattern in its entirety.

<aside name="delegate">

This looks like the <a href="http://en.wikipedia.org/wiki/Strategy_pattern" class="gof-pattern">Strategy</a> and <a href="type-object.html" class="pattern">Type Object</a> patterns. In all three, you have a main object that delegates to another subordinate one. The difference is *intent*.

* With Strategy, the goal is to *decouple* the main class from some portion of its behavior.

* With Type Object, the goal is to make a *number* of objects behave similarly by *sharing* a reference to the same type object.

* With State, the goal is for the main object to *change* its behavior by *changing* the object it delegates to.

</aside>

## Where Are the State Objects?

I did gloss over one bit here. To change states, we need to assign `state_` to point to the new state, but where does that object come from? With our enum implementation, that was a no-brainer: enum values are just primitive values like numbers. But now our states are classes which means we need an actual instance to point to. There are two common answers to this:

### Static states

If the state object doesn't have any other <span name="fn">fields</span>, then the only data it stores is a pointer to the internal virtual method table so that its methods can be called. In that case, there's no reason to ever have more than one instance of it: every instance would be identical anyway.

<aside name="fn">

If your state has no fields and only *one* virtual method in it, you can simplify the pattern. Replace each state *class* with a state *function*, just a plain vanilla top-level function. Then the `state_` field in your main class becomes a simple function pointer.

</aside>

In that case, you can just make a single static instance of it. Even if you have a bunch of FSMs all going at the same time in that same state, they can all point to the <span name="flyweight">same instance</span> since it has nothing machine-specific about it.

<aside name="flyweight">

This is the <a href="http://en.wikipedia.org/wiki/Flyweight_pattern" class="gof-pattern">Flyweight Pattern</a>.

</aside>

*Where* you put that static instance is up to you. Find a place that makes sense. For no particular reason, let's put ours inside the base state class:

^code heroine-static-states

Each of those static fields is the one instance of that state that the game uses. To make the heroine jump, the standing state would do something like:

^code jump

### Instantiated states

Sometimes, though, this doesn't fly. A static state won't work for the ducking state. It has a `chargeTime_` field, and that's specific to the heroine that happens to be ducking. This may coincidentally work in our game because there's only one heroine, but for something like an enemy state where you can have multiple enemies on screen, they each need their own instance.

In that case, you'll actually have to <span name="fragment">create</span> a state object when you transition to it, like:

^code duck

This lets each FSM have its own instance of the state. When I can, I prefer to use static states since they don't burn memory and CPU cycles allocating objects each state change. For states that have more, uh, *state*, though, this is the way to go.

<aside name="fragment">

When you dynamically allocate states, you may have to worry about fragmentation. An <a href="object-pool.html" class="pattern">Object Pool</a> can help.

</aside>

## Enter and Exit Actions

The goal of the state pattern is to encapsulate all of the behavior and data for one state in a single class. We've done that pretty well, but there's still one loose end. Our `DuckingState` class handles user input and updating the charge time, but there's one lingering bit of ducking-specific code left floating around. Over in the standing state, when we *start* ducking, it does some initialization:

^code start-ducking

The ducking state should really take care of resetting the charge time (after all, it's the object that has that field) and playing the animation. We can handle that by giving states have *entry actions*. This is just a method we call when we transition to a state:

^code ducking-with-enter

Back in `Heroine` we wrap changing the state in a little convenience method:

^code change-state

Now, in the standing code, we have:

^code enter-ducking

Now the state really is encapsulated. One particularly nice thing about entry actions is that they run when you enter the state regardless of which state you're coming *from*.

Most real-world state graphs have multiple transitions into the same state. For example, maybe our heroine can fire her weapon while standing, ducking, and jumping. That means you end up duplicating some code at each place where that transition occurs. Entry actions give you a way to consolidate that.

We can, of course, also extend this to support *exit actions*. This is just a method we call on the state we're *leaving* right before we switch to the new state.

## What's the Catch?

I've spent all this time selling you on FSMs and now I'm going to pull the rug out from under you. Everything I've said so far is true, and they are a good fit for some problems. But their greatest virtue is also their greatest flaw.

State machines help you untangle hairy code by enforcing a very <span name="turing">constrained</span> structure on it. All you've got a fixed set of states, a single current state, and some hardcoded transitions.

<aside name="turing">

A finite state machine isn't even *Turing complete*. Automata theory describes computation using a series of abstract models, each more complex than the previous. A *Turing machine* is one of the most expressive models.

"Turing complete" means a system (usually a programming language) is expressive enough to implement a Turing machine in it, which means all Turing complete languages are, in some ways, equally expressive. FSMs are not expressive enough to do this.

</aside>

If you try using a state machine for something more complex like game AI, you will crash face-first into the limitations of that model. Fortunately, our game programming forebears have found ways to dodge some of those barriers. I'll close this chapter out by walking you through a couple of them.

## Concurrent State Machines

We've decided to give our heroine the ability carry a gun. When she's packing heat, she can still do everything she could before: run, jump, duck, etc. But, she'll also need to be able to fire her weapon while doing it.

If we want to stick to the confines of an FSM, we have to *double* the number of states we have. For each state we already had, we'll need one for when she's armed: standing, standing with gun, jumping, jumping with gun, you get the idea.

Add a couple of more weapons and the number of states explodes combinatorially. Not only is it a huge number of states, it's a huge amount of duplication: the unarmed and armed states are almost identical except for the little bit of code to handle firing.

The problem is that we've <span name="combination">jammed</span> two sets of states&mdash;what she's *doing* and what she's *carrying*&mdash; into a single machine. To model all possible combinations, we need a state for each *pair*. The fix is obvious: have two separate state machines.

<aside name="combination">

If we want to cram *n* states for what she's doing and *m* states for what she's carrying into a single machine, we need *n &times; m* states instead of a much smaller *n + m*.

</aside>

We keep our original state machine for what she's doing and leave it alone. Then we define a separate state machine for what she's carrying. `Heroine` will have *two* "state" references, one for each, like:

^code two-states

For illustrative purposes, we use the full State pattern for her equipment. In practice, since it just has two states, a simple boolean flag would suffice. Imagine that later we'll be adding more guns, ammo, and add-ons, so we would eventually have `UnarmedState`, `PistolState`, `ShotgunState`, etc.

When the Heroine delegates inputs to the states, it hands it to both of them:

<span name="consume"></span>

^code handle-two-inputs

<aside name="consume">

A more full-featured system would probably have a way for one state machine to *consume* an input so that the other doesn't receive it. That would prevent both machines for erroneously trying to respond to the same input.

</aside>

Each state machine can the respond to inputs, spawn behavior, and change its state independently of the other machine. When the two sets of states are mostly unrelated, this works well.

In practice, you'll find a few cases where the states do interact. For example, maybe she can't fire while jumping, or maybe she can't do a dive attack if she's armed. To handle that, in the code for one state, you'll probably just do some crude `if` tests of the *other* machine's state to coordinate them. Not the most elegant solution, but it gets the job done.

## Hierarchical State Machines

After fleshing out our heroine's behavior some more, she'll likely have a bunch of similar states. For example, she may have standing, walking, running, and sliding states. In any of those, pressing B jumps and pressing down ducks.

With a simple state machine implementation, we have to duplicate that code in each of those states. It would be better if we could implement that once and reuse it across all of the states.

If this was just object-oriented code instead of a state machine, one way to share code across those states is by using <span name="inheritance">inheritance</span>. We could define a class for an "on ground" state that handles jumping and ducking. Standing, walking, running, would then inherit from that and add their own additional behavior.

<aside name="inheritance">

This has both good and bad implications. Inheritance is a powerful means of code reuse, but also a very strong coupling between two chunks of code. It's a big hammer, so swing it carefully.

</aside>

It turns out, this is a common implementation technique called a *hierarchical state machine*. A state can have a *superstate* (which makes it a *substate*). When an event comes in, if the substate doesn't handle it, it rolls up to its superstate which can then take it over. That superstate in turn may have its own superstate to delegate to, all the way up the chain. In other words, it works just like overiding inherited methods.

In fact, if we're using the actual State pattern to implement your FSM, we can use class inheritance to implement the hierarchy. We define a base class for the superstate:

^code on-ground

And then each substate inherits it:

^code duck-on-ground

This isn't the only way to implement the hierarchy, of course. If you aren't using the Gang of Four's state pattern, this won't work. Instead, you can model the current state's chain of superstates explicitly using a *stack* of states instead of a single state in the main class.

The current state is the one on the top of the stack, under that is its immediate superstate, and then *that* state's superstate and so on. When you dish out some state-specific behavior, you start at the top of the stack and walk down until one of the states handles it. (If none do, you just ignore it.)

## Pushdown automata: Remembering where we left off

There's another common extension to finite state machines that also uses a stack of states. Confusingly, the stack represents something entirely different than the state of superstates in an HSM, and is used to solve a different problem.

The problem is that finite state machines have no concept of *history*. You know what state you *are* in, but have no memory of what state you *were* in. There's no easy way to go back to a previous state.

Here's an example: Earlier, we let our fearless heroine arm herself to the teeth. When she fires her gun, we need a new state that plays the firing animation and spawns the bullet and any visual effects. So we slap together a `FiringState` and make <span name="shared">all of the states</span> that she can fire from transition into that when the fire button is pressed.

<aside name="shared">

Since this is behavior duplicated across several states, this may also be a good place to use a hierarchical state machine to reuse that code.

</aside>

The tricky part is what state she transitions to *after* firing. She can pop off a round while standing, running, jumping, and ducking. When the firing sequence is complete, she should transition back to what she was doing before.

If we're sticking with a vanilla FSM, we'll have to define a slew of nearly identical states&mdash;firing while standing, firing while running, firing while jumping, and so on&mdash;just so that each one can have a hardcoded transition that goes back to the right state when it's done. What we'd really like is a way to *store* the state she was in before firing and then *recall* it later. Again, automata theory is here to help. The relevant data structure is called a [*pushdown automaton*](http://en.wikipedia.org/wiki/Pushdown_automaton).

Where a finite state machine has a *single* pointer to a state, a pushdown automaton has a *stack* of them. In an FSM, transitioning to a new state *replaces* the previous one. A pushdown automaton lets you do that, but also gives you two additional operations:

1. You can *push* a new state onto the stack. The "current" state is always the one on top of the stack, so this transitions to the new state. But it leaves the previous state directly under it on the stack instead of discarding it.

2. You can *pop* the topmost state off the stack. That state is discarded, and the state under it becomes the new current state.

This is just what we need for firing. We create a *single* firing state. When the fire button is pressed while in any other state, we *push* the firing state onto the stack. When the firing animation is done, we *pop* that state off, and the pushdown automaton automatically transitions us right back to the state we were in before.

## So How Useful Are They?

Even with those common extensions to state machines, they are still pretty limited. The trend these days in game AI is more towards exciting things like [behavior trees](http://www.altdevblogaday.com/2011/02/24/introduction-to-behavior-trees/) and [planning systems](http://web.media.mit.edu/~jorkin/goap.html). If AI is what you're interested in, all this chapter has done is whet your appetite. You'll want read other books to satisfy it.

This doesn't mean finite state machines, pushdown automata, and other simple systems aren't useful. They're a good modelling tool for certain kinds problems. You may not yank them out of the toolbox often, but you should still have them in there.

Finite state machines are useful when:

1. You have an entity whose behavior changes based on some internal state.
2. That state can be rigidly divided into one of a relatively small number of distinct options.
3. The entity responds to a series of inputs or events over time.

In games, they are most known for being used in AI, but they are also common in implementations of network protocols, user input handling, navigating menu screens, and parsing text. When you find a problem where a state machine seems like a good fit, the Gang of Four's State pattern is a decent (if verbose) way of implementing it in many object-oriented languages.
