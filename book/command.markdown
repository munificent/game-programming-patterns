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

## Undo and Redo

The last example is the most well-known use of this pattern. If a command object can *do* things, it's a small step to let it be able to *undo* things. Undo is used in some more strategic games where you can roll back moves that you didn't like. It's, of course, required in tools that people use to *create* games. The <span name="hate">surest way</span> to make your game designers hate you is giving them a level editor that can't undo their fat-fingered mistakes.

<aside name="hate">

I may be speaking from experience here.

</aside>

Without the command pattern, implementing undo is surprisingly hard. With it, it's a piece of cake. For our example, let's say we're making a single player turn-based game and we want to let users be able to undo moves so they can focus more on strategy and less on chance.

We're conveniently already using the command pattern to abstract input handling, so every move the player makes that changes the state of the game is already encapsulated in a command. For example, moving a unit may look like:

^code move-unit

Note this is a little different from our previous commands. In the last example, we wanted to decouple the command from the actor it affected. In this case, we specifically want to *bind* it to the unit being moved.

An instance of this command isn't a general "move something" operation that you could use in a bunch of contexts, it's a specific concrete move in the game's sequence of turns.

This highlights a variation in how the command pattern gets implemented. In some cases, like our first couple of examples, a command is a reusable object that represents a *thing that can be done*. Our earlier input handler held on to a single command object and called its `execute()` method anytime the right button is pressed.

Here, the commands are more specific. They represent a thing that can be done at a specific point in time. This means that the input handling code will be *creating* an instance of this every time the player chooses a move. You can imagine something like:

^code get-move

The fact that commands are one-use-only here will come to our advantage in a second. To make commands undoable, we define another operation each command class needs to support:

^code undo-command

Each implementation of this reverses the game state changed by the corresponding `execute()` method. An undoable unit move looks like:

^code undo-move-unit

Note that we've added some <span name="memento">more state</span> to this class. When we've moved a unit, the unit itself has no recollection of where it used to be. If we want to be able to undo that move, we have to remember its old position ourselves, which is what `xBefore_` and `yBefore_` in the command are.

<aside name="memento">

This "previous state of an object" is an obvious place to use the <a href="http://en.wikipedia.org/wiki/Memento_pattern" class="gof-pattern">Memento pattern</a>. Right before you perform the command, capture a memento of the object you're about to change. To undo, just restore the object from its memento.

If that works for you, great. In practice, I haven't used this. Most commands tend to be pretty minor modifications of a small part of an object's state. For example, here we're just changing a coordinate. Stuffing all of the rest of the object's unchanged state is a waste of time and memory. It's more efficient (if more tedious) to just manually store only the bits of data you change.

Another option is to make your data model use <a href="http://en.wikipedia.org/wiki/Persistent_data_structure">*persistent data structures*</a>. With these, the object is immutable (unchanging), and modifying it actually returns a new object. Through clever implementation, these new objects share much of the same state as the previous ones, so this is much cheaper than cloning the entire object.

Using this, each command can simply store a reference the object before the command was performed, and undo means just switching back to the old object.

</aside>

Now to let the player undo moves, we just keep around the last command they performed. When they bang on Control-Z, we undo that command. (If they've already undone, then it becomes "redo" and we execute the command again.)

Supporting multiple levels of undo isn't much harder. Instead of remembering the last command, we keep around a list of commands and a reference to the current command. Every new command is added to the end of the list and the reference is bumped with it.

When the player chooses undo, we undo the current command and move the pointer back one. When they choose redo, we advance the pointer and then execute that command. If they choose a new command after undoing some, everything in the list after the current command is discarded.

Maybe I'm just easily impressed, but when I first implemented this in a level editor, I was as giddy as a school girl at how straightforward this was and how well it worked. It takes a little discipline to make sure every modification to your state goes through a command, but if you do that, the rest is easy.

## Classy and Dysfunctional?

Earlier, I said commands are similar to first class functions or closures, but every example I showed was using class definitions. If you're familiar with functional programming, wading through all of this boilerplate code has been an <span name="smug">exercise</span> in pain.

<aside name="smug">

Or perhaps just a chance to feel smug at how much better these examples would look in your preferred language.

</aside>

I wrote the examples this way because C++ has pretty limited support for first-class functions. Functors are weird and still require defining a class, function pointers are stateless, and the new lambdas in C++11 are tricky to work with because of manual memory management.

That's *not* to say you shouldn't use functions for the command pattern in other languages. If you have the luxury of a language with real closures, by all means use them! In <span name="some">some</span> ways, the command pattern is really about emulating closures in languages that don't have them.

<aside name="some">

I say "some" here because building actual classes or structures for commands can still be useful even in languages that have closures. If your command has multiple operations (like undoable commands), mapping that to a single function is awkward.

Defining an actual object with structure also helps readers easily tell what state the command is holding onto. Closures are a wonderfully terse way of automatically wrapping up some state, but they can be so automatic that it's hard to see what state they're actually holding.

</aside>

For example, if we were building a game in JavaScript, we could create a move unit command just like this:

    function makeMoveUnitCommand(unit, x, y) {
      // This function here is the command object:
      return function() {
        unit.moveTo(x, y);
      }
    }

If you're comfortable with a functional style, this way of doing things is probably obvious. If you aren't, this chapter may have helped you along the way a bit without you realizing it. For me, the lesson here is that the usefulness of the command pattern is really a sign of how useful the functional paradigm is for many problems.

## See Also

- Command object almost always modify some state. They often store a reference to the state they touch directly, but sometimes there's a larger context that they need access to execute. Think the entire chessboard when the command is for moving a piece.

    The command could store a reference to that too, but that can be a waste
    of space when every command will end up using the same context. A lighter-weight option is to just pass that in when you execute the command. This is what the <a class="pattern" href="context-parameter.html">Context Parameter</a> pattern is about.

- In our examples, we always knew which actor would be handling the command. In some cases, especially where your object model is hierarchical, it may not be so cut and dried. An object may respond to a command, or it may hand it off to some subordinate object. If you do that, you've got yourself the <a class="gof-pattern" href="http://en.wikipedia.org/wiki/Chain-of-responsibility_pattern">Chain of Responsibility</a> pattern.

- Some of the first command examples in this chapter were stateless: the `JumpCommand` in the first example just wrapped a method call. In cases like that, having <span name="singleton">more</span> than one instance of that class wastes memory, since all instances are equivalent. The <a class="gof-pattern" href="flyweight.html">Flyweight</a> can make that cheaper.

<aside name="singleton">

You could make it a <a href="singleton.html" class="gof-pattern">Singleton</a> too, but friends don't let friends create singletons.

</aside>