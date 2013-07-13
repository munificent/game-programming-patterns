^title Subclass Sandbox
^section Behaving Patterns
^outline

## Intent

*Create varied behavior while minimizing coupling by defining subclasses that only use operations provided by their shared base class.*

## Motivation

Making superhero action game. Have lots of different superheroes with
different powers. Each will be subclass of Superhero. Each does
different things and touches different systems: audio, vfx, ai, etc.

Basic implementation would have subclasses coupled to all sorts
of other game systems by calling them directly. Problems:

* May be lots of redundancy in superpower code. Lots of powers do similar
  things. Want high level API to express them.

* If many people on team are implementing superheros, they won't know
  which parts of the engine should be called into, and which
  shouldn't. End up doing all sorts of bad things, touching systems
  not meant for public use.

* If you need to change a system, will have to touch every superhero
  using it.

* Lot of times, base class wants to enforce properties of its
  subclasses. Say superhero wants to ensure all audio calls get
  queued. Since subclasses talk to audio directly, can't get in way.

* Calling into other system is often too low-level and may not have
  a friendly API.

What we want is to say a superhero can perform these and only these
operations. Each superhero subclass gets a sandbox where it can work,
and a set of toys it can play with. Sandbox is an abstract method in
the base Superhero class. Toys are protected methods in base class.
Making them protected and non-virtual tells the user "these methods
are for you to use".

Derived superheroes are now coupled only to base class, and
programmers implementing them don't have access to call random
functions all over game engine.

Operations provided by Superhero can be designed specifically to be
easy and simple to use for subclasses, and can hide some of the
machinery needed to talk to raw system.

There is often one base class with a lot of subclasses (shallow but wide hierarchy). Patterns like this put more effort into base class to make subclasses easier to write. Since there's lots of subclasses, this is a net win. A little time and love put into Superhero benefits many classes. So idea is that we design Superhero to have protected methods that define the nicest API we can for creating a new kind of superhero. Superclass exposes a DSL to subclass.

## The Pattern

**Base class** defines an abstract **sandbox method**. Also provides
several protected **allowed operations**. Marking them protected
makes it clear to a user that they are for use by derived classes.
Derived **sandboxed subclass** implements sandbox method by calling
inherited allowed operations.

## When to Use It

* Very common pattern. Probably already using it.

* When all classes defining behavior share a base class.

* When the base class is able to provide all of the operations.

## Keep in Mind

*   Can make base class overly heavy. It ends up providing
    *everything* a derived class would need. Can lead to brittle base
    class problem.

    If that happens, consider moving some operations into Context
    Objects or into objects which are accessible from base class.

*   Keeps derived classes coupled only to base class. This is a good
    decoupling strategy. A small number of core base classes are
    coupled to each other. A larger set of derived "leaf" classes are
    coupled only to their parents. Since most work is in leaf classes,
    minimizes dependencies in code most people are working in. Hub and spoke
    architecture.

## Design Decisions

*   Which/how many operations to provide?

    *   Can go far down the path so that subclass *only* talks to superclass.
    *   Can dial back so that subclass talks to base for some stuff (usually
        stateful stuff) but allow coupling to certain things.
    *   The more superclass handles, the less coupled subclasses are, but the
        messier superclass becomes. Takes coupling out of subclasses, but just
        dumps it into superclass. Superclass can become huge and unweildy.

    *   Rules of thumb:

        *If you're adding something to superclass that will only
        be used by one or two subclasses, not getting a lot of bang for buck.
        Just let them call it directly.

    *   If method doesn't modify any state, then coupling directly to it is
        less risky (since a subclass calling it can't break anything).

    *   If the method you put in superclass is just a straight forwarding method
        and isn't easier to use than calling it directly, then it doesn't add
        as much value.

*   Provide operations directly in base class, or provide objects
    which in turn provide methods?

    *   Providing method directly is simpler and best for methods that
        are logically related to class's primary role.

        *Show derived entity class calling SetPosition method to move
        itself.*

    *   Wrapping methods in the base class reduces derived class's
        coupling. Aside: Law of Demeter is really "rule of thumb of Demeter".

    *   Exposing objects reduces the number of methods base class has
        to implement.

        *Show code where derived class calls a protected method to get
        a AudioService object from base class, then calls play sound
        method on it.* This way base class doesn't have to wrap every
        sound function.

    *   Exposing object lets you change service without having to also
        edit wrapper methods.

*   How does base Superhero class get state it needs?

    Base class often needs state that it wants to hide from
    subclasses. For example, provides a nice simple playSound()
    method. But implementation of that needs access to audio system,
    which we don't want derived classes to know about. Since base ctor
    is called from derived one, passing it in through ctor can expose
    stuff we don't want.

    *   Pass through ctor:

        *   Simple
        *   Ensures heroes are always fully initialized
        *   Exposes base class to implementation details
        *   If Superhero later needs more state, have to touch every
            derived ctor.

    *   Two-stage initialization:

        Superhero* createSuperhero(SuperheroType type) {
          Superhero* hero;
          switch (type) {
            case AWESOME_GUY: hero = new AwesomeGuy(); break;
            case FLAMING_MAN: hero = new FlamingMan(); break;
            case THE_TOASTER: hero = new TheToaster(); break;
          }

          hero->init(soundSystem, ...);

          return hero;
        }

        *   Keeps derived classes decoupled from state
        *   Have to make sure you don't forget to init(). Usually best to
            wrap both stages in single factory method.

    *   Static state.

        Hero has private *static* fields. Provides non-static protected accessors. Hero *class* must be initialized once at game startup.

        *   Not bad like singleton since only subclasses have access to it.
        *   Doesn't increase size of Hero instance.
        *   Requires all Heroes to share exact same state.

    *   Service locator or lazy init:

        Hero gets what it needs on its own later when it needs it.

        *   Can't forget to init like two-stage.
        *   Keeps derived classes decoupled from state.
        *   Can be slow or unpredictable.
        *   Hero has to make sure to check that it's inited state
            before it uses it.

## Sample Code

*Show Superhero class with abstract OnTakeTurn() method and protected
methods for Move(x, y), PlaySound(SoundID), etc.*

*Show derived AwesomeGuy class that overrides OnTakeTurn() and calls
some of the protected methods in Superhero.*

## See Also

*   The base class may implement the operations it provides by
    forwarding to a Service that it owns.

    *Show sample base class with a pointer to a Service and a
    protected operation method that forwards to that.*

    This keeps derived classes coupled only to base, and not service.

*   This pattern is basically Context Parameter, where the parameter is `this`.