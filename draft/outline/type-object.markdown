^title Type Object
^section Behaving
^outline

## Intent
*Allow new "classes" to be defined easily by creating a single class each instance of which describes a different type of object.*

## Motivation
*   we're tasked with implementing the monsters for a game. monsters
    have a bunch of properties: health, damage, graphics, etc.

*   there are many different breeds: trolls, dragons, gnomes, etc. and
    the breed determines many of the stats: every troll has the same
    attacks and art, etc.

### The Typical OOP Answer

*   we're using an oop language, so we start defining classes. first
    a base Monster class
    
    class Monster
        virtual int GetDamage() = 0;
        virtual string GetAttackText() = 0;
    
    then we define subclasses:
    
    class Troll
        GetDamage return 4
        
    class Dragon
        GetDamage return 20
        
*   all is well and good and before long we have dozens of monster
    subclasses.
    
*   now starts to bog down. every time we want to add a new monster,
    we have to create a new class and recompile. when a designer wants
    to change a stat, they have to ask a coder to do it. slow!
    
*   what we want is for designers to be able create and modify monster
    classes on their own, but they don't know c++

### A Class for a Class

*   the key observation is that our subclasses don't really do much
    expect provide different values. if that's all we need, we don't
    need to use c++'s type system.
    
*   instead, we'll define our own type that represents a breed of
    monster. it has fields for the damage, attacks, and other
    breed-specific properties.
    
*   each monster instance will have a reference to its breed. every
    monster of the same breed points to the same breed instance.

*   when the monster needs breed-specific data, it pulls it from its
    breed ref.
    
*   the nice thing now is that everything about a monster breed is
    stored in member vars. if we give breed a ctor that reads those
    from a file, we now have completely data-driven monster types.
    designers can create a new data file on their own and get a new
    breed in game.

## The Pattern

Define a **type object** class and an **object** class. Each
*instance* of the type object class represents a different logical
type. Instances of the object class store a reference to the type
object that describes their type.

Instance-specific data is stored in the object instance, and data or
behavior that should shared across all instances of the same
conceptual type is stored in the type object. Objects referencing the
same object will function as if they were the same type. This lets us
share data and behavior across a set of similar objects much like
subclassing lets us do, but without having a fixed set of hard-coded
subclasses.

## When to Use It

*   you need to share data/behavior between instances. if each monster
    was totally unique, this wouldn't buy us anything.

*   you want greater flexibility that subclassing gives you: either
    you don't know the set of subclasses you'll need, or you want to
    be able to define them in data

## Keep in Mind

*   each type object instance must be instantiated and kept in memory
    unlike raw classes which are automatically put into static
    memory by the compiler.

*   tends to be more limited than subclassing. with subclasses, you
    can override a method and do anything you want. with type objects,
    you're usually limited to just storing different values. can
    make them more expressive by using function pointers. if you're
    defining metaclass instances in data, one option is to use
    bytecode to make them more expressive.

## Sample Code

### A Basic Type Object

start simple. for our example, monsters have health and a string they
use when attacking. the current health is instance specific (it goes
down when you punch it in the face), but the starting health is
determined by the breed. the breed also determines the attack string.

we'll start with the breed.

    class breed
        breed(starting, attack)
        
        int getstartinghealth { return mstarting; }
        const char* attack { return mattack; }

pretty simple. it's basically just a container for some data: starting
health and the attack. now let's show a basic monster

    class monster
        monster (int health, breed)
        attack()
            cout << mbreed->attack
        breed* mBreed
        int mhealth

when we create a monster now, we give it a breed now instead of
subclassing. attack function uses breed to get string.

as it stands, not using starting health from breed. the assumption is
that we'll use it when we construct the monster. we can codify that
assumption and make things a little more solid by making type object
even more like a class: we'll give it responsibility for constructing
monsters

    class breed
        monster* new()
            return new monster(mstartinghealth, this)

the new method in breed now makes a monster and properly sets the
breed and starting health. now, to outside code, the breed really is
the entry point for monsters: you create a new one by asking the 
breed to instantiate it.

we can make this more explicit by making another slight change:

    class monster
        private ctor
        friend class breed
        
now, *only* breed can make monsters. our type objects now work like
classes in languages like ruby and smalltalk where you create new
instances by asking the class to.

### Making it Data Driven

now we can define monsters that share attributes, but how is this
better than subclassing? the important part is that the breed class
is just a data container now. this means it's easy for us to define
a breed in a data file like xml:

    <breed>
      <health>100</health>
      <attack>The orc punches you in the face!</attack>
    </breed>

and then we can right some simple code to parse the file and construct
breeds:

    loadbreed(xml)
        int health = xml.getnode("health").asint;
        char* attack = xml.getnode("attack");
        return new breed(health, attack)

(this isn't a book on xml, so you'll have to image we have a nice xml
lib here.)

now not only do we get the benefits of subclassing, but we can now
define new breeds without having to touch a line of code or recompile.
so easy a designer could do it!

### Sharing Data Through Inheritance

what we have so far is perfectly serviceable especially for our simple
breed example. imagine we have hundreds of different breeds, each with
dozens of different props. if a designer wants to make all of the
thirty different elf breeds a little stronger, it's going to suck to
have to edit that in thirty places.

in oop languages, we can solve problems like this using inheritance.
let's see if we can add that to our type object system. we just do
single inheritance. each breed gets a parent breed:

    class breed
        breed(startinghealth, attack, breed* parent)
        breed * mparent

to have the parent breed be useful, a child needs to be able to
indicate what properties its overriding and what it's leaving alone.
a real-world system may do something more complex, but for our little
example, we'll say that breed overrides by having non-zero health
or non-null attack. if zero or null, just inherit value:

    class breed
        gethealth
            if health != 0 return health
            return parent->gethealth
        getattack
            if attack != null return attack
            return parent->attack

now, if we were defining breeds in xml, we could do something like:

    (aside: studies show ! in attack makes the game 10x more 
    exciting!)
    
    <breed name="elf">
        <health>25</health>
        <attack>The elf hits you!</attack>
    </breed>
    <breed name="elf archer" parent="elf">
        <attack>The archer fires an arrow!</attack>
    </breed>
    <breed name="elf wizard" parent="elf">
        <attack>The wizards casts a spell on you!</attack>
    </breed>
    
if we construct the breeds appropriately for that data, and default
any missing prop to zero or null, then the elf wizard and archer will now inherit their health from the parent elf breed. if our designer
laters want to change the health of all elves, they only need to
touch that.

### More Powerful Overriding

what we have now is powerful enough to cover most use cases you'll 
ever encounter. but, for kicks, lets talk about a more powerful
system.

with what we have so far, overriding means just replacing the value
with the one the parent provides. how about being able to combine the
parent and child values in different ways? let's ignore the attack
stuff for now and just focus on health. consider:

    class breed
        breed(breed* parent, float multParenthealth, int addparenthealth)
        
        gethealth
            int health = 0
            if parent != null) health = parent->gethealth
            
            health *= multparenthealth
            health += addparenthealth
            
            return health

with this, our child breeds can now override the parent health by
applying a multiplier and/or an additive bonus to it. with this, we
could give a base health for elf, and then make a warrior elf whose
health is the base * 1.5 + 10. if we want to totally replace the
parent health, just use a zero multiplier.

you probably won't use this, but if you find yourself building a very
comprehensive content definition system, it's something to think about.

### Objective-C and Real Type Objects ###

the hoops we've jumped through so far let us create an object that
represents a class in c++ because in c++, classes themselves aren't
objects. let's look at a language where they are: obj-c (others too:
smalltalk, etc.)

we can implement same pattern in objective c, but make our breed
instances actual obj-c classes.

**bob: obj-c magic!**

## Design Alternatives

### encapsulate the type or expose it?

encapsulating means that to outside code, objects are essentially
typeless. the fact that groups of them happen to share certain
properties is coincidental, and the internal type objects are just an
implementation detail to make that work well.
    
if its exposed, the type objects become part of the contract of the
system itself: all objects have a type.

*   encapsulate

    *   outside code is abstracted away from use of type object
        pattern
    
    *   object class can selectively override to provide
        instance-specific behavior
    
    *   have to write forwarding methods for everything the type
        supports in object class

*   expose

    *   outside code can interact with class directly without
        being associated with a specific object. for example,
        the type object can have a "ctor" that outside code can
        call.
        
    *   type object pattern is now part of public api of class.

*   mix - expose type object through a restricted interface that
    lets you do some stuff on the type directly but requires you
    to go through an object for others.
    
    *   lets you control which things are object-overridable and
        which are tied only to the type object itself
        
    *   more complex

### how are objects constructed?

each object is now a pair: an object and the type object it uses. how
is this pair built?

*   construct object and pass in type

    *   lets outside code control where object is allocated.
    
    *   simpler

*   call "ctor" function in type object, which returns object

    *   type object controls allocation. could be good if you want
        it to use its own object pools. could be bad if outside code
        wants to use *its* own object pools.
        
    *   gives type object a chance to do other validation and initing,
        for example, monster has a current health. the starting value
        for that comes from the breed. if the breed constructs 
        monster, it can initialize that properly.

    *   i like this option. it feels more oop-like: you're basically
        calling a ctor.

### can the type change?

we usually assume that once an object is created, its type object is
locked down, but it could change.

for example, when a monster is killed, we could change its breed to a
special "corpse" breed so that we can see the body on the ground.

*   type object doesn't change

    *   simpler. lines up with oop assumptions.
    
    *   easier to debug.
    
*   type object can change

    *   less swapping out of objects. for example, a shape-shifter
        can simply swap its type object. without that, we'd have to
        create a new object with the new type, copy any 
        object-specific values over and discard the old one.

    *   need to be careful that type object's assumptions aren't
        invalidated. object and its type are usually tightly coupled.
        for example, a breed may assume the monster's health is
        always within some breed-dependent range. if we swap the type
        but leave the existing health, that invariant may no longer
        be true.
        
        when swapping types, we may need to perform a validation step
        to make sure the object's per-instance state makes sense with
        the new type too.

### what kind of inheritance is supported?

this is a bit open-ended. when designing a type object system, you're
basically designing a type system for a programming language: the
field is wide open to support inheritance, multiple inheritance, 
mixins or categories, or any other feature you can come up with.

can't cover all cases, but will provide some notes for a few:

*   flat types, no inheritance etc.

    *   simplest, which is often best.
    
    *   for data-driven types, easiest for non-technical users to
        author and understand.
        
    *   fastest, don't have to walk inheritance chain.
    
    *   can lead to a lot of duplication across types. i've yet to see
        a system where designers *didn't* want inheritance: when 
        you've got 50 different kinds of elves, having to tune their
        health by changing a number in 50 places *sucks*.

*   single inheritance.

    *   pretty simple to implement and understand. good compromise
        between simplicity and allowing data reuse.
        
        there's a reason a lot of programming languages stop here: it
        seems to be a sweet spot.

*   multiple inheritance

    *   more powerful and flexible. lets you do things like Zombie
        Dragon which is both a Dragon and a Zombie.
        
    *   complex to understand. which properties are controlled by
        which base classes? users need to understand how the system
        traverses the inheritance graph.

## See Also

*   some languages support this intrinsicly: Obj-C, Smalltalk

*   another option is to use prototypes and not have "classes" at all

*   this is similar in some ways to the state pattern: the type object
    defines *some* state for the object, which happens to be shared
    across multiple instances. if you change the type object an
    object is using, it will appear to change its class, much like
    changing the state in the state pattern does.
