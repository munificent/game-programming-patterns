^title Prototype
^section Design Patterns Revisited

- chapter ostensibly about prototype design pattern, but hard for me to not
  talk about lang paradigm too
- get grounded, here's example of design pattern first
- imagine making game like gauntlet
- lots of different monsters
- and generators in level that pump out monsters of different types
- for purposes of example, say we have different classes for each kind of
  monster
- in practice, probably use type object or components or something
- (in fact, type object is alternative to prototype)
- need a generator for each class of monster
- brute force solution is separate class for each kind of generator
- then use factory pattern to have each one create the right kind of monster
- of course, could just use callback instead
- still need callback fn for each class
- other option is prototype pattern
- add abstract Clone() method to monster base class
- each monster must implement this and return copy of itself
- when we create generator, we give it instance of monster
- this monster is prototypical monster
- when generator needs to create monster, it clones prototype
- now only need one generator class, and no callbacks
- of course, do have to implement clone in each monster class
- one neat thing about this is generators can vary not just on class of monster
  but on *attributes* too
- could create generator with protypical goblin with tons of health or just a
  little
- assuming clone clones health, each generator will produce matching goblins
- lots of flexibility
- basically lets you use an actual instance of some class, and all of the
  detailed state that object has and turn around and use it as a template to
  stamp out other similar objects

- that's prototype *pattern*. even if you've heard of "prototypes" before, good
  chance didn't know the pattern, so now you do
- kind of interesting, but not that useful pattern for code, honestly
- implementing clone isn't much less work than implementing a factory for each
  class

- as a design pattern for code, not too useful, but it crops up in other places
- first is programming languages
- some oop languages aren't class-based, instead they are prototype-based

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

