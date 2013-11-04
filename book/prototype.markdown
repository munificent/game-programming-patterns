^title Prototype
^section Design Patterns Revisited

Given that this chapter lives under "Design Patterns Revisited", you wouldn't be foolish for assuming this chapter will be all about the <a href="http://en.wikipedia.org/wiki/Prototype_pattern" class="gof-pattern">Prototype design pattern</a>. You would be mistaken, though. Sure, I'll go over that too, but I think prototypes show up in some much more interesting corners of programming these days.

## The Prototype Design Pattern

But before we wander abroad, let's cover the <span name="original">original</span> pattern.

<aside name="original">

I don't say "original" lightly here. *Design Patterns* cites Ivan Sutherland's legendary [Sketchpad](http://en.wikipedia.org/wiki/Sketchpad) project in *1963* as one of the first examples of this pattern in the wild. While everyone else was listening to Dylan and the Beatles, he was busy just, you know, inventing the basic concepts of CAD, interactive graphics, and object-oriented programming.

Watch [the demo](http://www.youtube.com/watch?v=USyoT_Ha_bA) and prepare to be blown away.

</aside>

Pretend we're making a game roughly in the style of Gauntlet. We've got different creatures and fiends swarming around the hero, vying for their share of his flesh. These unsavory dinner companions enter the dungeon by way of generators, and there is a different generator for each kind of monster.

For the sake of this example, let's say we have different classes for each kind of monster in the game. We'll have actual C++ classes for `Ghost`, `Demon`, `Sorcerer`, etc., like:

^code monster-classes

A generator constructs instances of some monster class. To support every monster in the game, a brute-force solution would be have a generator class for each monster class, leading to a parallel class hierarchy:

<img src="images/prototype-hierarchies.png" />

Coding that up looks like this:

^code generator-classes

Unless you get paid by the line, this is pretty obviously not a fun way to solve this problem. Lots of classes, lots of boilerplate, lots of redundancy, lots of duplication, lots of repeating myself...

The prototype pattern offers a solution. The key idea is that *an object can be a generator of other objects similar to itself*. If you have a ghost, you can make more ghosts from it. If you have a demon, you can make other demons. Any monster can be treated as a *prototypical* monster used to spawn other versions of itself.

To implement this, we give our base class, `Monster`, an abstract `clone()` method:

^code virtual-clone

Each monster subclass provides an implementation that returns a new object identical in class and state to itself. For example:

^code clone-ghost

Once all our monsters support that, we no longer need generator classes for each kind of monster. Instead, we define a single generator class:

^code generator-clone

It internally holds a monster, a hidden one whose sole purpose is to be used by the generator as a template to stamp out more monsters like itself, sort of like a queen bee who never leaves the hive.

<img src="images/prototype-generator.png" />

To create a ghost generator, we just create a prototypical ghost instance, and then create a generator using that prototype:

^code generate-ghost-clone

One neat part about this pattern is that it doesn't just clone the *class* of the prototype, it clones its *state* too. This means we could make a generator of fast ghosts, or weak ones, or slow ones, just by creating an appropriate prototype ghost.

I find something both elegant and yet surprising about this pattern. I can't imagine coming up with it myself, but it's such a simple and expressive concept.

### How well does it work?

Well, we don't have to create a separate generator class for each monster, so that's good. But we *do* have to implement `clone()` in each monster class. That's almost as much code as the separate classes.

There are also some nasty semantic ratholes when you sit down to try to write a correct `clone()`. Does it do a deep clone or shallow one? (I.e. when a field is itself a reference to an object, do we clone that object, or just the reference?) How does it interact with ownership?

Also, not only does it not look like it's saving us much code in this contrived problem, there's the fact that it's a *contrived problem*. We had to take as a given that we have separate classes for each monster. These days, that's definitely *not* the way most game engines roll.

Most of us learned the hard way that big class hierarchies like this are a pain to manage, which is why we instead use patterns like <a href="component.html" class="pattern">Component</a> and <a href="type-object.html" class="pattern">Type Object</a> to model different kinds of entities without enshrining each in its own class.

### Generator functions

Even if we do have different classes for each monster, there are other ways to decorticate this feline. Instead of making separate generator *classes* for each monster, we could make generator *functions*, like so:

^code callback

This is less boilerplate than rolling a whole *class* for constructing a monster of some type. Then the one generator class can just store a function pointer:

^code generator-callback

To create a generator for ghosts, you just do:

^code generate-ghost-callback

### Templates

By <span name="templates">now</span>, most C++ developers are familiar with templates. Our generator class needs to construct instances of some type, but we don't want to hard code some specific monster type name. The obvious solution then is to make it a *type parameter*, which is what templates let us do:

<aside name="templates">

It's hard to tell if C++ programmers learned to love templates, or if templates just scared some of them completely away from C++. Either way, everyone I see using C++ today uses templates too.

</aside>

<span name="base"></span>

^code templates

Using it looks like:

^code use-templates

<aside name="base">

The pair of classes here -- `Generator` and `GeneratorFor` -- are so you can have non-templated code that works with a Generator of any type. If we only had the templated `GeneratorFor` class, there'd be no way to write code that accepts a "vanilla" generator of any kind of monster.

`Generator` gives us that single base class that every type-specific generator is derived from. Code that doesn't care what kind of monster a generator emits can use that and avoid needing to be templatized itself.

</aside>

### First-class types

The previous two solutions address the need to have a class, `Generator`, which is parameterized by a type. In C++, types aren't generally first-class, so that requires some <span name="type-obj">gymnastics</span>. If you're using a dynamically-typed language like JavaScript, Python or Ruby where classes *are* just regular objects you can pass around, you can solve this much more directly.

<aside name="type-obj">

In some ways, the <a href="type-object.html" class="pattern">Type Object</a> pattern is another workaround for the lack of first-class types. That pattern can still be useful even in languages with first-class types, though, because it lets *you* define what a "type" is. You may need something different than the language's idea of a type.

</aside>

When you make a generator, just pass in the class of monster that it should construct -- literally the actual runtime object that represents the monster's class. Easy as pie.

With all of those other options, I honestly can't say I've found a case where I felt the prototype *design pattern* was the right answer. Maybe your experience will be different, but for now let's put that away and talk about something else: prototypes as a *language paradigm*.

## The Prototype Language Paradigm

Many people think "object-oriented programming" is synonymous with "classes". While determining the definition of OOP has the flavor of a religious debate, one rough approximation is that it *lets you define "objects" which contain both data and behavior.* Compared to structured languages like C and functional languages like Scheme, the defining characteristic of OOP is that it tightly binds state and behavior together.

You may think classes are the one and only way to do that, but a handful of guys including Dave Ungar and Randall Smith beg to differ. They created a language in the 80s called Self. While as OOP as can be, it has no classes.

### Self

In a pure sense, Self is *more* object-oriented than a class-based language. We think of OOP as marrying state and behavior, but languages with classes actually have a line of separation between them.

Consider the semantics of your favorite class-based language. To access some state on an object, you look in the memory for the instance itself. State is *contained* in the instance.

To invoke a <span name="vtable">method</span>, though, you look up the instance's class, and then you look up the method *there*. Behavior is contained in the *class*. The instance just has a references to its class. There's always that level of indirection to get to a method.

<img src="images/prototype-class.png" />

<aside name="vtable">

For example, to invoke a virtual method in C++, you look in the instance for the pointer to its vtable, then look up the method there.

</aside>

Self eliminates that distinction. To look up *anything*, you just look on the object. An instance can contain both state and behavior. You can have a single object that has a method completely unique to it.

<span name="island"></span>

<img src="images/prototype-object.png" />

<aside name="island">

No man is an island, but this object is.

</aside>

If that was all Self did, it would be hard to use. Inheritance in class-based languages (despite its faults) gives you a useful mechanism for reusing polymorphic code and avoiding code duplication. To do the same without classes, Self has *delegation*.

To find a field or call a method on some object, we first look in the object itself. If it has it, we're done. If it doesn't, we look at the object's <span name="parent">*parent*</span>. This is just is a reference to some other object. When we fail to find a property on the object, we try its parent (and its parent, and so on). In other words, failed lookups are *delegated* to an object's parent.

<aside name="parent">

I'm simplifying here. Self actually supports multiple parents. Parents are just specially marked fields, which means you can do things like inherit parents, or change them at runtime.

</aside>

<img src="images/prototype-delegate.png" />

Parent objects let us reuse behavior (and state!) across multiple objects, so we've covered part of the utility of classes. The other key thing classes do is give us a way to create instances. When you need a new Thingamabob, you can just do `new Thingamabob` (or whatever your preferred language's syntax is). A class is implicitly a factory for instances of itself.

Without classes, how do we make new things? In particular, how do we make a bunch of new things that all have stuff in common? Just like the design pattern, the way you do this in Self is by *cloning*.

In Self, it's as if *every* object supports the Prototype design pattern automatically. Any object can be cloned. To make a bunch of similar objects, you just:

1. Make one object that looks like you want. You can just clone the base `Object` built into the system and then stuff properties and methods into it.
2. Clone it to make as many... uh... clones as you want.

This gives us the elegance of the Prototype design pattern without the tedium of having to implement `clone()` ourselves. It's just built into the system.

This is such a beautiful, clever, minimal system that as soon as I learned about it, <span name="finch">I started</span> creating a prototype-based language just to get more experience with it.

<aside name="finch">

I realize building one from scratch is not the most efficient way to learn, but what can I say, I'm a bit peculiar. If you're curious, the language is called [Finch](http://finch.stuffwithstuff.com/).

</aside>

### How did it go?

I was super excited to play with a pure prototype-based language, but once I had mine up and running, I <span name="no-fun">discovered</span> an unpleasant fact: it just wasn't that fun to program in.

<aside name="no-fun">

I've since heard through the grapevine that many of the Self programmers came to the same conclusion. The project was far from a loss, though. Self was so dynamic that it needed all sorts of virtual machine innovations in order to run fast enough.

The ideas they came up with in Self for just-in-time compilation, garbage collection, and optimizing method dispatch are the exact same techniques (often implemented by the same people!) that now make many of the world's dynamically-typed languages faster than they've ever been.

</aside>

Sure, the language was simple to implement, but that was because it punted the complexity onto the user. As soon as I started trying to use it, I found myself trying to recapitulate the structure that classes give you at the library level since the language didn't have it.

Maybe this is just because most of my prior experience is in class-based languages, so my mind has been tainted by that paradigm. But my hunch is that most people just like well-defined "kinds of things". We love baseball cards and stamp collecting.

In addition to the runaway success of classes-based languages, look at how many games have explicit character classes, and a precise roster of different sorts of enemies, items, and skills, each neatly labelled. You don't see many games where each monster is a unique snowflake, like "sort of halfway between a troll and a goblin with a bit of snake mixed in".

While prototypes are a really cool paradigm, and one that I wish more people knew about, I'm glad that most of us aren't actually programming using them every day. <span name="telling">The code</span> I've seen that fully embraces prototypes has a weird mushiness to it that I find hard to wrap my head around.

<aside name="telling">

It's also telling how *little* code there actually is written in a prototypal style.

</aside>

### What about JavaScript?

OK, if prototype-based languages are so unfriendly, how do I explain JavaScript? That's a language with prototypes used by millions of people every day. More computers run JavaScript than any other language on Earth.

<span name="ten">Brendan Eich</span>, the creator of JavaScript, took inspiration directly from Self, and many of JavaScript's semantics are prototype-based. Each object can have an arbitrary set of properties, both fields and "methods" (which are really just functions stored as fields). An object can also have another object, called its "prototype", that it delegates to if a field access fails.

<aside name="ten">

As a language designer, one appealing thing about prototypes is that they are simpler to implement than classes. Eich took full advantage of this: the first version of JavaScript was created in ten days.

</aside>

But, despite that, I believe that JavaScript in practice has more in common with class-based language than with prototypal ones. One hint that JavaScript has taken steps away from Self is that the core operation in a prototype-based language, *cloning*, is nowhere to be seen. There is no method to clone an object in JavaScript.

The closest it has is `Object.create` which lets you create a new object that delegates to an existing one. Even that wasn't added until ECMAScript 5, fourteen years after JavaScript came out, and it isn't commonly used.

Instead, let me walk you through the typical way you define types and create objects in JavaScript. You start with a *constructor function*:

    :::javascript
    function Weapon(range, damage) {
      this.range = range;
      this.damage = damage;
    }

This creates a new object and initializes its fields. You invoke it like:

    :::javascript
    var sword = new Weapon(10, 16);

The `new` here invokes the body of the `Weapon` function with `this` bound to a new empty object. The body adds a bunch of fields to it, then it's implicitly returned. So now we've got a little object with some state.

The `new` also does one other thing for you. When it creates that blank object, it automatically sets its prototype (or parent in Self terms) to a prototype object. You can get that object directly using `Weapon.prototype`.

To define behavior, you usually add methods to the prototype object. Something like this:

    :::javascript
    Weapon.prototype.attack = function(target) {
      if (distanceTo(target) > this.range) {
        console.log("Out of range!");
      } else {
        target.health -= this.damage;
      }
    }

Since every object returned by `new Weapon()` delegates to `Weapon.prototype`, you can now call `sword.attack()` and it will call that function. It looks a bit like this:

<img src="images/prototype-weapon.png" />

So here we've got:

* The way you create objects is by a "new" operation that you invoke on an object that represents the type (the constructor function).

* State is stored on the instance itself.

* Behavior goes through a level of indirection (delegating to the prototype) and is stored on a separate object that represents the set of methods shared by all objects of a certain type.

Call me crazy, but that sounds a lot like my description of classes above. You *can* write prototype-style code in JavaScript (*sans* cloning), but the syntax and idioms of the language encourage a class-based approach.

Personally, I think that's a <span name="good">good thing</span>. Like I said, I find doubling down on prototypes actually makes code harder to work with, so I like that JavaScript wraps the core semantics in something a little more classy.

## Prototypes for Data Modeling

OK, I keep talking about things I *don't* like prototypes for, which is making this chapter a real downer. Everybody likes a happy ending, so let's close this out with an area where I *do* think prototypes, or more specifically *delegation*, can be useful.

If you were to count up all the bytes in a game that are code versus ones that are data, you'd see the ratio of code to data has been decreasing steadily since day one. Early games procedurally generated almost everything so they could fit on floppies and old game cartridges. Today, we think of game code as an "engine" that just runs the game itself which is defined entirely in data.

That's great, but pushing piles of content into data files doesn't magically solve the organizational challenges of a large project. If anything, it makes it harder. The reason we use programming languages is because they help us manage complexity.

Instead of copying and pasting a chunk of code in ten places, we move it into a function that we can call by name. Instead of copying a method in a bunch of classes, we can put it in a separate class that those classes inherit from or mixin.

When your game's data reaches a certain size, you really start wanting similar features. Data modelling is a deep subject that I couldn't hope to do justice here, but I do want to throw out one feature for you to consider in your own games: using prototypes and delegation for reusing data.

Lets say we're defining the data model for the <span name="shameless">shameless Gauntlet rip-off</span> I mentioned earlier. The game designers need to be able to specify the attributes for monsters and items in some kind of file.

<aside name="shameless">

I mean completely original title in no way inspired by any previously existing top-down multi-player dungeon crawl arcade games. Please don't sue me.

</aside>

One common approach is to use JSON: data entities are basically *maps*, or *property bags* or any of a dozen other terms because there's nothing programmers like more than <span name="inventing">inventing</span> names for stuff that already exists.

<aside name="inventing">

We've re-invented them so many times that Steve Yegge calls them ["The Universal Design Pattern"](http://steve-yegge.blogspot.com/2008/10/universal-design-pattern.html).

</aside>

So a goblin in the game might be defined something like:

    :::json
    {
      "name": "goblin grunt",
      "minHealth": 20,
      "maxHealth": 30,
      "resists": ["cold", "poison"],
      "weaknesses": ["fire", "light"],
    }

This is pretty straightforward and even the most text-averse designer can handle that. So you throw in a couple of sibling branches on the great Goblin Family Tree:

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

Now, if this was code, our aesthetic sense would be tingling. There's a lot of duplication between these entities, and well-trained programmers *hate* that. It wastes space and takes more time to author. You have to read carefully to tell if the data even *is* the same. It's a maintenance headache. If we decide to make all of the goblins in the game stronger, we need to remember to update the health of all three of them. Bad bad bad.

If this was code, we'd create an abstraction for a "goblin" and reuse that across the three goblin types. But dumb JSON doesn't know anything about that. So let's make it a bit smarter.

If an object has a <span name="meta">`"prototype"`</span> field, then that defines the name of another object that this one delegates to. Any properties that don't exist on the first object fall back to being looked up on the prototype.

<aside name="meta">

This makes the `"prototype"` a piece of *meta*data instead of data. Goblins have warty green skin and yellow teeth. They don't have prototypes. Prototypes are a property of the data modelling system, and not the thing being modelled.

</aside>

With that, we can simplify our goblin horde to:

    :::json
    {
      "name": "goblin grunt",
      "minHealth": 20,
      "maxHealth": 30,
      "resists": ["cold", "poison"],
      "weaknesses": ["fire", "light"]
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

Since the archer and wizard have the grunt as their prototype, we don't have to repeat the health, resists and weaknesses in each of them. The logic we've added to our data model is super simple -- just basic single delegation -- but we've already gotten rid of a bunch of duplication.

One interesting thing to note here is that we didn't set up a fourth "base goblin" *abstract* prototype for the three concrete goblin types to delegate to. Instead, we just picked one of the goblins who was the simplest and delegated to it.

That feels natural in a prototype-based system where any object can be used as a clone to create new refined objects, and I think it's equally natural here too. It's a particularly good fit for data in games where you often have one-off special entities in the game world.

Think about bosses and unique items. These are often refinements of a more common object in the game, and prototypal delegation is a good fit for defining those. The magic Sword of Head-Detaching, which is really just a longsword with some bonuses can be expressed as that directly:

    :::json
    {
      "name": "Sword of Head-Detaching",
      "prototype": "sword",
      "damage-bonus": "20d8"
    }

A little extra power in your game engine's data modelling system can make it easier for designers to add lots of little variations to the armaments and beastiers populating your game world, and that richness is exactly what delights players.
