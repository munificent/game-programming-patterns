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

Consider the semantics of a class-based language. To access some state on an object, you look in the memory for the instance itself. Each instance has the state. To invoke a <span name="vtable">method</span>, though, you look up the instance's class, and then you look up the method *there*. State lives in instances, and behavior lives in classes. There's always a level of indirection for the latter.

<aside name="vtable">

For example, to invoke a virtual method in C++, you:

1. Find the pointer to the vtable in the object's memory.
2. From there, look up the method in the vtable.

</aside>

Self eliminates that distinction. To look up *anything*, you just look on the object. An instance can contain both state and behavior. You can have a single object that has a method completely unique to it.

If that was all Self did, it would be hard to use. Inheritance in class-based languages (despite its faults) enables code reuse and lets you avoid code duplication. To do the same without classes, Self has *delegation*.

To access a field or call a method on some object, we first look in the object itself. If it has it, we're done. If it doesn't, we look at the object's <span name="parent">*parent*</span>. Each object can have a "parent", which is a reference to another object. When we fail to find a property on the object, we try its parent (and its parent, and so on). In other words, failed lookups are *delegated* to an object's parent.

<aside name="parent">

I'm simplifying here. This is a crappy tutorial on Self, but hopefully an OK introduction to prototypes.

</aside>

Parent objects give us a way to reuse behavior (and state!) across multiple objects, so we've got half of what classes cover here. The other key thing classes do is give us a mechanism to create new instances. When you need a new Thingamabob, you can just do `new Thingabob` (or whatever your preferred language's syntax is). The class is implicitly a factory for instances of itself.

Without classes, how do we make new things? In particular, how do we make a bunch of new things that all have stuff in common? Just like the design pattern, the way you do this in Self is by *cloning*.

In Self, it's as if *every* object supports the Prototype design pattern implicitly. Any object can be cloned. To make a bunch of similar objects, you just:

1. Make one object that looks like you want. You can just clone the base `Object` built into the system and then add whatever properties you want.
2. Clone it to make as many... uh... clones as you want.

This gives us the cleverness of the Prototype design pattern without the headache of having to implement `clone()` ourselves. It's just built into the system.

This is such a beautiful, clever, minimal system that as soon as I learned about this, <span name="finch">I started building</span> an interpreter for a prototype-based language just to get more experience with it. A minimal but complete prototype-based language is much simpler than an equally minimal class-based one.

<aside name="finch">

I realize building one from scratch is not the most efficient way to learn, but what can I say, I'm a bit peculiar. If you're curious, the language is called [Finch](http://finch.stuffwithstuff.com/).

</aside>

### How did it go?

I was super excited to play with a pure prototype-based language, but once I had mine up and running, I <span name="no-fun">discovered</span> an unpleasant fact: it just wasn't that fun to program in.

<aside name="no-fun">

I've since heard through the grapevine that many of the Self programmers came to the same conclusion. The project was far from a loss, though. Self was so dynamic that it needed all sorts of virtual machine innovations in order to run fast enough.

The ideas they came up with in Self for just-in-time compilation, garbage collection, and optimizing method dispatch are the exact same techniques (often implemented by the same people!) that now make many of the world's dynamically-typed languages faster than they've ever been.

</aside>

Sure, the language was simple, but that was because it punted the complexity onto the user. As soon as I started trying to use it, the first thing I did was try to come up with a pattern for defining classes.

My hunch is that most people just like well-defined "kinds of things". In addition to the runaway success of classes-based languages, look at how many games have character classes, and a precise roster of different sorts of enemies, items, and skills, each neatly labelled. We love baseball cards and stamp collecting. You don't see many games where each monster is a unique snowflake, like "sort of halfway between a troll and a goblin with a bit of snake mixed in".

While prototypes are a really really cool paradigm, and one that I wish more people knew about, I'm glad that most of us aren't actually programming using them every day. The code I've seen that really tries to fully embrace prototypes has a weird mushiness to it that I find really hard to work with.

### What about JavaScript?

Readers who know what's what are ready to pounce now. If prototype-based languages are so unfriendly, explain JavaScript: a language with prototypes that's used by millions of people every day. More computers run JavaScript than any other language on Earth.

<span name="ten">Brendan Eich</span>, the creator of JavaScript, took inspiration directly from Self, and many of its internal semantics are prototype-based. Each object can have an arbitrary set of properties, both fields and "methods" (which are really just functions stored as fields). Every object can also have another object (called its "prototype") that it delegates to if a field access fails.

<aside name="ten">

When I said you can implement a prototype-based language more quickly than a class-based one, I meant it: Eich famously got the first version of JavaScript out the door in ten days.

</aside>

But, despite that, I will argue is that using JavaScript in practice is more like using a class-based language than a prototype-based one.

One hint that JavaScript has taken steps away from Self is that the core operation in a prototype-based language, cloning, is nowhere to be seen. There is no method to clone an object (create a new object with the same properties) in JavaScript.

The closest it has is `Object.create` which lets you create a new object that delegates to an existing one. That wasn't added until ECMAScript 5 a few years ago well after JavaScript was established, and it isn't commonly used.

Instead, let me walk you through the typical way you define and create types of objects in JavaScript. You start with a *constructor function*:

    :::javascript
    function Weapon(range, damage) {
      this.range = range;
      this.damage = damage;
    }

This creates a new object and initializes its fields. You invoke it like:

    :::javascript
    var sword = new Weapon(10, 16);

The `new` here creates a new, empty object, then invokes the body of the `Weapon` function with `this` bound to that object. The body adds a bunch of fields to it, then it's implicitly returned. So now we've got a little object with some state.

The `new` also does one other thing for you. When it creates that blank object, it automatically wires it up to a prototype object. You can access that object using `Weapon.prototype`.

**TODO: whoa this needs some illustrations**

To define behavior, you usually add methods to that prototype object. So we could do something like this:

    :::javascript
    Weapon.prototype.attack = function(target) {
      if (distanceTo(target) > this.range) {
        console.log("Out of range!");
      } else {
        target.health -= this.damage;
      }
    }

Since every object returned by `new Weapon()` delegates to `Weapon.prototype`, you can now call `sword.attack()` and it will call that function. So here we've got:

* The way you create objects is by a "new" operation that you invoke on an object that represents the type (the constructor function).

* State is stored on the instance itself.

* Behavior goes through a level of indirection and is stored on a separate object that represents the set of methods shared by all objects of a certain type.

Call me crazy, but that sounds a lot like my description of classes above. Yes, you *can* write prototype-style code in JavaScript (sort of, without cloning), but the syntax and idioms of the language encourage a much more class-based approach.

Personally, I think that's a <span name="good">good thing</span>. Like I've said, I think doubling down on prototypes actually makes code harder to work with, so I like that JavaScript wraps the core semantics in something a little more structured and "classy".

<aside name="good">

Others in the JS community disagree vehemently. There are strong forces pulling JavaScript to evolve both towards and away from classes and it will be interesting to see which way it goes in the next few years.

</aside>

## Prototypes for Data Modeling

OK, I keep talking about things I *don't* like prototypes for, which is making this chapter a real downer. Everybody likes a happy ending, so let's close this out with an area where I *do* think prototypes, or more specifically *delegation*, are a good fit.

If you were to count up all the bytes in a game that are code versus ones that are data, you'd see the ratio of data to code has been increasingly steadily pretty much since day one. Where early games procedurally generated almost everything and had next to no data, today, we typically think of a game codebase as an "engine" that just runs the game itself which is defined entirely in data.

That's great, but just having piles of data doesn't magically solve all of our organizational problems. Programming languages make our jobs easier because they provide lots features that let us get rid of duplication in our code.

Instead of having to copy and paste a chunk of code in a bunch of places, we move it into a function that we can refer to by name and call. Instead of copying a method in a bunch of classes, we can put it in a separate class that those classes inherit from or mixin.

When your game's data reaches a certain size, you really start wanting similar features. Data modelling is a deep subject that I couldn't hope to do justice here, but I do want to throw out one feature for you to consider in your own games: using prototypes and delegation for reusing data.

Lets say we're defining the data model for the shameless Gauntlet rip-off I mentioned earlier. The game designers need to be able to specify the attributes for monsters and items in some kind of file.

One common approach is to use JSON: data entities are basically *maps*, or *property bags* or any of a dozen other terms because there's nothing programmers like more than inventing names for stuff that already exists.

So a goblin in the game might be defined something like:

    :::json
    {
      "name": "goblin grunt",
      "minHealth": 20,
      "maxHealth": 30,
      "resists": ["cold", "poison"],
      "weaknesses": ["fire", "light"],
    }

This is pretty straightforward and even the most text-averse designer can handle that. So you throw in a couple more kinds of monsters:

    :::json
    {
      "name": "goblin wizard",
      "minHealth": 20,
      "maxHealth": 30,
      "resists": ["cold", "poison"],
      "weaknesses": ["fire", "light"],
      "spells": ["fire ball", "lightning bolt"]
    }

    {
      "name": "goblin archer",
      "minHealth": 20,
      "maxHealth": 30,
      "resists": ["cold", "poison"],
      "weaknesses": ["fire", "light"],
      "attacks": ["short bow"]
    }

Now, if this was code, our aesthetic sense would be tingling now. There's a lot of duplication between these entities, and well-trained programmers *hate* redundancy. It wastes space and takes more time to author. More importantly, it's a maintenance headache. If we decide to make all of the goblins in the game stronger, we need to remember to update the health of all three of these.

If this was code, we'd create an abstraction for a "goblin" and reuse that across the three goblin types. But dumb JSON doesn't know anything about that. So let's make it a bit smarter. We'll add a little meta-programming (metadata?) facility to these bags of properties.

If an object has an `'prototype'` field, then that defines the name of some other object that this one is derived from. We'll copy over any fields from that prototype object into the new one. In other words, the new object is a clone of that one, then with modifications applied.

With that, we can simplify our goblin horde to:

    :::json
    {
      "name": "goblin grunt",
      "minHealth": 20,
      "maxHealth": 30,
      "resists": ["cold", "poison"],
      "weaknesses": ["fire", "light"],
    }

    {
      "name": "goblin wizard",
      "prototype": "goblin grunt",
      "spells": ["fire ball", "lightning bolt"]
    }

    {
      "name": "goblin archer",
      "prototype": "goblin grunt",
      "attacks": ["short bow"]
    }

Since the archer and wizard have the grunt as their prototype, we don't have to repeat the health, resists and weaknesses in each of them. The semantics we've added to our data model is super simple, just basic delegation, but we've already gotten rid of a bunch of duplication.

One interesting thing to note here is that we didn't set up a separate "base goblin" abstract prototype for the three concrete goblin types to delegate to. Instead, we just picked one of the goblins who was the simplest and delegate to that.

That feels natural in a prototype-based system where any object can be used as a clone to create new refined objects, and I think it's equally natural here too. It's a particularly good fit for data in games where you often have one-off special entities in the game world.

Think bosses and unique items. These are often refinements of a more common object in the game, and prototypal delegation is a good fit for defining those. The magic Sword of Head-Detaching, which is really just a longsword with some bonuses can be expressed as that directly:

    :::json
    {
      "name": "Sword of Head-Detaching",
      "prototype": "sword",
      "damage-bonus": "20d8"
    }

A little extra power in your game engine's data modelling system can make it easier for designers to add lots of little variations to the armaments and beastiers populating your game world, and that richness is exactly what delights players.
