^title Prototype
^section Design Patterns Revisited

Ostensibly, we'll just be talking about the Gang of Four's "Prototype" design pattern here. But "prototype" these days means a good bit more than that pattern, and, as we'll see, the design pattern itself is just about the least interesting use of the term. We'll talk about prototypes as a programming language paradigm, as well as applying it an increasingly important part of games: data modelling.

## The Prototype Design Pattern

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

The generator internally holds a monster. This isn't a monster that's running around in the level. Instead, it's a hidden monster whose sole purpose is to be used by the generator as a template to stamp out more monsters like itself.

To create a ghost generator, we just create this prototypical ghost instance, and then create a generator using that prototype. One neat part about this pattern is that it doesn't just clone the *class* of the prototype, it clones its *state* too. This means we could make a generator of fast ghosts, or weak ones, or slow ones, just by creating an appropriate prototype ghost.

It's a neat idea. It has a sort of surprising elegance to it, and I can't imagine coming up with it myself. So it's great, right?

## How Useful Is It?

Well, not quite. Sure, we don't have to create a separate generator class for each monster now, but we still have to implement `clone()` in each monster class. That's just about the same amount of boilerplate when you look at it. There are also some really nasty semantic ratholes when you sit down to actually try to write a correct clone. Does it do a deep clone or shallow one? (I.e. when a field is itself a reference to an object, do we clone that object, or just the reference?)

And, of course, to even get this far, we've presumed that we have a bunch of separate classes for each monster. These days, that's definitely *not* the way most game architectures roll. Most of us have learned the hard way that big class hierarchies like this are a pain to manage, which is why we instead use patterns like <a href="component.html" class="pattern">Component</a> and <a href="type-object.html" class="pattern">Type Object</a> to model different kinds of behavior without enshrining each in its own class.

### Generator functions

Even if you do have different classes for each monster, there are other ways to skin this cat. Instead of making separate generator *classes* for each monster, you could make generator *functions*, like so:

^code callback

Then the one generator class can just store a function pointer:

^code generator-callback

To create a generator for ghosts, you just do:

^code generate-ghost-callback

### Templates

By now, most C++ developers are at least a little familiar with templates. Our generator class needs to construct instances of some type, but we don't want to hard code some specific monster type name. The obvious solution then is to make it a type parameter, which is what templates let us do:

^code templates

Using it looks like:

<span name="base"></span>

^code use-templates

<aside name="base">

The pair of classes here -- `Generator` and `GeneratorFor` -- are so that you can have non-templated code that works with a Generator of any type. If we only had the templated `GeneratorFor` class, there'd be no way to write code that accepts a "vanilla" generator of any kind of monster.

`Generator` gives us that single base class that every type-specific generator is derived from. Code that doesn't care what kind of monster a generator emits can just use that and not have to worry about template shenanigans.

</aside>

### First-class types

All of these dance around the basic problem which is that we have a class, `Generator` that we need to parameterize by a type. Templates are how you do that in C++ because types aren't generally first-class. But if you happen to be using a dynamically-typed language like Smalltalk or Ruby where classes *are* just regular objects you can pass around, then the natural solution is to do that.

When you make a generator, just pass in the class of monster that it should construct -- literally the actual runtime object that represents the monster's class. Easy as pie.

With all of those other options, I honestly can't say I've found a case where I felt the prototype *pattern* was the right answer. So let's put that in our Box of Clever but Not Useful Ideas and talk about something else: prototypes as a *language paradigm*.

## what are classes and prototypes for?

- can look at class-based languages and prototype-based languages as competing
  approaches to solving problems

- both based on observation that in a program

  - have lots of objects that are similar: same structure, same behavior, often
    even similar state
  - need way to make objects of a given flavor like this

- class based languages say:

  - objects are similar because you explicitly define a class that defines the
    behavior and sometimes structures of all of those objects
  - any given object is explicitly a member of some class
  - the class itself is the mechanism for creating new objects of that type
    - whatever the language, always mention class name when creating new
      instance

- prototype-based languages say:

  - objects are similar because they all came from the same prototypical
    instance. (or came from things that did.)
  - any given object has no explicit class or type. it's ad-hoc
  - to create new objects of a given type, you just find one that's already
    similar to what you want and clone it.

  - cloning is the fundamental idea behind prototypes. if you want an object
    like X, find X and copy it

- prototypes have lot going for them
- any object can become effectively like a "class"
- don't have ceremony and boilerplate of defining classes up front
- especially in languages like c++ and java, can spend a lot of time just
  writing all of the scaffolding required to define classes
- because cloning clones an object's *current* state, not only can any object
  define a "kind" of thing that you can create more of, but an object can define
  different kinds of things over time as its state changes.
- really flexible, really powerful
- at same time, really simple
  - if you're doing language design, you can make a minimal prototype-based
    language much more easily than a minimal class-based one
  - class-based languages need a lot of features: syntax for defining classes,
    instantiating them, defining constructors, static properties, etc.
  - with prototypes, you just need a way to modify objects and a clone operation

## how well does it work?

- sounds like a win win: simple language, super powerful. we should all be
  using prototype-based languages, right?

- well, as it turns out, most of us are these days
- but we aren't actually using the prototype paradigm, and the reason we're
  using a prototype-based language is mostly a historical accident

- the prototype concept is virtually as old as software. ivan sutherland's
  sketchpad demo in the sixties (!) showed it in beautifully intuitive graphic
  form.

- design pattern has been known and used since then

- but first *language* to try to double down on this model was self

- self is fascinating, really unique language

- also spawned the careers of many of the brilliant vm hackers who've made the languages we love today run as fast as lightning. self was so hard to optimize, they had to invent all sorts of techiniques to make it go fast, and it turns out those techniques work equally well in lots of other languages

- but self itself is virtually dead. great research vehicle, but no one uses it.

- have heard secondhand that even many of the original people on self team came to conclusion that it just wasn't a good way to write programs.

- turns out too much flexibility can be a good thing.

- seems like people like to think in terms of well-defined kinds of things.

- (just look at how many games have "classes", and differently sharply codified kinds of characters, moves, creatures, etc.)

- the pure prototype model of "just beat an object into some shape and then make a bunch of copies of it" never seem to catch on for code

## javascript

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

