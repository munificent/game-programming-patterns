^title Behavioral Patterns

Once you've built your game's stage and decorated it with actors and props, all
that remains is to start the scene. For this, you need behavior -- the script
that tells each entity in your game what do to.

Of course all code is "behavior", and all software is defining behavior, but
what's different about games is often the *breadth* of it that you have to
implement. While your word processor may have a long list of features, it pales
next to the number of inhabitants, items, and quests in your average MMORPG.

The patterns in this chapter help you define and refine a large quantity of
behavior quickly, and maintainably. [Type Objects](type-object.html) create
categories of behavior without the rigidity of defining an actual class. A
[Subclass Sandbox](subclass-sandbox.html) gives you a safe set of primitives to
you can use to define a variety of behaviors. The most advanced option is
[Bytecode](bytecode.html), which moves behavior out of code entirely and into
data.

## The Patterns

* [Bytecode](bytecode.html)
* [Subclass Sandbox](subclass-sandbox.html)
* [Type Object](type-object.html)
