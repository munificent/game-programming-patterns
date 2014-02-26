^title Bytecode
^section Behavioral Patterns

## Intent

*Increase flexibility by defining behavior as an encoded series of instructions in a higher-level virtual machine.*

## Motivation

- game progs have hard job
- games hard to write
- fiendishly complex, large, high perf requirements, high stability requirements
- to cope, have tools designed for those
- languages we write game engines are complex, lots of features, and require low level expression of intent
- good fit for much of engine
- when things like cache coherency matters, need to express things in bytes
- when have million line quickly changing codebase and bug means game doesn't ship, want complex type systems and ways to enforce invariants, modularity etc.

- but comes at cost
- require right mindset and  years of dedicated training to be productive in language
- dev process often slow and painful: game takes long time to compile
- being able to express at low level is important, *having* to is often painful
- when defining high level behavior, drag on productivity to have to worry about low level fiddly details

- dev in core engine language often slow and difficult, and rigid
- necessary evil for core of game that must be rock solid and efficient

- other parts of game have different constraints
- much high level gameplay behavior isn't performance critical
- but does change often as designers explore game space
- as scale of games has grown, less common for single language to be best fit for all behavior in game

### spell fight

- say working on magic combat game
- wizards duel with hundreds of spells
- designers constantly adding new ones to game
- each spell relatively simple
- couple of concrete examples:
- healing potion raises hero health by 50% of max health, but doesn't go over max
- lightning bolt does 50-100 points of damage with chance of double damage
- ...

- could define these in code but
- means engineer has to be involved every time new item or change
- when designer wants to tweak numbers to get feel for them, have to recompile
  entire game
- adding new items after game has shipped means patching actual game executable
- bug in item behavior can take down entire engine
- want to eventually support modding and then really need to sandbox what items
  can do

### behavior in data

- what does have flexibility we want? data
- can add new data files in patch without touching executable
- tweaking data just means reloading file, which can be done while game is still running
- as long as code processing data file is solid, bad data can't take down game

- best of both worlds then is to define behavior in data

- sort of talking about programming language
- but that brings lots of assumptions
- what we end up with may not be like language

- important bit is just that behavior isn't in code, is data
- but to bring data to life, do have to write code that reads data and does stuff
- lots of ways to do this
- gang of four covers one: interpreter

### interpreter pattern

- briefest possible intro
- basic idea is have some kind of language
- parse it to create abstract syntax tree
- **illustration**
- each node in tree is object
- different grammar entities are different classes
- classes implement "interpret" method
- data turns into behavior by calling that method
- for leaf classes like literals, just returns value
- other classes like exprs recursively call interpret on children
- in this way, can compose complex behavior out of small parts
- just like do when writing code

### vm

- interpreter pattern is neat
- easier to debug at runtime, since can see ast in memory
- but slower to execute, slower to load, trickier memory
- lots of little objects have to be created and wired up
- calling tons of tiny virtual interpret methods is slow

- not how language of game engine works
- game doesn't parse c++ and walk giant ast
- instead, have *compiler*
- compiler parses ast then translates it to machine code

- machine code has nice features
- dense
- linear series of instructions, no nesting, just blob of data
- small set of low-level instructions
- fast!

- original problem though is running compiler is slow and don't have access to
- it at runtime
- not sandboxed either: machine code can crash game
- hardware specific
- many consoles don't allow running dynamically loaded or generated machine
  code
- [jit]

- instead of translating to actual machine code, what if defined *virtual*
  machine
- like real cpu, has set of low level instructions
- code is just linear series of instructions
- we write tiny emulator for machine in c++
- can then take blobs of code and execute them in vm
- not quite as fast as machine code, but simpler, higher-level, platform-independent, and sandboxable

- how many real languages work: lua, ruby >1.9, cpython
- but seems daunting!
- goal of this chapter is to show that simple one can be approachable

## The Pattern

- virtual machine defined with custom instruction set
- behavior is defined as linear series of instructions, encoding in some compact binary form
- vm is implemented at application level

## When to Use It

have lots of behavior to define
underlying implementation language is wrong fit for behavior:
- too low level
- too rigid (either dev loop or after ship)
- too unsafe

- behavior needs to be relatively constrained in terms of what it can express
- spells can do lots of things, but set of things is still pretty narrow when
  consider scope of what you can do with entire engine
- every low level primitive behavior (changing a stat, moving entity) etc. has
  to be made available to vm (like ffi)
- if lots of those, binding layer so large may nullify value of abstraction

## Keep in Mind

- complex, heavyweight pattern
- will be showing small, narrow example here to highlight that don't have to
  go whole hog
- but this pattern often make architectural decision of engine
- hear lots of games talk about supporting scripting

### debugging

- defining behavior, i.e. programming is hard
- know what want machine to do, but often don't communicate intent to machine
  correctly
- bugs
- to help fix those, programmers have very mature tools to help them understand
  what code is doing, why, and how to fix it
- debuggers, static analyzers, etc.

- if defining own bytecode, basically creating programming language
- tools go out window
- if scale of behavior is small, can get by without them
- but as amount of behavior defined in this grows, users need more ability to
  see what's going on
- especially important if aiming for non-technical users, which scripting often
  does
- if creating vm, expect to invest time in tooling

### front end

- pattern about defiing behavior as chunk of low level instructions for vm
- users almost never hand-author those bytes
- if c++ is too low level, making users effectively write in assembly isn't
  improvement!

- will have to write tool that lets user define behavior and generates bytecode
  for them
- in other words, a compiler
- don't have to necessarily define behavior in text
- [hatsworth]
- can have ui to drag and drop bits of stuff
- but will need some kind of tool
- error handling very important!

## Sample Code

- building full-featured general purpose language vm is lots of work
- not doing that
- scoping it down
- just need to be able to define spells

### magic api

- first, need to think about instruction set
- instead of thinking about bytecode and vms, just think about api
- imagine designing api for spells
- provides basic operations
- every spell implemented as fn that calls that api

- set of things here can grow over time, but can get lot of mileage out of
  fairly small number, just above covers all sorts of damage, healing, and buff
  spells

- then couple of things to jazz up gameplay experience
- spawn vfx
- play sound
- don't affect gameplay, but important

- to simplify, imagine api didn't even take params
- literally just commands like "refill health" or "do damage" or "play bang"
- spell is just sequence of commands

### magic instruction set

- if wanted to implement that, easy
- just need enum for instructions

    enum

- to breath life into data, just call into right api for each instruction

    switch

- like name "bytecode" implies, code is just series of bytes
- each byte is one instruction
- [some vms have more complex isntructions]
- byte value determines which instru

- can make little vm that interprets bytecode
- takes array of instructions
- runs them

    vm

- dead simple, just loop over switch
- loop counter is "ip"
- there, have little bytecode vm
- super simple

- but not very flexible
- problem is that instructions too rigid
- can't take params
- have instruction for setting health, but only sets to one specific value

### stack machine

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
