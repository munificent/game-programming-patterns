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

Now let's see how we'd turn this *programmatic* API into something that can be controlled from data. We'll build our way up to the full thing, so let's strip a few things out at first. We'll ditch all of the parameters to these methods. We'll say the `set___()` methods always affect your own wizard and always max out the stat. Likewise, the FX operations always play a single hard-coded sound and particle effect.

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

The interpreter pattern handles things like parameters and subexpressions as a tree of objects, but we want to keep simplicitly and speed of a flat list of instructions. Instead of explicitly wiring up parameter values to the things that consume them, we'll make the data flow implicit. We can do this the same way your CPU does: <span name="stack-machine">with a stack</span>.

<aside name="stack-machine">

This architecture is unimaginatively called a [*stack machine*](http://en.wikipedia.org/wiki/Stack_machine).

</aside>

^code stack

The VM maintains an internal stack of values. In our example, the only kinds of values our instructions work with are <span name="numbers">numbers</span>, so we can just use a simple array of ints. Any bit of data that needs to work its way from one instruction to another gets there by going through the stack.

<aside name="numbers">

A VM for a general-purpose language would need to handle different types of values on the stack: strings, bools, objects, etc. For a dynamically-typed language like most scripting languages, it means you'll use something like a [variant](http://en.wikipedia.org/wiki/Variant_type) or [tagged union](http://en.wikipedia.org/wiki/Tagged_union) type.

</aside>

Like the name implies, values can be pushed onto or popped off of the stack, so let's add a couple of methods for that:

^code push-pop

Now to make our instructions take parameters, they can just pop them off the stack, like so:

^code pop-instructions

If we had a magic stack that summoned integers from the aether, we'd be set, but alas, we'll have to do that ourselves. We've got instructions that *consume* values, but how do we *produce* them? How do things get pushed?

For that, we'll need one more instruction: a literal. It represents a raw integer value. Of course, that just raises the question of where *it* gets its value from? How do we avoid some turtles-all-the-way-down infinite regress here?

To avoid that, we'll take advantage of the fact that our instruction stream is just a sequence of bytes: we'll stuff the number directly in the bytecode. We'll define another instruction type for a number literal. It's interpreted like so:

^code interpret-literal

So we advance our little index into the bytecode and snag the next byte *as a number*. We push that onto the stack and then move onto the next instruction. Here, I'm just reading a single byte for the value to avoid to avoid the fiddly code required to decode a multiple-byte integer, but in a real implementation, you'd want to be able to have literals that cover your full numeric range.

Now we can stitch a few of these instructions together. Here's a little chunk of code where each line represents a single byte:

    INST_LITERAL
    0
    INST_LITERAL
    10
    INST_SET_HEALTH

We'll walk through how the interpret executes it to get a feel for the stack.

1. We execute the first `INST_LITERAL`. That reads the next byte from the bytecode (`0`) and pushes it onto the stack.
2. We execute the second `INST_LITERAL`. That reads the `10` and pushes it. So now our stack, from bottom to top, is `[0, 10]`.
3. Finally, we execute `INST_SET_HEALTH`. It pops `10` and stores it in `amount`, then pops `0` and stores it in `wizard`. Then it calls `setHealth()` with those parameters.

Ta-da! We've got a spell that sets your wizard's health to ten points. Now we've got enough flexibility to define spells that modify either wizard's stats by whatever amounts we want. Also we can play different sounds and spawn particles.

But... this still just feels like a *data* format. We can't, for example, raise a wizard's health by half of their wisdom. Our designers want to be able to express *rules* for spells, not just *values*.

### Behavior = composition

Right now, designers can just tweak knobs on the primitive instructions, but they can't create anything that feels like novel behavior. If we think of our little VM like a programming language, all it supports is constants and a couple of built-in functions.

To get this to feel like *behavior*, what we're missing is *composition*. Our designers need to be able to create expressions that combine different values in interesting ways. For a simple example, they want spells that modify a stat *by* a certain amount instead of *to* a certain amount. In other words, they need to take into account a stat's current value.

We have instructions for *writing* a stat, we just need to add a couple to *read* stats:

^code read-stats

As you can see, it works a bit like both our previous set instructions and the literal one. It pops a parameter from the stack to determine which wizard to get the stat for. Then it looks up the stat's value and pushes that back onto the stack.

This lets us write spells that copy stats around. We could create a spell now that set a wizard's agility to their wisdom, or one that set one wizard's health to mirror his opponents.

Better, but still quite limited. What's missing is basic arithmatic. It's time our baby VM learned how to add 1 + 1. We'll add a few more instructions for the basic arithmetic operations. By now, you've probably got the hang of it and can guess how they look. I'll just do addition:

^code add

Like our other instructions, it pops a couple of values, does a bit of work, and then pushes the result back. Note that the order that we pop things is significant. We pop the *right* operand before the *left*. Pushing a series of values onto a stack and then popping them will reverse their order. If you push `1`, then `2`, then `3`, you'll pop them off as `3`, `2`, and `1`.

To cancel that out, in our instructions we fill in the parameters from right to left. That way, when you look at the stack, the values will be in their "natural" order as you read from bottom to top. Addition is associative -- you get the same result if you swap the operands -- so it doesn't matter here, but it does for subtraction and division.

So far, adding a couple of new instructions has given us an incremental improvement in expressiveness, but we've just made a big leap. It isn't obvious, but we can now handle all sorts of complicated, deeply nested arithmetic expressions.

Let's walk through a slightly more complex example. Say we want a spell that increases your wizard's health by the average of their agility and wisdom. In code, that's:

    setHealth(0, getHealth(0) + (getAgility(0) + getWisdom(0)) / 2)

We haven't added any "instructions" for using parentheses to group expressions, but our data can handle that implicitly. Imagine you were evaluating expression that by hand. What steps would you take? Something like:

1. Get the wizard's current health and remember it.
1. Get the wizard's agility and remember it.
2. Do the same for their wisdom.
3. Get those last two and add them, and remember the result.
4. Divide that by two and remember the result.
4. Recall the wizard's health and add it to that result.
5. Take that result and set the wizard's health to that value.

Do you see all of those "remembers" and "recalls"? Each "remember" corresponds to a push, and the "recalls" are pops. Let's hand translate that to bytecode. To keep things a bit more terse, a raw number here means an `INST_LITERAL` followed by that value.

Next to each instruction, I'll describe its intent and then show what the stack looks like after executing that instruction. For our example, we'll say the wizard's current stats are 45 health, 7 agility, and 11 wisdom.

    0                 [0]            # Wizard index
    0                 [0, 0]         # Wizard index
    INST_GET_HEALTH   [0, 45]        # getHealth()
    0                 [0, 45, 0]     # Wizard index
    INST_GET_AGILITY  [0, 45, 7]     # getAgility()
    0                 [0, 45, 7, 0]  # Wizard index
    INST_GET_WISDOM   [0, 45, 7, 11] # getWisdom()
    INST_ADD          [0, 45, 18]    # getAgility() + getWisom()
    2                 [0, 45, 18, 2] # Divisor
    INST_DIVIDE       [0, 45, 9]     # Average agility and wisdom
    INST_ADD          [0, 54]        # Add average to current health
    INST_SET_HEALTH   []             # Set health to result

If you watch the stack at each step, you can see how data flows through it almost like magic. We push that first zero at the beginning and it just hangs around at the bottom of the stack until we finally need it for the `INST_SET_HEALTH` call at the end.

### A virtual machine

We could keep going, adding more and more instructions, but this is probably a good place to stop. As it is, we've got a nice little VM that lets us define fairly open-ended behavior using a simple, compact data format. While "bytecode" and "virtual machines" sound intimidating, as you can see it's nothing more than a stack, a loop, and a switch statement.

Like we originally wanted, we also have full control over how our little VM executes that code. Obviously, the bytecode can't do anything malicious or reach out into weird parts of the game engine because it can only do things that we've explicitly created instructions for.

We control how much memory it uses by how big of a stack we create, and we're careful to make sure it can't overflow that. We can even control how much *time* it uses. In our instruction loop, we can track how many we've executed and bail out if it goes over some limit.

There's just one problem left: actually creating the bytecode. So far, we've taken bits of pseudocode and compiled them to bytecode by hand. Unless you've got a *lot* of free time, that's not going to work in practice.

### Spellcasting tools

One of our initial goals was to have a *higher*-level way to author behavior, but we've gone and created something *lower*-level than C++. It has the runtime performance and safety we want, but absolutely none of the designer-friendly usability.

To fill that gap, we need some tooling. We need a program that let's users define the high level behavior of a spell and then takes that and generates the apppropriate low-level stack machine bytecode. In other words, a compiler.

I know, that probably sounds like even more work than making the VM. Many programmers were dragged through a compilers class in college and took away from it nothing but PTSD at the sight of a <span name="dragon">book</span> with a dragon on the cover or the words "[lex](http://en.wikipedia.org/wiki/Lex_(software))" and "[yacc](http://en.wikipedia.org/wiki/Yacc)".

<aside name="dragon">

I'm referring, of course, to the classic text [*Compilers: Principles, Techniques, and Tools*](http://en.wikipedia.org/wiki/Compilers:_Principles,_Techniques,_and_Tools).

</aside>

In truth, compiling a text-based language isn't that bad, though it's a bit too broad of a topic to cram in here. However, you don't have to do that. What I said we need was a *tool*, it doesn't have to be one whose input format is text.

On the contrary, I encourage you to consider building a graphical interface to let users define their behavior, especially if the people using it won't be highly technical. Writing text that's free of syntax (not to mention semantic) errors is surprisingly hard for people that haven't had several years of training to get used to a compiler yelling at them.

Instead, you can build an app that lets users build and compose by clicking and dragging little boxes or pulling down menu items, or whatever else makes sense for the kind of behavior you want them to create. The nice thing about this is that your UI can make it impossible for users to create "invalid" programs. Instead of yelling error messages at them, you can just proactively disable buttons or automatically fill in defaults to ensure that the thing they've created is valid at all points in time.

**TODO: Illustrate**

This spares you from designing a grammar and writing a <span name="parser">parser</span> for a little language. But, I know, some of you think UI programming is equally unpleasant. Well, in that case, I don't have any good news for you. If you need the pattern in this chapter then you have some users that need to express themselves in your system. You're going to have to teach them how to speak your system's language, whether that's graphics or text.

<aside name="parser">

Parsers aren't anywhere near as bad as the reputation that precedes them. Just like physics, graphics, or any other programming domain, it's got a couple of typical patterns and techniques. Once you've got those down, you can whip up a parser in a day or two.

</aside>

## Design Decisions

I tried to keep this chapter as simple as I could, but at the heart of this pattern, what we're really doing is defining a language. As you can imagine, that's a pretty open-ended design space. Exploring it is fun, but make sure you're clear about whether you're trying to design a language, or ship a game.

### What's the intruction set?

Your instruction set will define the boundaries of what can and cannot be expressed in bytecode, and will also have a big impact on the performance of your VM. I'll just run through a laundry list of the different kinds of instructions you'll encounter:

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
