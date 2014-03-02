^title Bytecode
^section Behavioral Patterns

## Intent

*Increase flexibility by defining behavior as an encoded series of instructions in a higher-level virtual machine.*

## Motivation

Not to moan about it, but game programmers have a pretty rough jobs. Despite what hordes of gamers might believe, games are actually fiendishly hard to program. Modern games have complex, sprawling codebases. A single bug can crash the program and prevent a console manufacturer or marketplace from approving the game.

To cope with that complexity, we rely on big languages like C++ and Java with piles of modularity features and rich type systems to help us prevent or at least isolate bugs.

At the same time, games are expected to squeeze every drop of performance out of the hardware which requires controlling down to the lowest possible level. Games are one of the last bastions of inline assembly in modern software. To make the best of CPU caching, we carefully control how every byte is laid out in memory.

Despite very mature tooling, all of this has a cost. It takes years of dedicated training to program in this way. Once you've levelled yourself up, you get to fight with the sheer scale of your codebase. Build times for large applications can vary somewhere between "go get a coffee" and "go roast your own beans, hand grind them, pull an espresso, foam some milk and practice your latte art in the froth".

On top of all of these challenges, games have one more nasty constraint: *fun*, players expect games to offer a fresh experience that's precised balanced. That requires constant iteration on every feature and stat in the game. One overpowered move can throw off the entire game balance.

If every tweak and change in the game requires bugging an engineer to muck around in piles of low-level code and then wait for a glacial recompile, you can kill your creative flow. That lost productivity adds up, and often the difference between a great game and a failure boils down to who managed to cram in more iterations of tuning.

Some parts of a high performance game engine simply need to be expressed in the kind of language that lets control exactly what the CPU and RAM is doing. The core of you renderer, physics engine, and maybe AI will make up a large fraction of the game's execution time. Increasing performance by a few percent there can mean richer levels and more excitement on screen.

But much of a game's behavior isn't that performance critical. Things like level scripting and the behavior of items won't even show up in your profiler. And those are there exact places where the behavior is changing constantly as the designers hone the game experience.

As codebases have grown, they've reached a point where one language, one way to express behavior, is rarely the best tool for the entire game. If you can trade off a bit of performance for more flexibility, ease of use, or a quicker iteration time, that can ultimately lead to a better game.

### Spell fight!

Let's say we're working on a magic-based fighting game. Pairs of wizars square off and try to best each other with an array of hundreds of spells. Designers are constantly iterating on the spells, adding new ones, tuning, and tweaking them.

Each spell is relatively simple. It usually just modifies a few stats and does some sort of visual effect. A couple of examples:

- Rejuvenate: Raises the wizard's health by 50%.
- Lightning Bolt: Summons a bolt of lightning that strikes the opponent for 50-80 points of damage.
- Stupefy: Lowers opponents wisdom by one.

We could define all of these in code, but that would mean an engineer has to be involved every time a spell is modified. When a designer wants to tweak a few numbers and get a feel for them, they have to recompile the entire game, reboot it, and get back into a fight.

Now consider that our game is online. Like most games these days, we want to be able to update it after it ships, both to fix bugs and to add new content. If all of these spells are hard-coded, then modifying them means patching the actual game executable.

Let's take things a bit farther and say that we also want to support *modding*. We want *users* to be able to play around with adding new spells. If those are in code, that means every modder needs a full compiler toolchain to build the game, and we have to release the sources. Worse, if they have a bug in their spell, it could crash the game on some other player's machine.

### Data &gt; code

It's pretty clear that our engine's implementation language isn't the right fit for our needs here. We want to define spells in a way that's safely sandboxed from the main game executable. We want them to be easy to modify and reload, and physically separate from the rest of the game.

I don't know about you, but to me that sounds a lot like *data*. If we defined our behavior in separate data files that the game engine loaded and "executed" in some way, we can achieve all of our goals here.

Now we just need to figure out what "execute" means for data. How do you define some bytes in a file that represent behavior? There are lots of ways to do this. I think it will help you get a picture of *this* pattern's strengths and weaknesses if we compare it to another one -- the Gang of Four's <a href="http://en.wikipedia.org/wiki/Interpreter_pattern" class="gof-pattern">Interpreter pattern</a>.

### The Interpreter pattern

I could write a chapter just on this pattern, but I'm pretty sure four guys already did that for me. Instead, I'm going to cram the briefest of introductions in here. The basic idea is that you have some kind of language that you want to interpret.

Say, for example, it supports arithmetic expressions like this:

    1 + 2 * (3 + 4)

Then you take each piece of that expression -- each rule in the language's grammar -- and turn it into an *object*. The numbers will be objects:

**TODO: illustrate**

Basically just little boxes around the raw number. The operators will be objects too, and they'll have references to their operands. So, in the above, we're subtracting `4` from `3`, so we'd have this:

**TODO: illustrate**

If you take into account the parentheses and rules of arithmetic precedence, that original expression <span name="magic">magically</span> turns into a little tree of objects like so:

**TODO: illustration**

<aside name="magic">

What magic process is this? It's simple: *parsing*. A parser takes text -- a stream of characters -- and turns it into an *abstract syntax tree*, a tree-shaped graph of objects representing the grammatical structure of the text.

Whip up one of these and you've got yourself half of a compiler.

</aside>

The interpreter pattern isn't about *creating* that tree, it's about *executing* it. The way it works is pretty clever. Each those objects implements a base `Expression` interface:

    expression

There are different classes that implement that for each *kind* of expression in our language's grammar. For our example, that's numbers, addition and multiplication.

Each class is responsible for evaluating itself and returning the result. For numbers, it's trivial:

    number

A number expression's value is just the number. The two infix expressions are *compound* expressions -- they have subexpressions. So, before they can evaluate themselves, they need to recursively evaluate those first. Like so:

    addition

I'm sure you can figure out what the implementation of multiply looks like. Pretty neat right? Just a couple of simple classes and now we can represent and evaluate any possible arithmetic expression. We just need to create the right objects and wire them up correctly.

In our example here, the expressions just evaluated numbers, but you could have "expressions" for things like "spawn particles" or "change an entity's health". There's nothing prevent `evaluate()` from reaching out and calling into other code.

It's a great pattern. If you ever implement your own programming language, you'll probably write one in this style at some point. Ruby worked exactly like this all the way up to <span name="ruby">1.9</span>. But it's got it's problems.

<aside name="ruby">

In 1.9, they switched to a bytecode VM, which is what this chapter is about. It took something like 15 years for them to make the switch. Look how much time I'm saving you!

</aside>

Look up at the illustration. What do you see? Lots of little objects, and lots of arrows between them. Code in this style is represented as a sprawling fractal tree of tiny objects. That's got some problems:

* Loading code from disk requires instantiating and wiring up tons of these small objects.

* All of those objects take up memory. On a 32-bit machine that little arithmetic expression up there takes up at least 68 bytes, not including padding. (If you're playing along at home, don't forget to take into account the vtable pointers.)

* Traversing the pointers into subexpressions is murder on your data cache.

* Meanwhile, calling all of those virtual methods wreaks carnage on your instruction cache.

* Evaluating every single tiny subexpression involves the overhead of a virtual method call.

Add those up, and what do they spell? SLOW. There's a reason more languages in real use do *not* use the interpreter pattern. It's just too slow, and uses up too much memory.

### Machine code, virtually

Consider our game's executable. The computer doesn't run it by walking around these grammatical tree structures in memory. Instead, it executes machine code. What's machine code got going for it?

* It's dense. A chunk of machine code is a solid, contiguous blob of binary data, and no bit goes to waste.

* It's linear. Instructions are packed together and executed one right after another. No jumping around in memory (unless you're doing actual control flow, of course).

* It's low-level. Each instruction does one relatively minimal thing, and interesting behavior comes from *composing* instructions.

* It's fast. As a consequence of all of these (well, and the fact that it's implemented directly in hardware), machine code runs like the wind.

Of course, we don't want machine code for our spells, or we'd be right back to the original problem. Few these days are crazy enough to write machine code by hand, so we'd be right back to need a full compiler toolchain and all of the problems that come with it.

Also, if we were to dynamically load machine code into our game and start executing it, that's a good way to make it completely insecure and likely to crash or steal the player's credit card number. For this reason, many game consoles and iOS don't allow programs to execute machine code loaded or generated at runtime.

Is there a middle ground? What if instead of loading actual machine code and executing it directly, we defined our own *virtual* machine code? We'd then write a little emulator for it in our game. It would be similar to machine code -- dense, linear, relatively low-level -- but would also be handled entirely by our game so we can sandbox it and control it.

We'd call our little emulator a *virtual machine*, and the synthetic binary machine code it runs *bytecode*. It's got the flexibility and ease of use of defining things in data, but better performance than higher-level representations like the interpreter pattern.

This sounds pretty daunting, though. My goal for the rest of this chapter is to show you that if you can keep your feature list pared down, it's actually pretty approachable. At the worst, if you end up still not writing your own, you'll have a better appreciation for Lua and many other languages which are implemented as bytecode VMs.

## The Pattern

A **set of instructions** defines the low-level operations that are can performed. These are encoded as an efficient **binary sequence of instructions**. A **virtual machine** executes these instructions one at a time. By combining instructions, complex high level behavior can be defined.

## When to Use It

Even pared down, this is still a pretty big, complex pattern. It's not one you'll throw into your game lightly. At the very least, you'll have a lot of behavior that you need to implement. It would be pretty silly to use this in our example game if there were only a handful of spells.

In addition, the underlying language your game is implemented isn't a good fit:

* It's too low-level, making it tedious or error-prone to program in.

* Iterating on it takes too long due to slow compile times or other tooling issues.

* It has too much trust. If you want to ensure the behavior being defined can't break the game, you ened to be able to safely sandbox it from the rest of the codebase.

The above list describes many parts of your game. Aside from the performance-critical core of the engine, most of the code you're writing is pretty high level. And who doesn't want a faster iteration loop or more safety? This helps explain why many games today do use an embedded scripting language like Lua to define much of the high level logic. But there's one other soft requirement to keep in mind.

When you use <span name="other">this pattern</span>, there is an implicit separation between your game's codebase and the universe of stuff that's defined in bytecode. This separation is a good thing: that's how you get quick iteration, live reloading and sandboxing. But the behavior you're defining in bytecode doesn't exist in a vaccum. Ultimately, for it to do anything useful, it needs to communicate with your game.

<aside name="other">

This is equally true of using a scripting language, or the interpreter pattern, or any other scheme where you aren't defining your entire game in a single language or framework.

</aside>

Our spells are going to need to be able to tweak the stats of wizards and cause graphics to appear on screen. That means we'll have to explicitly provide hooks so that the bytecode can touch those parts of the game. This layer where two languages or systems communicate is called "interop", or "binding", or a "foriegn function interface".

Whatever you call it, you still have to *do* it. The more parts of the game that your bytecode needs to talk to, the wider that interop boundary will be. If it gets too wide, you'll spend so much time adding bindings that it may cancel out the benefits of the pattern itself.

This pattern works best when interface between bytecode and the game is pretty narrow. In our example, spells can really only do a few primitive things that touch the game itself: just change stats and a few graphical effects.

## Keep in Mind

Like I said, this is one of the most complex, heavyweight patterns in this book. I'll be doing a minimal example here to show that you don't have to go whole hog on it. Even so, these things have a tendency to grow. Almost every time I've seen someone define a little templating language or a minimal scripting system, they say, "Don't worry, it will be tiny."

Then, inevitably, they keep adding more and more little features until eventually it's Turing-complete and is pretty much a full-fledged language. Except, unlike many other existing languages, it's grown in an ad-hoc organic fashion and is riddled with cruft.

Of course, there's nothing wrong with being a full-fledged language. Just make sure you get there deliberately. Otherwise, be very careful to control the scope of what your little bytecode can express. Keep a short leash on it so it doesn't run away from you.

### Bytecode is opaque

Defining behavior in a way a machine can understand -- programming -- is hard. We know what we want the machine to do, but we don't always communicate our intent correctly. We write bugs.

To help find and fix those, we've amassed a pile of very mature tools to help us understand what our code is doing, why, and how to fix it. We have debuggers, static analyzers, decompilers, etc. All of those tools are designed to work with an existing language: either machine code or some higher level language.

If we define our own little bytecode VM, we've left those tools behind. Your IDE's debugger will let you step through your bytecode interpreter, but it doesn't understand the bytecode *itself*. Finding bugs there feels like fumbling around in a pitch black room. This is true both for the behavior defined in the bytecode and in your implementation of the VM itself.

If your VM is simple and the behavior you're defining in bytecode is pretty small like our little spells, you can probably get by without too much tooling to help you debug. But as the scale of your VM and your bytecode grows, plan to invest some real time in features to help you see into what the VM is doing. Those features might not ship in your game, but they'll be critical to ensure that you actually *can* ship your game.

### You'll need a front-end

This pattern is about defining behavior as a chunk of low level binary instructions. This is great for runtime efficiency, but a binary bytecode format is *not* what your users are going to author. One of the reason we define behavior in data is to be able to express things at a *higher* level. If C++ is too low level, making your users effectively write in <span name="assembly">assembly language</span> -- even one of your own design -- isn't an improvement!

<aside name="assembly">

Challenging that assertion is the venerable game [RoboWar](http://en.wikipedia.org/wiki/RoboWar). In that game, *players* write little programs to control a robot in a language very similar to assembly and the kind of instruction sets we'll be discussing here.

It was my first introduction to assembly-like languages.

</aside>

Much like the Gang of Four's Interpreter pattern, it's implied that you'll also be build a tool to *generate* the bytecode. Users will author their behavior in some higher level format and the tool will translate that to the bytecode that our VM understands. In other words, a compiler.

I know, that sounds scary. It's a bit of a raw deal to say, "Here's this awesome little pattern that lets you define behavior in data! The sample code is just a hundred lines of code! (Oh, but you'll have to write a compiler. Details not specified here.)"

<span name="text">However</span>, you don't necessarily have to write a compiler for a *text-based language*. In fact, if your users are not programmers, I'd encourage you to *not* use text as your input. Instead, you can write a tool with a graphical user interface for creating "scripts". Users can drag and drop bits of stuff to assembly complex behavior.

**TODO: illustrate**

<aside name="text">

The scripting system I wrote for Henry Hatsworh and the Puzzling Adventure worked exactly like this. It showed a tree-based UI for each actor's behavior. Users clicked around to add new actions to the tree and composed behavior that way.

</aside>

Of course, many programmers are as scared of UI programming as they are compilers. If that's your situation, then I can't do much to help you. Ultimately, this pattern is about letting users express themself in a higher-level format and then executing what they're expressed quickly, which entails a lower-level form.

To get from A to B means create a <span name="errors">user-friendly</span> high-level format, and the tooling to lower into the form our VM can handle. It is real work, but if you're up to the challenge, it can pay off in spades.

<aside name="errors">

One corner of "user-friendliness" bears special mention: handling errors. To err is human. As programmers, we tend to view human error as a shameful failure that we want to brush under the rug.

To make a system that users enjoy, you have to embrace their humanity, *including their fallibility*. Gracefully handling errors, through undo, error messages, etc. is a key part of making your tool usable.

</aside>

## Sample Code

OK, if I haven't scared you off yet, let's dig into some code. After the previous couple of sections, you might be surprised how straightforward it is. Now, if we were creating a full-featured general purpose language VM, the rest of this chapter would be <span name="book">a book</span> in itself. Fortunately, we don't need to do that. We just need a little VM for defining spells, so we can scope things way down.

<aside name="book">

A book I would love to write, actually.

</aside>

### A magical API

First, we need to craft and instruction set for our VM. Before we start thinking about bytecode and stuff, let's just think about it like an API. Imagine we were going to define spells in straight C++ code. What kind of API would be need that code to be able to call into? What are the basic operations in the game engine that spells are defined in terms of?

Most spells ultimately change one of the stats of a wizard, so we'll start with a couple for that:

    ^code magic-api

The first parameter identifies which wizard is affected, say `0` for the player's and `1` for their opponent. This way, healing spells can affect your own wizard, while damaging attacks harm your nemesis. Already, this covers a wide swath of spell behavior.

If the spells just silently tweaked stats, the gameplay would be fine, but the game would bore player's to tears. Let's fix that:

    ^code magic-api-fx

These don't affect gameplay, but they crank up the intensity of the gameplay *experience*. We could probably add some more here for camera shake, animation, etc., but this is enough to get us started.

### A magical instruction set

Now let's see how we'd turn this *programmatic* API into something that can be controlled from data. We'll build our way up to the full thing, so let's strip a few things out at first. We'll ditch all of the parameters to these methods. We'll say the `change___()` methods always change your own wizard and always add one to the stat. Likewise, the FX operations always play a single hard-coded sound and particle effect.

If our API looked like that, then a spell would just be a sequence of instructions. Each instruction would just identify which of the primitive operations you wanted to perform. We could list them out and assign a number to each. In other words, an enum:

    ^code instruction-enum

To encode a spell in data, we just store an array of enum values. We've only got a few different primitives, so the range of enum values easily fits into a byte. This means a spell is just a list of <span name="byte">bytes</span> -- ergo "bytecode".

<aside name="byte">

Some bytecode VMs use more than a single byte for each instruction and have more complicated rules for how they are decoded. Actual machine code on common chips like x86 is a good bit more complex.

But a single byte is good enough for the [Java Virtual Machine](http://en.wikipedia.org/wiki/Java_virtual_machine) and Microsoft's [Common Language Runtime](http://en.wikipedia.org/wiki/Common_Language_Runtime) which forms the backbone of the .NET platform, and it's good enough for us.

</aside>

To execute a single instruction, we just see which primitive it is and call the right API method:

    ^code interpret-instruction

We can call into the game engine because our interpreter is defined in the language of the game itself. It forms the bridge between code world and data world. A spell is then a sequence of these instructions. We can create a little VM that executes an entire spell like so:

    ^code vm

It wraps the instruction dispatch in a loop and stops when it reaches the end. There, type that in and you'll have written your first virtual machine. Pat yourself on the back!

OK, self-congratulation time is over now. We have a VM now, but it's not very flexible. The problem is that our instructions are too rigid. We can't define a spell that touches your opponent, or lowers a stat. We can only play one sound!

To get something that starts to have the expressive feel of an actual language, we need to get parameters in here.

### A virtual stack machine

**todo: link to stack machine**

- want to have params for ins, be able to compose expressions
- like interpreter
- how do that without complexity of interpreter?
- want to keep flat list of ins

- do it just like cpu does: with stack

    stack

- stack is just array of values
- in our example, only data vm works with is numbers, so array of nums
- every instruction implicitly modifies that stack

- all data flow is implicit, using single stack shared by all instructions
- [some langs work this way]

- each ins can push stuff onto stack or pop off
- for example, set health ins needs to know what to set health too
- gets it by popping off top of stack

    set health

- how does value get there?
- need some inst for creating and working with nums
- first is literal: represents raw num
- literal ins pushes value onto stack
- where it get value?
- right from bytecode itself: stuff value as raw data right in bytecode after
  ins
- to interp, read value out of bytecode and push

    literal

- so if wanted to set health to 20, do:

    push 20
    set health

- if run this, load literal 20 onto stack then exec set health which consumes

- can do this for other ins that need params, can even pop multiple values
- (other ins...)

- much more expressive now
- can create lots of spells that have basically different tuned constants

### behavior = composition

- still doesn't feel like behavior
- really just tweaking knobs
- want to be able to *compose* stuff

- add more ins
- want spells that modify stats
- add 10 to health
- need ins to read stat

    read

- opposite of set: pulls attribute from game pushes onto stack
- can now do stuff like read one stat and store it as other
- could make weird things like spell that makes as strong as smart

- can still only copy values from one stat to other
- want to do arith
- add some ins
- plus pops two, adds, and pushes result

    plus

- ditto for other ops

- spells have chance, let's make random ins
- pops min and max from stack, and pushes rand num in range
- now can make spells that have varied effect

- with these, can now do really rich behavior
- spell that increases health by 1/5 of stamina
- spell that plays random sound

- not obvious, but can even eval complex, nested exprs

- consider (10 / 3) - (2 * 3))
- start with ten, but put off to side for now
- because of parens, need to mult first
- then take 2 and 3 and multiply
- then get ten and result of mult and subtract
- that's result

- translate to bytecode

    ...

- almost exact same thing
- note data flow implicit and right values get to right instructions from stack

### vm

- ta-da, now have tiny little vm that supports all kinds of open-ended behavior
  for spells
- seems really simple, but thanks to arithmetic and ability to compose
  expressions using stack, sky is limit

- note that we have full control over how execute code
- bytecode can't poke into disallowed places in game engine because only has
  access to instructions we chose to implement
- can control how much memory it uses (just stack) and handle it overflowing
- can even control code that runs for too long by stopping if execute too many
  instructions

- just one problem... hand writing instructions even harder than writing c++!

### spellcasting tools

- being able to look at chunk of code and hand-compile it to stack machine
  bytecode is perhaps world's lamest party trick
- not something most people want to do

- one of goals was *higher* level way to author behavior, not lower
- need tool to let users express spell semantics in something friendlier
- then automatically translate to low level stack machine bytecode
- compiler

- but! doesn't have to be textual
- can be, but many non-tech people not text oriented
- prog langs unforgiving of syntax mistakes, really unpleasant for many users
- instead, can build gui for creating behavior

**illustrate**

- nice thing is ui prevents user from creating invalid code
- don't have to write parser
- [writing parser not that hard. difficulty overstated. good error messages
  are work though, and critical.]
- do still have to design and implement tool ui
- no escaping: you are creating system to let user define behavior
- have to make user experience
- text prog lang is ui too

## Design Decisions

- while keep chapter simple, really about defining lang
- very open ended
- remember, trying to ship game, not design world's greatest lang

### instructions?

- have to decide what prim instructions
- have set of them that touch actual game engine in visible way: modify entities, graphics, etc. "side effects"
- these define limits of what code can and cannot do
- if don't provide prim for moving camera, no amount of clever bytecode can fake it

- usually have basic arithmetic ones
- couple stack juggling ones
- may add flow control ins. didn't cover here, but these pop value from stack
  and, based on it, move the index used to walk through bytecode
  (goto)
- if then and even ?: and short-cirtuit && || use this

- if users want to reuse chunks of bytecode, need something like function call
  instr
- basically instr that jumps to other point in bytecode
- have separate stack of return addresses
- when done with subroutine hit "end" instr
- pop return address off stack and set ip to that

### instruction encoding

- have to decide how inst are encoded in binary
- in example, just one byte each, simple enum mapping
- good enough for jvm
- other vm more complex
- some "register-based"
- bit if misnomer, not like regs in cpu
- instead of always getting args from top of stack, inst encodes offsets into
  stack
- can try to pack more inst into fewer bytes
- perf game: denser code is faster [data locality]
- but takes more time to decode
- profile and tweak

### value rep

- what kind of runtime values vm handle?
- in example, just simple numbers
- may need multiple types: numbers, strings, etc.
- have to decide how represented and stored in stack
- typically have something like union or variant type

### how bytecode generated

- by far most important question
- what kind of tool (or tools!) have to create bytecode

- **text based language**

    - tried and true
    - don't have to write editor: use text editor
    - portable: not bound to ui system
    - have to write parser: easy
    - have to design syntax: hard!
    - need good error messages
    - non-progs not comfortable with text

- **ui**

    - when done well, fun to use
    - less error-handling since ui can prevent malformed code
    - avoid syntax rathole
    - have to do lots of ui programming

## See Also

- sister pattern is interpreter. often go hand-in-hand with this. when write
  tool to generate bytecode, tool itself often parses or uses ui to build tree
  then "interpret" pattern is used to emit bytecode

- similar to command pattern: can think of as dense encoding of series of
  commands

- lua most well known bytecode vm in games

- jvm and clr well-known bytecode vms

- genre of "programming games" where player writes code. some early ones simple bytecode based like robowar
