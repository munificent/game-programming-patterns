^title Prototype
^section Design Patterns Revisited

Ostensibly, we'll just be talking about the Gang of Four's "Prototype" design pattern here. But "prototype" these days means a good bit more than that pattern, and, as we'll see, the design pattern itself is just about the least interesting use of the term. We'll talk about prototypes as a programming language paradigm, as well as applying it an increasingly important part of games: data modelling.

## Prototype as a Design Pattern

But first, let's go over the <span name="original">original</span> design pattern to see what it is.

<aside name="original">

I don't say "original" lightly here. Design Pattern's cite Ivan Sutherland's legendary Sketchpad project in *1963* as one of the first examples of this pattern in the wild. While everyone else was listening to Dylan and the Beatles, he was busy just, you know, inventing the basic concepts of CAD, interactive graphics, and object-oriented programming.

Watch [the demo](http://www.youtube.com/watch?v=USyoT_Ha_bA) and prepare to be blown away.

</aside>

Lets say we're making a game roughly in the vein of Gauntlet. You've got tons of different kinds of creatures and fiends vying for their share of the hero's flesh. These beasties enter the dungeon by way of generators, and there is a generator for each kind of monster.

For the sake of this example, let's say we have different classes for each kind of monster in the game. We'll have actual C++ classes for `Ghost`, `Demon`, `Sorceror`, etc., like:

^code monster-classes

The generator for each kind of monster needs to construct instances of the different classes. A brute-force solution would be just have a generator class for each monster class, so a parallel class hierarchy like so:

^code generator-classes

Unless you get paid by the line of code, this is pretty obviously not a fun way to solve this problem. Lots of classes, lots of boilerplate.

The prototype pattern offers a solution to this. The key idea is to allow *an object to be a generator of other objects similar to itself*. If you have a ghost, you can make more ghosts from it. If you have a demon, you can make other demons. Any monster can be treated as a *prototypical* monster used to spawn other versions of itself.

Mechanically, the way this pattern works is that your base class -- here `Monster` -- defines a `clone()` method:

^code virtual-clone

Each subclass implements that to return a new object that's identical in class and state to itself. For example:

^code clone-ghost

Once all our monsters support that, we only need a single generator class:

^code generator-clone

The generator internally holds a monster. This isn't a monster that's running around in the level. Instead, it's a hidden monster whose sole purpose is to be used by the generator as a template to stamp out more monsters like itself, sort of like a queen bee who never leaves the hive.

To create a ghost generator, we just create a prototypical ghost instance, and then create a generator using that prototype. One neat part about this pattern is that it doesn't just clone the *class* of the prototype, it clones its *state* too. This means we could make a generator of fast ghosts, or weak ones, or slow ones, just by creating an appropriate prototype ghost.

It's a neat idea. It has a sort of surprising elegance to it, and I can't imagine coming up with it myself. So it's great, right?

## How Useful Is It?

Well, not quite. Sure, we don't have to create a separate generator class for each monster now, but we still have to implement `clone()` in each monster class. That's just about the same amount of boilerplate when you look at it. There are also some really nasty semantic ratholes when you sit down to actually try to write a correct clone. Does it do a deep clone or shallow one? (I.e. when a field is itself a reference to an object, do we clone that object, or just the reference?)

And, of course, to even get this far, we've presumed that we have a bunch of separate classes for each monster. These days, that's definitely *not* the way most game architectures roll. Most of us have learned the hard way that big class hierarchies like this are a pain to manage, which is why we instead use patterns like <a href="component.html" class="pattern">Component</a> and <a href="type-object.html" class="pattern">Type Object</a> to model different kinds of behavior without enshrining each in its own class.

### Generator functions

Even if you do have different classes for each monster, there are other ways to skin this cat. Instead of making separate generator *classes* for each monster, you could make generator *functions*, like so:

^code callback

This is less boilerplate than rolling a whole *class* for constructing a monster of some class. Then the one generator class can just store a function pointer:

^code generator-callback

To create a generator for ghosts, you just do:

^code generate-ghost-callback

### Templates

By <span name="templates">now</span>, most C++ developers are at least a little familiar with templates. Our generator class needs to construct instances of some type, but we don't want to hard code some specific monster type name. The obvious solution then is to make it a type parameter, which is what templates let us do:

<aside name="templates">

It's hard to tell if C++ programmers have learned to love templates, or if templates have just scared some people off C++ completely. Either way, these days everyone I see using C++ uses templates too.

</aside>

<span name="base"></span>

^code templates

Using it looks like:

^code use-templates

<aside name="base">

The pair of classes here -- `Generator` and `GeneratorFor` -- are so that you can have non-templated code that works with a Generator of any type. If we only had the templated `GeneratorFor` class, there'd be no way to write code that accepts a "vanilla" generator of any kind of monster.

`Generator` gives us that single base class that every type-specific generator is derived from. Code that doesn't care what kind of monster a generator emits can just use that and not have to worry about template shenanigans.

</aside>

### First-class types

All of these dance around the basic problem which is that we have a class, `Generator`, that we need to parameterize by a type. Templates are how you do that in C++ because types aren't generally first-class. But if you happen to be using a dynamically-typed language like Smalltalk or Ruby where classes *are* just regular objects you can pass around, then the natural solution is to do that.

When you make a generator, just pass in the class of monster that it should construct -- literally the actual runtime object that represents the monster's class. Easy as pie.

With all of those other options, I honestly can't say I've found a case where I felt the prototype *pattern* was the right answer. So let's put that in our Box of Clever but Not Useful Ideas and talk about something else: prototypes as a *language paradigm*.

## Prototypes as a Language Paradigm

Many people think "object-oriented programming" is synonymous with "classes". While definitions of OOP are subject to endless flamewars, one rough approximation is that it lets you define "objects" which contain both data and behavior. Compared to structured languages like C and functional languages like ML, the defining characteristic of OOP is that it tightly binds state and behavior together.

You may think classes are the one and only way to do that, but a handful of guys in the 80s including Dave Ungar and Randall Smith would beg to differ. They created a language called Self. While as OOP as can be, it had no classes.

### Self

In many ways, Self is *more* object-oriented than a class-based language in some pure philosophical sense. We think of OOP as marrying state and behavior, but class based languages actually have a line of separation there.

Consider the semantics of a class-based language. To access some state on an object, you look in the memory for the instance itself. Each instance has the state. To invoke a method, though, you look up the instance's class, and then you look up the method *there*. State lives in instances, and behavior lives in classes. There's always a level of indirection for the latter.

Self eliminated that distinction. To look up *anything*, you just look on the object. An instance can contain both state and behavior. You can have a single object that has a method completely unique to it.

If that was all Self did, it would be hard to use. Inheritance in class-based languages (despite its faults) enables code reuse and lets you avoid code duplication. To do the same without classes, Self has *delegation*.

To access a field or call a method on some object, we first look in the object itself. If it has it, we're done. If it doesn't, we look at the object's <span name="parent">*parent*</span>. Each object can have a "parent", which is a reference to another object. When we fail to find a property on the object, we try its parent (and its parent, and so on). In other words, failed lookups are *delegated* to an object's parent.

<aside name="parent">

I'm simplifying here. This is a crappy tutorial on Self, but hopefully an OK introduction to prototypes.

</aside>

Parent objects give us a way to reuse behavior (and state!) across multiple objects, so we've got half of what classes cover here. The other key thing classes do is give us a mechanism to create new instances. When you need a new Thingamabob, you can just do `new Thingabob` (or whatever your preferred language's syntax is). The class is implicitly a factory for instances of itself.

Without classes, how do we make new things? In particular, how do we make new things that all have stuff in common? As you can guess by the chapter, the way you do this in Self is by *cloning*.

In Self, it's as if *every* object supports the Prototype design pattern implicitly. Any object can be cloned. To make a bunch of similar objects, you just:

1. Make one object that looks like you want. You can just clone the base `Object` built into the system and then add whatever properties you want.
2. Clone it to make as many... uh... clones as you want.

This gives us the cleverness of the Prototype design pattern without the headache of having to implement `clone()` ourselves. It's just built into the system.

This is such a beautiful, clever, minimal system that as soon as I learned about this, <span name="finch">I started building</span> an interpreter for a prototype-based language just to get more experience with it. A minimal but complete prototype-based language is much simpler than an equally minimal class-based one.

<aside name="finch">

I realize this reaction may not be normal for most people. If you're curious, the language is called [Finch](http://finch.stuffwithstuff.com/).

</aside>

### How Did it Go?

I was super excited to play with a pure prototype-based language, but once I had mine up and running, I discovered an unpleasant fact: it just wasn't that <span name="no-fun">fun</span> to program in.

<aside name="no-fun">

I've since heard through the grapevine that many of the Self programmers came to the same conclusion. The project was far from a loss, though. Self was so dynamic that it needed all sorts of virtual machine innovations in order to run fast enough. The ideas they came up with in Self for just-in-time compilation, garbage collection, and optimizing method dispatch are the exact same techniques (often implemented by the same people!) that now make many of the world's dynamically-typed languages faster than they've ever been.

</aside>

Sure, the language was simple, but that was because it punted the complexity onto the user. As soon as I started trying to use it, the first thing I did was try to come up with a pattern for defining classes.

My hunch is that most people just like well-defined "kinds of things". In addition to the runaway success of classes-based languages, look at how many games have character classes, and a precise roster of different sorts of enemies, items, and skills, each neatly labelled, icon-ified, and lovingly documented. You don't see many games where each monster is a unique snowflake, like "sort of halfway between a troll and a goblin with a bit of snake mixed in".

While prototypes are a really really cool paradigm, and one that I wish more people knew about, I'm glad that most of us aren't actually programming using them every day. The code I've seen that really tries to fully embrace prototypes has a weird shapelessness to it that I find really hard to work with.

### What about JavaScript?

- said most of use are using a prototype-based language but it's not self. it's javascript. eich was inspired directly by self when he created js.

- said prototype languages are simpler and he took full advantage of that fact and created js in just ten days.

- but js isn't a pure prototype-based language. in fact, i'd argue it's closer to class-based.

- there isn't a clone method in site. to define kinds of objects in js, you usually define a "constructor function" that works much like a class. and to create new instances you use "new" just in like C++ or Java.

- there is some prototypal machinery hiding under the hood, but it's often not visible, and, in practice, coding in js often feels more like using a particularly dynamic and flexible but still class-based language.

- there are some who feel js's prototype underpinnings should be brought to the fore and used more, and small amount of js code is in that style.

- others feel js should follow other languages and be more class-like.

- [if want modern language that is fully proto based, see io]

## data pattern

- personally don't feel prototypes are great for a *programming language*, but i do think they're cool for something else: *data*.

- games increasingly data-driven

- big games have tons of data: hundreds of enemies, items, skills, events, levels, etc. data data data

- volume is large enough that really need more than just "uh, you can write xml files".

- many programming language features exist to get rid of duplication.
- for example, can create procedures so that you can call it from multiple
  places, instead of having to copy the code
- when you have enough data, start to want similar features

- prototypes are a great one to consider

- say we're designing data model for game at beginning of chapter
- can define attributes for monsters and items
- each entity is property bag: map of key value pairs

- for example, here's different kinds of goblins in game

    {
      'name': 'goblin grunt',
      'minHealth': 20,
      'maxHealth': 30,
      'resists': ['cold', 'poison'],
      'weaknesses': ['fire', 'light'],
    }

    {
      'name': 'goblin wizard',
      'minHealth': 20,
      'maxHealth': 30,
      'resists': ['cold', 'poison'],
      'weaknesses': ['fire', 'light'],
      'spells': ['fire ball', 'lightning bolt']
    }

    {
      'name': 'goblin archer',
      'minHealth': 20,
      'maxHealth': 30,
      'resists': ['cold', 'poison'],
      'weaknesses': ['fire', 'light'],
      'attacks': ['short bow']
    }

- bunch of duplication there
- would be cool to be able to abstract that in one place
- idea is to let any kind be a prototype for others
- can define new kind as a clone of another
- it defines its own properties that override the cloned ones
- but anything not changed is the same as the prototype
- then just:

    {
      'name': 'goblin grunt',
      'minHealth': 20,
      'maxHealth': 30,
      'resists': ['cold', 'poison'],
      'weaknesses': ['fire', 'light'],
    }

    {
      'name': 'goblin wizard',
      'extends': 'goblin grunt',
      'spells': ['fire ball', 'lightning bolt']
    }

    {
      'name': 'goblin archer',
      'extends': 'goblin grunt',
      'attacks': ['short bow']
    }

- here 'extends' is sort of metaprogramming (metadata)
- says name of kind this one is clone of
- really simple
- really effective
- just like single inheritance has proven a good fit for bunch of languages,
  think this can work well for a lot of data uses

