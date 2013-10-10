^title Command
^section Design Patterns Revisited

I'm pumped about this chapter because I find the Command pattern to be one of the most useful ones in my arsenal. I think most large programs I write, games or otherwise, end up using it somewhere. Using it in the right place has very neatly untangled some really gnarly code.

For such a cool pattern, the Gang of Four has a typically abstruse way of describing it:

> Encapsulate a request as an object, thereby letting users parameterize clients with different requests, queue or log requests, and support undoable operations.

Is that terrible? Yes, yes, that's terrible. First of all, it's mangling whatever metaphor its trying to established. Outside of the weird world of software where words can mean anything, a "client" is a *person*, someone you do business with. Last I checked, people can't be *parameterized*.

Then, the rest of that sentence is just a list of stuff you could maybe possible use the pattern for. Not very illuminating.

*My* pithy way of describing the command pattern is: **a command is a *reified* method call.** Of course, "pithy" often means "so terse it's impenetrable", so this may not be much of an improvement. Let me unpack that a bit.

"Reify", in case you've never heard it, means "make real". It comes from the Latin "res" for "thing" with the English suffix "fy" which means "to make or cause to be". So "reify" basically means "thingify", which, honestly, would be a better and more fun word to use.

When we talk about <span name="reflection">"reifying"</span> in programming, we usually mean making something "first class". And that, in turn, means taking some *concept* and turning it into an actual piece of real data that you can store in memory, stick in variables, pass to functions, return from functions, etc.

<aside name="reflection">

One example of reification in languages is *reflection* systems. Reflection lets you work with the types in your program imperatively, at runtime. You can get an object that represents the class or type of some other object, and you can play with that to see what the type can do. In other words, reflection is a *reified type system*.

</aside>

When you make some *concept* a real *object* in your program, it means you can use all of the features your language has for manipulating data on it. Unlike "second-class" things that you can't deal with directly, first class objects have handles that you can grab onto and manipulate.

So by saying the command pattern is a "reified method call", what I mean is that it's a method call wrapped in an object. That sounds a lot like a "callback", first class function" or a "function pointer", or a "closure", or a "partially applied function", depending on which language you're coming from, and indeed those are all in the same ballpark. The Gang of Four later says:

> Commands are an object-oriented replacement for callbacks.

Which would be a much better slugline for the pattern than the one they chose. But all of this is abstract and nebulous. I like to start chapters with something concrete, and I blew that here. To make up for it, the rest of the chapter will run through a handful of concrete examples where I think the command pattern is a brilliant fit.

## Configuring Input

If you reduce videogames to some Skinner-esque minimum, the basic idea is the user pushes buttons and exciting stuff happens. Different buttons do different (but, presumably still exciting) stuff. OK, as a design document, that probably needs a bit of fleshing out, but it's enough to get us started.

Somewhere in every game codebase is some code that reads in raw user input, button presses, keyboard events, mouse clicks, whatever. For each input, it triggers some meaningful action in the game. Something like this:

<span name="lurch"></span>

^code handle-input

<aside name="lurch">

Pro tip: Don't press B very often.

</aside>

This function usually gets called once per frame by the <a class="pattern" href="game-loop.html">Game Loop</a> and you can figure out what it does. This works if we're willing to hardwire user inputs to game actions. But many games let the use *configure* how their inputs are wired up.

To support that, we need to replace those direct calls to `jump()` and `fireGun()` with something that we can swap out. "Swapping out" sounds a lot like assigning a variable, so we need an object that we can use to represent a game action. Enter the Command pattern.

We define a base class that represents a triggerable game command:

^code command

Then we create a subclasse for each of the different game actions:

^code command-classes

In our input handler, we'll have a pointer to a command for each button:

^code input-handler-class

Now the input handling just delegates to those:

<span name="null"></span>

^code handle-input-commands

<aside name="null">

Note that we're not checking for `null` here. We're assuming each button will have *some* command wired up to it. If you want to be able to have buttons do nothing, but don't want to have to check for `null` everytime, you can define a command class whose `execute()` method does nothing. Then, instead of setting a button handler to `null`, you point it to that object.

It does the same thing, but doesn't require a check. This is an example of a pattern called [Null Object](http://en.wikipedia.org/wiki/Null_Object_pattern).

</aside>

This is the command pattern in a nutshell. If you can see the utility of this now, consider the rest of this chapter bonus.

## Directing Game Actors

The command classes we just defined work for that example, but they're a bit limited. The problem is that they assume there are these *global* `jump()`, `fireGun()`, etc. functions that they can call which presumably know how to find the player's avatar and make him dance like the puppet he is.

That implicit coupling limits the usefulness of those commands. The *only* thing the `JumpCommand` can make jump is the player. Let's loosen the restriction. Instead of calling functions that find the commanded object directly, we'll *pass in* the object that we want to order around:

^code actor-command

Here, `GameActor` is our main "game object" class that represents a character in the game world. We pass it in to `execute()` so that the derived command can invoke methods on any object of our choice, like so:

^code jump-actor

Now we can use this one class to make any character in the game hop around. The missing piece is a little bit of code between the input handler and the command that can take the command and invoke it on the right object. First, we'll change `handleInput()` so that it *returns* commands:

^code handle-input-return

Instead of executing the command immediately (which it can't do, since it doesn't know what actor to pass in), it just returns it. As you can see, we're taking advantage of the fact that the command is a reified call, by *delaying* when the call is performed.

Then we need some code that can take that command and run it on the actor representing the player. Something like:

^code call-actor-command

Assuming `hero` is a reference to the player's character, this will correctly drive him based on the user's input. This simple code has given us a neat little ability for free: we could let the player control any actor in the game now just by changing the object we perform the commands on.

In practice, that's a pretty uncommon feature. But there is a similar use case that's more common. So far, the only thing that's provided commands is the input handler. That's correct for the player's character, but what about all of the other actors in the world?

Those will be driven by the game's AI system. By using the Command pattern, we now have a great interface between the AI engine and the actors: the AI code just chooses and returns `Command` objects.

The decoupling here between the AI code that selects commands, and the actor code that performs them gives us a lot of flexibility. We can use different AI engines for different actors. Or we can mix and match AI for different kinds of behavior. Want a more aggressive opponent? Just plug-in a more aggressive AI to generate commands for it. In fact, we can even bolt AI onto the *player's* character, which can be useful for things like demo mode where the game needs to run on auto-pilot.

By making the commands that control an actor first class objects, we've removed the tight coupling of a direct method call. Instead, you can think of it as a <span name="queue">queue</span> or stream of commands. Some code (the input handler or AI) produces commands and places them in the stream. Other code (the dispatcher or actor itself) consumes commands and invokes them. By having that queue in the middle, the producer on one end and the consumer on the other are decoupled from each other.

<aside name="queue">

Here's another example where this pattern is useful. If we take those commands in the queue and make them *serializable*, we can send them over the network. That means we can take the player's input, push it over the network to another machine, and then replay it.

If the game is deterministic (which it should be!), the only state that you need to transmit to keep the two versions of it running on different machines in sync is the external input from the player, and this command stream captures exactly that for us.
That's one important piece of making a networked multi-player game and this pattern can help.

</aside>

## Attackers and Defenders

Now let's change directions a bit. This is a different use for commands than the typical producer-consumer stream of operations. I haven't seen them used as often for this problem, but I think it's a cool use.

Most uses of the command pattern are for *decoupling* the code selecting or creating the command from the code invoking or performing it. In this example, the goal is to *share* the responsibility for the command across two classes.

Let's say we're implementing the combat system in our game. We have an attacker and a defender. Each side has a bunch of combat rules. The attacker has a weapon, a strength bonus, maybe some spells that temporarily boost their attack power, range, etc. Meanwhile, the defender has their armor, evasiveness, defensive spells, and cover.

To resolve a single attack, all of those rules have to come into play and be resolved. Since the attacker and defender are different objects, it isn't clear which class should actually contain the method to handle a round of combat. If we put it in the attacker, we have to pass all of this defender-specific state in and it has to muck around with it. We have the opposite problem if we lump it all in the defender.

Exacerbating this is the fact that the attacker and defender may even be different *classes*. You can imagine different kinds of combatants having different classes encoding different combat rules. You really wouldn't <span name="every">want every</span> combatant class coupled to details of every other one.

<aside name="every">

If you have *n* classes where each one knows details of every other one, that's *n &times; (n - 1)* connections to maintain. That quadratic number scales quickly out of hand when the number of classes increases.

</aside>

The Command pattern can help here. We'll define a class representing a round of combat itself. Instead of "handle hit" being a *verb*, a single operation, we'll make a "hit" a *noun*, an instantiated class.

    class Hit
    {
      // Stuff...
    };

It will form a bridge between the attacker and

**TODO: aside, similar to mediator pattern**

- here's different motivation for using this pattern
- previous examples were for decoupling when/where/who initiates a command
  from who performs it
- this is example is more about *sharing* responsibility for an operation
  between two objects
- say we're implementing combat
- have attacker and defender
- lots of combat rules on both sides
- attacker has weapon, strength bonus, spells that boost attack power, etc.
- defender has armor, dexterity bonus, spells, resistances, weaknesses, etc.
- to handle a single attack, all of that has to come into play
- have different objects for attacker and defender, isn't obvious who should
  actually contain method to handle hit
- if attacker has it, have to pass all this stuff from defender in. couples
  it to lots of defense details.
- same problem if do it other way
- particularly bad if you have different classes for different kinds of combatants
  in game world
- really don't want some Hero class coupled to details of Monster or Boss class
  or vice versa
- command pattern can help
- define class representing the attack itself
- attacker creates this and populates it with its details
- then give it to defender
- defender finishes filling it out with its details
- then tells the attack object to resolve the hit
- this way, neither attacker nor deferender is coupled to each other
- key idea is have class representing the attack -- the verb -- itself, which
  is exactly what command is about

## Undo and Redo

- last example: undo/redo
- super common in tools like level editors used to make games
- also used in some more strategic games
- say we're making a solitaire game and want to let users undo moves in case
  they back themselves into a corner
- already using command pattern to abstract input handling
- every action that changes game state is already a command: move piles,
  draw card, etc.

  ...sample code...

- to support undo, we just extend command class with another operation: undo
- does exact opposite of what command does
- if command moves card from stack one to two, undo moves it from two to one
- means command needs more state: needs to store what state was in before
  move
- many moves are destructive, so command needs to store that destroyed state
  so it can be restored
- (aside: sometimes memento helps here, but usually too heavyweight. instead
   commands usually just store minimal state required to describe change.)

  ...code...

- to support one level of undo, just remember last performed command. if
  player chooses undo, call its undo method.
- to support multiple undo, need list of commands and pointer to current
  command
- every new command is added to list
- to undo, undo most recent command, then move pointer back one
- to redo, move pointer forward then do that command
- if player does a new command after some commands have been undone,
  every command after that is discarded
- subtle architectural difference with commands in this example and others
  - with previous examples, commands were mostly stateless
  - described abstract "thing that can be done" like jump or move
  - could execute the same command object repeatedly
  - with this example, each command object needs to store chunk of game state
    that existed before command was performed
  - that state is relative to *point in time when that command was performed*
  - means these commands are basically single-use, every time you perform
    command, will be creating new command instance
  - if we are also using command pattern for input handler, can't just store
    a command object in there and keep reusing it
  - instead, input handler will have object that *creates* commands each time
  - in other words, factory

## Classy and Dysfunctional?

- earlier, described commands as similar to closures, but examples here don't
  use any first class functions
- c++ has very limited support for this, functors are weird, pointers to
  members are fiddly, new lambdas in c++11 are tricky with memory
- if writing game in other language, though, feel free to actually use fns for
  commands
- for example, if we were doing this in js, input handler could be:
  ...
- in fact, if using language with first class fns and closures, probably
  already naturally doing this
- in a way, command pattern being so useful just shows how useful the
  functional paradigm is

## See Also

- executing a command often requires some context: see context param
- chain of responsibility is often used with this: give command to one object
  which may then hand it off
- sometimes commands are stateless: just have pure behavior
  - for example, a PauseGame command has no state, just method to pause
  - in this case, can make them flyweight or even <shudder> singleton
