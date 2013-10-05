^title Command
^section Design Patterns Revisited

*Sorry, this chapter hasn't been written yet!*

**outline:**

- intro
  - GoF: "Encapsulate a request as an object, thereby letting users parameterize clients with different requests, queue or log requests, and support undoable operations."
  - prose like this why people don't read design patterns
  - opaque, mixes metaphors: "clients" are people, can't parameterize people!
  - my pithy take: "commands are reified method calls"
  - reify means "make real"
    - in context of oop, means "make into a first-class object"
    - first-class means, can store in variable, pass to fn, etc.
    - reification is common theme in sw eng for making something more powerful
      - making something first class means can now use existing language
        features on it
      - reflection = reified type system
      - closures = reified local scope
  - pithy, but maybe not clear
  - can think of oop code as mix of nouns and verbs
  - nouns are objects, things, you create, stuff in variables and pass around
  - verbs are method calls, things you do to objects
  - reified method call means making noun out of verb
    - so commands are like gerunds
  - in other words, its a first class function
    - in particular, command is lot like partially applied fn
    - indeed, closures are a poor man's command pattern
      - command pattern is a poor man's closure!
    - GoF: "Commands are an object-oriented replacement for callbacks."

- examples
  - ok, that was high level abstract and confusing
  - try not to start chapters like that, got carried away
  - let's get concrete
  - find command pattern to be one of most useful patterns in design patterns
    - almost every non-trivial prog i write ends up using it somewhere
  - run through grab bag of useful places for it

- configurable input
  - basic idea of game: user pushes buttons and exciting stuff happens
  - different buttons do different stuff
  - simple code like

        handleInput() {
          if (isPressed(BUTTON_X)) jump();
          if (isPressed(BUTTON_Y)) fireGun();
          if (isPressed(BUTTON_A)) swapWeapon();
          if (isPressed(BUTTON_B)) stumbleAroundIneffectively();
        }

  - but what if want to let users configure button configuration?
  - need to take those hard-coded function calls and replace them with something
    we can swap in and out.
  - need an object represents function call, like a callback
  - command!
  - have base command class
  - subclasses for each input action
  - input handler has field for binding a command to each button
  - handling code becomes

        handleInput() {
          if (isPressed(BUTTON_X)) xButtonCommand_->execute();
          if (isPressed(BUTTON_Y)) yButtonCommand_->execute();
          if (isPressed(BUTTON_A)) aButtonCommand_->execute();
          if (isPressed(BUTTON_B)) bButtonCommand_->execute();
        }

  - (mention do nothing command is null object pattern)
  - now by swapping out fields, can bind different actions to different buttons
  - increasingly important given multiplatform games and mobile devices with
    different input types
  - may actually have different handleInput fns for different devices, mouse
    touch etc.
  - since it just executes commands, rest of game isn't coupled to details of
    input scheme

- a script for game actors
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

- networking
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

- attackers and defenders
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

- undo
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

- so much code
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

- see also
  - executing a command often requires some context: see context param
  - chain of responsibility is often used with this: give command to one object
    which may then hand it off
  - sometimes commands are stateless: just have pure behavior
    - for example, a PauseGame command has no state, just method to pause
    - in this case, can make them flyweight or even <shudder> singleton

**random notes:**

- GoF: "Commands are an object-oriented replacement for callbacks."

- GoF: "Encapsulate a request as an object, thereby letting users parameterize clients with different requests, queue or log requests, and support undoable operations."

http://steveproxna.blogspot.com/2009/12/command-design-pattern.html