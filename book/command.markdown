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

    handleInput() {
      if (isPressed(BUTTON_X)) jump();
      if (isPressed(BUTTON_Y)) fireGun();
      if (isPressed(BUTTON_A)) swapWeapon();
      if (isPressed(BUTTON_B)) stumbleAroundIneffectively();
    }

This function usually gets called once per frame by the <a class="pattern" href="game-loop.html">Game Loop</a> and you can figure out what it does. This works if we're willing to hardwire user inputs to game actions. But many games let the use *configure* how their inputs are wired up.

To support that, we need to replace those direct calls to `jump()` and `fireGun()` with something that we can swap out. "Swapping out" sounds a lot like assigning a variable, so we need an object that we can use to represent a game action. Enter the Command pattern.

We define a base class that represents a triggerable game command:

    class Command {
      virtual ~Command();
      virtual void execute();
    };

Then we create subclasses that invoke each of the different game actions:

    class JumpCommand : public Command {
      virtual ~Command();
      virtual void execute() {
        jump();
      }
    };

    class FireCommand : public Command {
      virtual ~Command();
      virtual void execute() {
        fireGun();
      }
    };

    // You get the idea...

In our input handler, we'll have a pointer to a command for each button:

    class InputHandler {
    private:
      Command* xButtonCommand_;
      Command* yButtonCommand_;
      Command* aButtonCommand_;
      Command* bButtonCommand_;
    };

Now the input handling just delegates to those:

<span name="null"></span>

    handleInput() {
      if (isPressed(BUTTON_X)) xButtonCommand_->execute();
      if (isPressed(BUTTON_Y)) yButtonCommand_->execute();
      if (isPressed(BUTTON_A)) aButtonCommand_->execute();
      if (isPressed(BUTTON_B)) bButtonCommand_->execute();
    }

<aside name="null">

Note that we're not checking for `null` here. We're assuming each button will have *some* command wired up to it. If you want to be able to have buttons do nothing, but don't want to have to check for `null` everytime, you can define a command class whose `execute()` method does nothing. Then, instead of setting a button handler to `null`, you point it to that object.

It does the same thing, but doesn't require a check. This is an example of a pattern called [Null Object](http://en.wikipedia.org/wiki/Null_Object_pattern).

</aside>

## A Script for Game Actors

- in previous example commands just did thing, jump, etc.
- the actual in game character, the player's avatar was implicit
- often, better to make that explicit
- have commands for jump and move, etc, but command doesn't know *who* should
  do that
- instead, pass that into command

        class JumpCommand : public Command
        {
        public:
          virtual void execute(GameEntity* entity) {
            entity.jump();
          }
        }

- now can tell any entity in game to jump with one command
- note that jump command is intimitely tied to game entity
- brings up interesting design question for commands
    - usually have some object being commanded
    - have to decide how close command object and commanded object are to each
      other
    - and have to decide where behavior is implemented
    - at one level, command is just wrapper around call to public api in
      commanded class, like we saw
        - commanded class doesn't even know its being commanded
        - all behavior is still in main class
    - other end of spectrum, all behavior is in command itself
    - commands are public api for class

            class JumpCommand : public Command
            {
            public:
              virtual void execute(GameEntity* entity) {
                entity.velocity.y = -JUMP_VELOCITY;
              }
            }

        - usually means commanded class gives special access to commands, think
          friend in c++
        - class itself just maintains state and methods for code reuse
        - ensures all modifcations go through command system
        - can mean spreading behavior across a bunch of classes
    - any point on continuum can work, just see what makes sense for you

- with this, now we have input handler creating commands and then we throw
  them at actors
- gives us interesting new ability: any actor in the game can be controlled
  by player: just throw commands at different actors. could be fun game
  mechanic
- not used widely, but other angle is
- don't have to use input handler to select commands
- instead, ai system does
- ai in game basically becomes chunk of code that selects commands for an
  actor each frame
- by having common command api, can use different ai systems
- can have different ai personalities for different actors
- can swap them out, mix and match, whatever
- one example is using ai to control player character in demo mode
- command stream is basically communication channel that decouples thing
  producing commands from thing obeying them

- networking [aside?]

    - "command stream is communication channel" makes me think of networking
    - indeed that's place where commands can be really helpful
    - when playing online, all machines need to have state updated
    - when you move, that move needs to be sent over network so other players
      see it too
    - when other players move, you need to see it
    - if all input is going through commands, then that stream of data exactly
      captures the information that needs to be sent to other players
    - make those command objects serializable
    - push them down the wire
    - like magic!
      - ok, actually, much harder when you think about latency, ordering, etc.
      - but it's a start

## Attackers and Defenders

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
