^title Bytecode
^section Behavioral Patterns

## Intent

*Define behavior in data as a series of low-level instructions executed by a virtual machine.*

## Motivation

Making games may be fun, but it certainly ain't *easy*. Modern games require <span name="sprawling">enormous</span>, complex codebases. Console manufacturers and app marketplace gatekeepers have stringent quality requirements, and a single crash bug in any of that code can prevent your game from shipping.

<aside name="sprawling">

I worked on a game that had six million lines of C++ code. For comparison, the software controlling the Mars Curiosity rover is less than half that.

</aside>

At the same time, we're expected to squeeze every drop of performance out of the hardware. Games push hardware like nothing else, and we're expected to optimize relentlessly just to keep pace with the competition.

To handle these high stability and performance requirements, we reach for big, complex languages like C++ that have both low-level expressiveness to make the most of the hardware, and rich type systems to prevent or at least corral bugs.

We pride ourselves on our ability to do this, but it has its cost. Being a proficient programmer takes years of dedicated training. Once you've levelled yourself up, you get to fight with the sheer scale of your codebase. Build times for large applications can vary somewhere between "go get a coffee" and "go roast your own beans, hand-grind them, pull an espresso, foam some milk and practice your latte art in the froth".

On top of these challenges, games have one more nasty constraint: *fun*. Players demand a play experience that's both novel and yet carefully balanced. That requires constant iteration, but if every tweak requires bugging an engineer to muck around in piles of low-level code and then waiting for a glacial recompile, you've killed your creative flow.

### Spell fight!

Let's say we're working on a magic-based fighting game. A pair of wizards square off and fling enchantments at each other until victor is pronounced. While each spell is relatively simple, there are piles of them and designers are constantly refining them.

We could define them all in code, but that means an engineer has to be involved every time a spell is modified. When a designer wants to tweak a few numbers and get a feel for them, they have to recompile the entire game, reboot it, and get back into a fight.

Now consider that our game is online. Like most games these days, we want to be able to update it after it ships, both to fix bugs and to add new content. If all of these spells are hard-coded, then updating them means patching the actual game executable.

Let's take things a bit farther and say that we also want to support *modding*. We want *users* to be able to create their own spells. If those are in code, that means every modder needs a full compiler toolchain to build the game, and we have to release the sources. Worse, if they have a bug in their spell, it can crash the game on some other player's machine.

### Data &gt; code

It's pretty clear that our game's implementation language isn't the right fit for our needs here. We want to define spells in a way that's safely sandboxed from the core game engine. We want them to be easy to modify and reload, and physically separate from the rest of the game.

I don't know about you, but to me that sounds a lot like *data*. If we could define our behavior in separate data files that the game engine loads and "executes" in some way, we can achieve all of our goals here.

Now we just need to figure out what "execute" means for data. How do you define some bytes in a file that represent behavior? There are a few different ways to do this. I think it will help you get a picture of *this* pattern's strengths and weaknesses if we compare it to another one -- the <a href="http://en.wikipedia.org/wiki/Interpreter_pattern" class="gof-pattern">Interpreter pattern</a>.

### The Interpreter pattern

I could write a whole chapter just on this pattern, but some gang of four guys already covered that for me. Instead, I'll cram the briefest of introductions in here. It starts with a language -- think *programming* language -- that you want to execute. Say, for example, it supports arithmetic expressions like this:

    1 + 2 * (3 + 4)

Then you take each piece of that expression -- each rule in the language's grammar -- and turn it into an *object*. The number literals will be objects:

**TODO: illustrate**

Basically just little boxes around the raw value. The operators will be objects too, and they'll have references to their operands. So, in the above, we're subtracting `4` from `3`, so we'd have this:

**TODO: illustrate**

If you take into account the parentheses and the rules of arithmetic precedence, that expression <span name="magic">magically</span> turns into a little tree of objects like so:

**TODO: illustration**

<aside name="magic">

What magic process is this? It's simple: *parsing*. A parser takes text -- a stream of characters -- and turns it into an *abstract syntax tree*, a tree-shaped graph of objects representing the grammatical structure of the text.

Whip up one of these and you've got yourself half of a compiler.

</aside>

The interpreter pattern isn't about *creating* that tree, it's about *executing* it. The way it works is pretty clever. Each object in the tree is an expression, or a subexpression. In true object-oriented fashion, we'll let expressions evaluate themselves.

First, we define a base interface that all expressions will implement:

^code expression

There we define classes that implement that for each kind of expression in our language's grammar. The simplest one is numbers:

^code number

A literal just evaluates to its value. Addition and multiplication are a bit more complex because they contain subexpressions. Before they can evaluate themselves, they need to recursively evaluate those first. Like so:

<span name="addition"></span>

^code addition

<aside name="addition">

I'm sure you can figure out what the implementation of multiply looks like.

</aside>

Pretty neat right? Just a couple of simple classes and now we can represent and evaluate arbitrarily complex, nested arithmetic expressions. We just need to create the right objects and wire them up correctly.

In our example here, the expressions just evaluated numbers, but we could add expression classes for things like "spawn particles" or "change an entity's health". There's nothing preventing `evaluate()` from reaching out and calling into other code.

<aside name="ruby">

Ruby was implemented like this for something like 15 years. At version 1.9, they switched to the kind of bytecode VM this chapter describes. Look how much time I'm saving you!

</aside>

It's a <span name="ruby">beautiful</span>, simple pattern, but is has some problems. Look up at the illustration. What do you see? Lots of little objects, and lots of arrows between them. Code is represented as a sprawling fractal tree of tiny objects. That has some unpleasant consequences:

* Loading it from disk requires instantiating and wiring up tons of these small objects.

* Those objects and the pointers between them use a lot of memory. On a 32-bit machine that little arithmetic expression up there takes up at least 68 bytes, not including padding. (If you're playing along at home, don't forget to take into account the vtable pointers.)

* Traversing the pointers into subexpressions is murder on your <span name="cache">data cache</span>. Meanwhile, all of those virtual method calls wreak carnage on your instruction cache.

<aside name="cache">

See the chapter on <a href="data-locality.html" class="pattern">Data Locality</a> for more on what the cache is and how it affects your performance.

</aside>

Put those together, and what do they spell? S-L-O-W. There's a reason most programming languages in wide use aren't based on the interpreter pattern. It's just too slow, and uses up too much memory.

### Machine code, virtually

Consider our game engine. When we execute that, the computer doesn't traverse a bunch of C++ grammar tree structures at runtime. Instead, we compile it ahead of time to machine code and the CPU runs that. What's machine code got going for it?

* *It's dense.* A chunk of machine code is a solid, contiguous blob of binary data, and no bit goes to waste.

* *It's linear.* Instructions are packed together and executed one right after another. No jumping around in memory (unless you're doing actual control flow, of course).

* *It's low-level.* Each instruction does one relatively minimal thing, and interesting behavior comes from *composing* instructions.

* *It's fast.* As a consequence of all of these (well, and the fact that it's implemented directly in hardware), machine code runs like the wind.

Of course, we don't want machine code for our spells, or we'd be right back to the original problem. Few people these days are crazy enough to write machine code by hand, so we'd be right back to needing a full compiler toolchain and all of the overhead that comes with it.

Also, if we dynamically load machine code into our game and start executing it, that's a good way to <span name="jit">breach its security</span> and break the game. Is there a middle ground?

What if instead of loading actual machine code and executing it directly, we defined our own *virtual* machine code? We'd then write a little emulator for it in our game. It would be similar to machine code -- dense, linear, relatively low-level -- but would also be handled entirely by our game so we could safely sandbox it.

<aside name="jit">

This is why many game consoles and iOS don't allow programs to execute machine code loaded or generated at runtime.

That's a drag because the fastest programming language interpreters do exactly that. They contain a "just-in-time" compiler, or *JIT*, that translates the language to highly optimized machine code on the fly.

</aside>

We'd call our little emulator a *virtual machine*, and the synthetic binary machine code it runs *bytecode*. It's got the flexibility and ease of use of defining things in data, but better performance than higher-level representations like the interpreter pattern.

This sounds daunting, though. My goal for the rest of this chapter is to show you that if you keep your feature list pared down, it's actually pretty approachable. At the worst, if you end up still not writing your own, you'll have a better understanding of Lua and many other languages which are implemented as bytecode VMs.

## The Pattern

An **instruction set** defines the low-level operations that can be performed. These are encoded as a **sequence of bytes**. A **virtual machine** executes these instructions one at a time, usually using a **stack for intermediate data**. By combining instructions, complex high-level behavior can be defined.

## When to Use It

This is one of the most complex patterns in this book, and not one you'll throw into your game lightly. Use it when you have a lot of behavior you need to define and your game's implementation language isn't a good fit because:

* It's too low-level, making it tedious or error-prone to program in.

* Iterating on it takes too long due to slow compile times or other tooling issues.

* It has too much trust. If you want to ensure the behavior being defined can't break the game, you ened to be able to safely sandbox it from the rest of the codebase.

Of course, that list describes a bunch of your game. Who doesn't want a faster iteration loop or more safety? However, that doesn't come free. Bytecode is slower than native code, so it isn't a good fit for performance-critical parts of your engine.

### The interop boundary

There's one last constraint to consider. This pattern separates the behavior defined in bytecode from the rest of your game's codebase. This barrier is a good thing -- it's how you get quick iteration and sandboxing -- but you have to cross that barrier if your bytecode is going to do anything useful.

We want spells to change the stats of wizards and cause graphics to appear on screen, which means our VM needs hooks into those parts of the game engine. The more parts of the game that your bytecode needs to talk to, the wider this <span name="interop">interop</span> boundary is. If it gets too wide, you'll spend so much time maintaining that you may cancel out any benefit from the pattern.

<aside name="interop">

In addition to "interop", you sometimes hear this called "binding" or a "foreign function interface".

</aside>

This pattern works best when the interface between bytecode and the game is narrow. While spells may be complex, ultimately, they can only do a few things that touch the game itself: just change stats and a few graphical effects.

## Keep in Mind

There's something <span name="seductive">seductive</span> about defining your own language or system-within-a-system. I'll be doing a minimal example here, but in the real world, these things have a tendency to grow. Every time I've seen someone define a little language or a scripting system, they say, "Don't worry, it will be tiny."

<aside name="seductive">

For me, game development is seductive in the same way. In both cases, I'm striving to create a virtual space for others to play and be creative in.

</aside>

Then, inevitably, they add more and more little features until eventually it's a full-fledged <span name="template">language</span>. Except, unlike many other existing languages, it's grown in an ad-hoc organic fashion and has all of the architectural elegance of a shanty town.

<aside name="template">

For example, see every templating language ever.

</aside>

Of course, there's nothing *wrong* with making a full-fledged language. Just make sure you do so deliberately. Otherwise, be very careful to control the scope of what your little bytecode can express. Put a short leash on it before it runs away from you.

### You'll miss your debugger

Programming is hard. We know what we want the machine to do, but we don't always communicate that correctly -- we write bugs. To help find and fix those, we've amassed a pile of tools to understand what our code is doing wrong, and how to right it.

We have debuggers, static analyzers, decompilers, etc. All of those tools are designed to work with some existing language: either machine code or a higher level language.

When we define our own bytecode VM, we leave those tools behind. Sure, you can step through the VM in your debugger, but that tells you what the VM *itself* is doing, and not what the bytecode it's interpreting is up to. It certainly doesn't help you map that bytecode back to the high-level form is was compiled from.

If the behavior you're defining is simple, you can scrape by without too much tooling to help you debug it. But as the scale of your bytecode grows, plan to invest real time into features that help you see what the VM is doing. Those features might not <span name="debugger">ship</span> in your game, but they'll be critical to ensure that you actually *can* ship your game.

<aside name="debugger">

Of course, if you want your game to be moddable, then you *will* ship those features, and they'll be even more important.

</aside>

### You'll need a front-end

Low-level bytecode instructions are great for performance, but a binary bytecode format is *not* what your users are going to author. One reason we're even moving behavior out of code is so that we can express it at a *higher* level. If C++ is too low level, making your users effectively write in <span name="assembly">assembly language</span> -- even one of your own design -- isn't an improvement!

<aside name="assembly">

Challenging that assertion is the venerable game [RoboWar](http://en.wikipedia.org/wiki/RoboWar). In that game, *players* write little programs to control a robot in a language very similar to assembly and the kind of instruction sets we'll be discussing here.

It was my first introduction to assembly-like languages.

</aside>

Much like the Gang of Four's Interpreter pattern, it's assumed that you also have some way to *generate* the bytecode. Usually, users author their behavior in some higher level format and a tool translates that to the bytecode that our VM understands. In other words, a compiler.

I know, that sounds scary. That's why I'm mentioning it here. If you don't have the resources to build an authoring tool, then bytecode probably isn't a good solution. But it may not be as bad as you think. We'll talk more about this later. First, let's write some code!

## Sample Code

After the previous couple of sections, you might be surprised how straightforward the implementation is. First, we need to craft an instruction set for our VM. Before we start thinking about bytecode and stuff, let's just think about it like an API.

### A magical API

Imagine we were going to define spells in straight C++ code. What kind of API would be need that code to be able to call into? What are the basic operations in the game engine that spells are defined in terms of?

Most spells ultimately change one of the stats of a wizard, so we'll start with a couple for that:

^code magic-api

The first parameter identifies which wizard is affected, say `0` for the player's and `1` for their opponent. This way, healing spells can affect your own wizard, while damaging attacks harm your nemesis. These three little methods cover a surprisingly wide variety of magical effects.

If the spells just silently tweaked stats, the game logic would be fine, but playing it would bore players to tears. Let's fix that:

^code magic-api-fx

These don't affect gameplay, but they crank up the intensity of the gameplay *experience*. We could probably add some more here for camera shake, animation, etc., but this is enough to get us started.

### A magical instruction set

Now let's see how we'd turn this *programmatic* API into something that can be controlled from data. We'll build our way up to the full thing, so let's strip a few things out at first. We'll ditch all of the parameters to these methods. We'll say the `set___()` methods always affect your own wizard and always max out the stat. Likewise, the FX operations always play a single hard-coded sound and particle effect.

Given that, a spell is just a sequence of instructions. Each instruction identifies which primitive operation you want to perform. We can list them out and assign a number to each. In other words, an enum:

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

OK, self-congratulation time is over. We have a VM now, but it's not very flexible. The problem is that our instructions are too rigid. We can't define a spell that touches your opponent, or lowers a stat. We can only play one sound!

To get something that starts to have the expressive feel of an actual language, we need to get parameters in here.

### A virtual stack machine

The interpreter pattern handles parameters and subexpressions by explicitly wiring them together as a tree of nested objects, but we want the speed of a flat list of instructions. We'll do that by making the data flow from parameter to call *implicit* the same way your CPU does: <span name="stack-machine">with a stack</span>.

<aside name="stack-machine">

This architecture is unimaginatively called a [*stack machine*](http://en.wikipedia.org/wiki/Stack_machine). Programming languages like [Forth](http://en.wikipedia.org/wiki/Forth_(programming_language)), [PostScript](http://en.wikipedia.org/wiki/PostScript) and [Factor](http://en.wikipedia.org/wiki/Factor_(programming_language)) expose this model directly to the user.

</aside>

^code stack

The VM maintains an internal stack of values. In our example, the only kinds of values our instructions work with are numbers, so we can just use a simple array of ints. Any bit of data that needs to work its way from one instruction to another gets there by going through the stack.

Like the name implies, values can be pushed onto or popped off of the stack, so let's add a couple of methods for that:

^code push-pop

When an instruction needs to receive parameters, it pops them off the stack, like so:

^code pop-instructions

To get some values onto that stack, we need one more instruction: a literal. It represents a raw integer value. Of course, that instruction can't get its value by popping it off the stack! How do we avoid some turtles-all-the-way-down infinite regress here?

The trick is to take advantage of the fact that our instruction stream is just a sequence of bytes: we can stuff the number directly in the bytecode. We define another instruction type for a number literal like so:

^code interpret-literal

<aside name="byte">

Here, I'm just reading a single byte for the value to avoid to avoid the fiddly code required to decode a multiple-byte integer, but in a real implementation, you'd want to be able to have literals that cover your full numeric range.

</aside>

It reads the next <span name="byte">byte</span> in the bytecode stream *as a number*, pushes it onto the stack, and then advances past it. Now we can stitch a few of these instructions together. Here's each line is one instruction in a chunk of bytecode:

    LITERAL 0
    LITERAL 10
    SET_HEALTH

Let's see how the interpreter executes it to get a feel for how the stack works.

1. It executes the first `INST_LITERAL`. That reads the next byte from the bytecode (`0`) and pushes it onto the stack.
2. It executes the second `INST_LITERAL`. That reads the `10` and pushes it. So now our stack, from bottom to top, is `[0, 10]`.
3. Finally, it executes `INST_SET_HEALTH`. That pops `10` and stores it in `amount`, then pops `0` and stores it in `wizard`. Then it calls `setHealth()` with those parameters.

Ta-da! We've got a spell that sets your wizard's health to ten points. Now we've got enough flexibility to define spells that set either wizard's stats to whatever amounts we want. We can also play different sounds and spawn particles.

But... this still just feels like a *data* format. We can't, for example, raise a wizard's health by half of their wisdom. Our designers want to be able to express *rules* for spells, not just *values*.

### Behavior = composition

If we think of our little VM like a programming language, all it supports now is a couple of built-in functions and constant parameters to them. To get bytecode to feel like *behavior*, what we're missing is *composition*.

Our designers need to be able to create expressions that combine different values in interesting ways. For a simple example, they want spells that modify a stat *by* a certain amount instead of *to* a certain amount.

That requires taking into account a stat's current value. We have instructions for *writing* a stat, we just need to add a couple to *read* stats:

^code read-stats

As you can see, it works with the stack in both directions. It pops a parameter to determine which wizard to get the stat for, then it looks up the stat's value and pushes that back onto the stack.

This lets us write spells that copy stats around. We could create a spell that set a wizard's agility to their wisdom, or a strange incantation that set one wizard's health to mirror his opponent's.

Better, but still quite limited. Next we need arithmetic. It's time our baby VM learned how to add 1 + 1. We'll add a few more instructions. By now, you've probably got the hang of it and can guess how they look. I'll just do addition:

^code add

Like our other instructions, it pops a couple of values, does a bit of work, and then pushes the result back. Up until now, every new instruction gave us an incremental improvement in expressiveness, but we just made a big leap. It isn't obvious, but we can now handle all sorts of complicated, deeply nested arithmetic expressions.

Let's walk through a slightly more complex example. Say we want a spell that increases your wizard's health by the average of their agility and wisdom. In code, that's:

^code increase-health

You might think we'd need instructions to handle the explicit grouping that parentheses give you in the expression here, but the stack supports that implicitly. First, here's how you could evaluate this by hand:

1. Get our wizard's current health and remember it.
1. Get our wizard's agility and remember it.
2. Do the same for their wisdom.
3. Get those last two and add them, and remember the result.
4. Divide that by two and remember the result.
4. Recall the wizard's health and add it to that result.
5. Take that result and set the wizard's health to that value.

Do you see all of those "remembers" and "recalls"? Each "remember" corresponds to a push, and the "recalls" are pops. That means we can translate this to bytecode pretty easily. For example, the first line to get the wizard's current health is:

    LITERAL 0
    GET_HEALTH

This bit of bytecode pushes our wizard's health onto the stack. If we mechanically translate each line like that, we end up with a chunk of bytecode that evaluates our original expression. To give you a feel for how the instructions compose, I've done that below.

To show how the stack changes over time, we'll walk through a sample execution where the wizard's current stats are 45 health, 7 agility, and 11 wisdom. Next to each instruction is what the stack looks like after executing it and then a little comment explaining the instruction's purpose.

    LITERAL 0    [0]            # Wizard index
    GET_HEALTH   [45]           # getHealth()
    LITERAL 0    [45, 0]        # Wizard index
    GET_AGILITY  [45, 7]        # getAgility()
    LITERAL 0    [45, 7, 0]     # Wizard index
    GET_WISDOM   [45, 7, 11]    # getWisdom()
    ADD          [45, 18]       # Add agility and wisdom
    LITERAL 2    [45, 18, 2]    # Divisor
    DIVIDE       [45, 9]        # Average agility and wisdom
    ADD          [54]           # Add average to current health
    SET_HEALTH   []             # Set health to result

If you watch the stack at each step, you can see how data flows through it almost like magic. We push the wizard's current health at the beginning and it just hangs around at the bottom of the stack until we finally need it for last addition at the end.

### A virtual machine

I could keep going, adding more and more instructions, but this is a good place to stop. As it is, we've got a nice little VM that lets us define fairly open-ended behavior using a simple, compact data format. While "bytecode" and "virtual machines" sound intimidating, you can see they're often as simple a stack, a loop, and a switch statement.

Remember our original goal to have behavior be nicely sandboxed? Now that you've seen exactly how the VM is implemented, it's obvious that we've accomplished that. The bytecode can't do anything malicious or reach out into weird parts of the game engine because we've only defined a few instructions that touch the rest of the game.

We control how much memory it uses by how big of a stack we create, and we're careful to make sure it can't overflow that. We can even <span name="looping">control how much *time*</span> it uses. In our instruction loop, we can track how many we've executed and bail out if it goes over some limit.

<aside name="looping">

Controlling execution time isn't necessary in our sample because we don't have any instructions for looping. We could limit execution time just by limiting the total size of the bytecode. This also means our bytecode isn't Turing-complete.

</aside>

There's just one problem left: actually creating the bytecode. So far, we've taken bits of pseudocode and compiled them to bytecode by hand. Unless you've got a *lot* of free time, that's not going to work in practice.

### Spellcasting tools

One of our initial goals was to have a *higher*-level way to author behavior, but we've gone and created something *lower*-level than C++. It has the runtime performance and safety we want, but absolutely none of the designer-friendly usability.

To fill that gap, we need some tooling. We need a program that lets users define the high-level behavior of a spell and then takes that and generates the apppropriate low-level stack machine bytecode. In other words, a compiler.

I know, that probably sounds like even more work than making the VM. Many programmers were dragged through a compilers class in college and took away from it nothing but PTSD triggered by the sight of a <span name="dragon">book</span> with a dragon on the cover or the words "[lex](http://en.wikipedia.org/wiki/Lex_(software))" and "[yacc](http://en.wikipedia.org/wiki/Yacc)".

<aside name="dragon">

I'm referring, of course, to the classic text [*Compilers: Principles, Techniques, and Tools*](http://en.wikipedia.org/wiki/Compilers:_Principles,_Techniques,_and_Tools).

</aside>

In truth, compiling a text-based language isn't that bad, though it's a bit too broad of a topic to cram in here. However, you don't have to do that. What I said we need was a *tool*, it doesn't have to be a compiler whose input format is a text file.

On the contrary, I encourage you to consider building a <span name="text">graphical interface</span> to let users define their behavior, especially if the people using it won't be highly technical. Writing text that's free of syntax errors is surprisingly hard for people that haven't had several years of training to get used to a compiler yelling at them.

<aside name="text">

The scripting system I wrote for [Henry Hatsworth and the Puzzling Adventure](http://en.wikipedia.org/wiki/Henry_Hatsworth_in_the_Puzzling_Adventure) worked like this.

</aside>

Instead, you can build an app that lets users build and compose by clicking and dragging little boxes or pulling down menu items, or whatever else makes sense for the kind of behavior you want them to create.

The nice thing about this is that your UI can make it impossible for users to create <span name="errors">"invalid"</span> programs. Instead of yelling error messages at them, you can just proactively disable buttons or automatically fill in defaults to ensure that the thing they've created is valid at all points in time.

<aside name="errors">

I want to stress how important error-handling is. As programmers, we tend to view human error as a shameful personality flaw that we strive to eliminate in ourselves.

To make a system that users enjoy, you have to embrace their humanity, *including their fallibility*. Making mistakes is what people do, and is a fundamental part of the creative process. Handling them gracefully with features like undo helps your users be more creative and create better work.

</aside>

**TODO: Illustrate**

This spares you from designing a grammar and writing a <span name="parser">parser</span> for a little language. But, I know, some of you find UI programming equally unpleasant. Well, in that case, I don't have any good news for you.

<aside name="parser">

Parsers aren't anywhere near as bad as the reputation that precedes them. Just like physics, graphics, or any other programming domain, it's got a couple of typical patterns and techniques. Once you've got those down, you can whip up a parser in a day or two.

</aside>

Ultimately, this pattern is about expressing behavior in a user-friendly, high-level format. You have to design the user experience for that format, and to execute it efficiently, you need to translate it into a lower-level form. It is real work, but if you're up to the challenge, it can pay off in spades.

## Design Decisions

I <span name="failed">tried</span> to keep this chapter as simple as I could, but at the heart of this pattern, what we're really doing is creating a language. As you can imagine, that's a pretty open-ended design space. Exploring this space can be tons of fun, so make sure you don't forget to finish your game.

<aside name="failed">

Given that this is the longest chapter in the book, I probably failed in that task.

</aside>

### What's the intruction set?

Your instruction set defines the boundaries of what can and cannot be expressed in bytecode, and also has a big impact on the performance of your VM. Here's a laundry list of the different kinds of instructions you may want:

---

* **External primitives.** These are the ones that reach out of the VM into the rest of the game engine and do stuff that the user can see. They control what kinds of real behavior can be expressed in bytecode. Without these, your VM can't do anything more than burn CPU cycles.

* **Internal primitives.** These work with values that are already in the VM. Things like the literals and arithmetic operations we implemented. This covers comparison operators and stuff like instructions that juggle the stack around to duplicate or discard items from it.

* **Control flow.** Our example didn't cover these, but when you want behavior that's imperative and conditionally executes instructions or loops and executes instructions more than once, you'll need control flow. In the low-level language of bytecode, they're surprisingly simple: jumps.

    When we were executing the code, we looped over the instructions and stored the current instruction index in a local variable. All a jump instruction does is assign to that variable and dictate where in the bytecode stream we will continue to execute. In other words, it's a `goto`. Your tool can then build higher-level control flow in terms of that.

* **Abstraction.** If your users start defining a *lot* of stuff in data, eventually they'll want to start reusing bits of bytecode instead of having to copy and paste it. You may want something like callable procedures.

    In its simplest form these aren't much more complex than a jump. The only difference is that the VM maintains a second *return* stack. Every time you do a "call" instruction, you jump to another place in the bytecode, but you push the old instruction index onto that stack. When you hit a "return", you pop that index and jump back to it.

### How are the instructions encoded?

Your bytecode is conceptually a list of instructions, some of which (like literals in our example) may have arguments embedded in them or following right after. You'll have to decide how to map that data to raw bits.

Bytecode VMs come in two main flavors: stack-based and register-based. Ours was a stack-based one.

* **With a stack-based VM:**

    In a stack-based VM, instructions that consume and produce values always do so from the top of the stack. This is how our sample VM works. For example: the `INST_ADD` instruction pops two values, adds them, and pushes the result.

    * *Instructions are small.* Since each instruction implicitly finds its arguments on top of the stack, you don't need to encode any data for that. This means each instruction can be pretty small, usually a single byte.

    * *Code generation is simpler.* When you get around to write the compiler or tool that outputs bytecode, you'll find it a simpler task to generate stack-based bytecode. Since each instruction implicitly works from the top of the stack, you just need to output instructions in the right order to pass parameters between them.

    * *You have more instructions.* Each instruction only sees the very top of the stack. This means that generate code for something like `a = b + c`, you'll need separate instructions to move the variables you need to the top of the stack, perform the operation, then move the result back into the proper variable.

* **With a register-based VM:**

    This style of bytecode is growing in popularity since Lua started using it several years ago and reported a performance improvement. The name is a bit confusing because these VMs still have a stack.

    The difference is that the instructions can read their inputs from anywhere in that stack. Instead of "add" always popping the top two items, an "add" instruction in a register-based VM will have two arguments stored with the bytecode that identify which two stack indices contain the values to be added. This means you don't need separate instructions to bring those values up from the bottom of the stack before "add" can get to them.

    * *Instructions are larger.* Of course storing those arguments in the bytecode takes up room, so register-based VMs have larger instructions. For example, an instruction in <span name="lua">Lua</span> -- probably the most well-known register-based VM -- is a full 32-bits. It uses 6 bits for the opcode (i.e. the thing you switch on), and the rest are arguments for things like identifying registers.

    <aside name="lua">

    Lua doesn't specify their bytecode format and it changes from version to version. What I'm describing here is true as of Lua 5.1. For an absolutely amazing deep dive into Lua's internals, read [this](http://luaforge.net/docman/83/98/ANoFrillsIntroToLua51VMInstructions.pdf).

    </aside>

    * *You have fewer instructions.* Since each instruction can do more work, you don't need as many of them. Some say you get a performance improvement too, since you don't have to copy values around in the stack as much.

So which should you do? My recommendation is to stick with a stack-based VM. They're simpler to implement, and much simpler to write generate code for. Register-based VMs have a reputation for being a bit faster, but that depends *deeply* on your actual instruction set and lots of other details of your VM.

### How our values represented?

Our sample VM only works with one kind of value: integers. That makes it easy: the stack is just a stack of ints. A more full-featured VM might support different data types: numbers, strings, objects, lists, etc.

You'll have to decide how those are stored and identified internally.

* **A single datatype:**

    * *It's simple.* You don't have to worry about tagging, conversions, or type-checking.

    * *You can't work with different data types.* The obvious downside. Only supporting a single data type either limits you or forces you to do ugly hacks like treating 0 and 1 as booleans.

* **A tagged variant:**

    This is the common representation for dynamically-typed languages. Every value is two chunks of bits. The first is a type tag -- an enum -- that identifies what data is being stored. The rest of the bits are then interpreted appropriately according to that type.

    For example, if the type is an int, the other bits are the int value. If it's a string, they'll be a pointer to the string data on the heap.

    * *You can determine the type of a value at runtime.* The nice thing about this representation is that values know their type. You can check it at runtime, which is important for dynamic dispatch and ensuring you don't try to perform operations on types that don't support it.

    * *It takes more memory.* Every value has to carry around a few extra bits with it to identify its type. With something as low-level as a VM, a few bits here and there can add up quickly.

* **An untagged union:**

    This uses a union like the previous form, but does *not* have a type tag that goes along with it. It's like a raw union in C or C++. You have a little blob of bits that could represent more than one type, and it's up to you to ensure you don't misinterpret them.

    This is how <span name="untyped">statically-typed</span> languages represent things in memory. Since the type system ensures that you don't misinterpret values at compile time, you don't validate it at runtime.

    <aside name="untyped">

    This is also how *untyped* languages like assembly and Forth store values.
    Those languages leave it to the *user* to make sure they don't write code that misinterprets a value's type. Not for the feint of heart!

    </aside>

    * *It's compact.* You can't get any more efficient than storing just the absolute bits you need for the value itself.

    * *It's fast.* Not having type tags implies you're not spending cycles checking them at runtime either. This is one of the reasons statically-typed languages tend to be faster than dynamic ones.

    * *It's unsafe.* <span name="unsafe">This</span> is the real cost, of course. A bad chunk of bytecode that causes you misinterpret a value and treat a number like a pointer or vice versa can violate the security of your game or crash.

    <aside name="unsafe">

    If you're bytecode was compiled from a statically-typed language, you might think you're safe here because the compiler won't generate unsafe bytecode. That may be true, but remember malicious users may have hand-crafted evil bytecode without going through your compiler.

    That's why, for example, the Java Virtual Machine has to do *bytecode verification* when it loads a program.

    </aside>

### How is the bytecode generated?

I saved the most important question for last. I've walked you through the code to *consume* and *interpret* bytecode, but it's up to you to build something to *produce* it. The typical answer here is to write a compiler, but it's not the only option.

* **If you define a text-based language:**

    * *You have to define a syntax.* Both amateur and professional language designers categorically underestimate how difficult this is to do. Making a grammar that can be parsed by a machine is easy. Making one that can be easily parsed by a human is *hard*.

        Syntax design is user interface design, and that process doesn't get easier when you slap the constraint of being confined to a linear stream of characters onto it.

    * *You have to implement a parser.* In contrast, this part is fairly simple. Despite their reputation, parsers are pretty easy. Either user a parser generator like ANTLR or Bison, or -- like I do -- hand-roll a little recursive descent and you're good to go.

    * *You have to handle syntax errors.* This is one of the most important and most difficult parts of the process. When users make syntax and semantic errors -- which they will, constantly -- it's your job to guide them back onto the path of success. Giving appropriate, helpful feedback isn't easy when often all you know is that your parser is sitting on some random unexpected punctuation.

    * *It will likely turn off non-technical users.* We programmers like text files. Combined with a powerful command-line, we think of them as the LEGO blocks of computing: simple but easily composible in a million ways.

        Most non-programmers don't think of text like that. To them, it feels like filling in tax forms for a robotic auditor that yells at them if they forget a single semicolon.

* **If you define a graphical authoring tool:**

    * *You have to implement a user interface.* Buttons, clicks, drags, stuff like that. Some programmers cringe at the thought of this, but I personally love it. If you go down this route, it's important to treat designing the user interface as a core part of doing your job well, and not just an unpleasant task to be muddled through.

        Every little bit of extra work you do here will make your tool easier and more pleasant to use and that directly leads to better content in your game. If you look behind many of the games you love, you'll often find the secret was powerful, enjoyable authoring tools.

    * *You have fewer error cases.* Because the user will be building content incrementally one step at a time, your application can guide them away from mistakes as soon as they happen.

        With a text-based language, the tool doesn't see *any* of the user's content until they throw an entire file at it. That makes it much harder to prevent and handle errors.

    * *Portability is harder.* The nice thing about text compilers is that text files are universal. A simple compiler just reads in one file and writes one out. Porting that across operating systems is trivial.

        When you're building a UI, you have to choose which framework to use, and many of those are specific to one operating system. There are cross-platform UI toolkits too, but those often sacrifice familiarity ubquity: they work on many platforms by feeling equally weird and non-standard on all of them.

## See Also

* This pattern's close sister is the Gang of Four's <a href="http://en.wikipedia.org/wiki/Interpreter_pattern" class="gof-pattern">Interpreter pattern</a>. Both give you a way to express composable behavior in terms of data.

    In fact, you'll often end up using *both* patterns. The tool you use to generate bytecode will internally have some tree of objects that represent the code. This is exactly what the interpreter pattern gives you.

    In order to compile that to bytecode, you'll recursively walk the tree, just like you do to *interpret* it with the interpreter pattern. The *only* difference is that instead of executing a primitive piece of behavior immediately, you'll generate the bytecode instruction to perform it later.

* The [Lua](http://www.lua.org/) programming language is the most widely-used scripting language in games. It's implemented internally as a very compact register-based bytecode VM.
