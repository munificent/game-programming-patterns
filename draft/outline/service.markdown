^title Service
^theme Communicating
^outline

## Intent
*Provide a global point of access to a service, without coupling users
of the service to the concrete class that implements it.*

## Motivation
Many systems in a game need to cause sounds to play. Want to provide
global access to ability to play sound.

Usually use a Singleton for this, but that still couples to concrete class, and also brings in an unneeded single instance limitation.

Better is to separate out the class providing *access* to a service
from the class *implementing* it. This way, game is only coupled to
abstract service class, and not any concrete audio classes.

## The Pattern
**Service Provider** class provides a globally-accessible reference to an instance of a **Service**, an interface for a set of operations. At
runtime, an instance of a **Service Implementation** is registered
with the provider.

## When to Use It
Use it when many things will want access to a single content-independent module: audio, networking, visual effects.

Very useful when there may be multiple implementations of the service.

## Keep in Mind
*   It's global, anything globally accessible can still cause unwanted
    interdependencies. Don't overuse.

*   Unlike Singleton, Service isn't automatically instantiated. Make
    sure you register a service before it's used.

*   Service doesn't have to be *global*. Could also limit it to a
    base class. For example, audio service is registered with the
    base Actor class, which then exposes it to all subclasses.

*   Have to go through v-table for calls. May not be efficient enough
    for services used very frequently.

## Design Decisions

*   What is the scope of the service (who can use it)? Global?
    Namespace? Class? Narrowed minimizes coupling, but can cause you
    to have multiple classes store reference to same service. Means
    you have to be careful that the service is registered everywhere
    it needs to be.

*   Who registers the service? One place in code will be coupled to
    both the service provider, and the concrete implementation.
    
*   What happens if service before implementation is registered?

    Can handle this with a **Null Service**: an implementation of the
    service that does nothing. Return instance of this if you try
    to use Service without registering implementation. Allows you to
    safely turn off features, such as removing audio during 
    development.

*   What is the API for the service itself? This is a public interface
    that will be used and implemented frequently. Take care to design
    it well.

## Sample Code

### The Service
*Show an example AudioService API, an abstract class with a couple of
PlaySound() methods.*

*Show an example implementation, AudioImpl (bodies of functions will
not actually be implemented).*

### A Simple Provider
*Show static class AudioProvider. Has methods to get and set
AudioService pointer.*

### Service Unavailable
*Show NullAudioImpl null service class. Show how AudioService returns
a pointer to that if no other service is registered.*

### Logging Decorator
Logging is useful during development, but don't want the performance
hit in the shipping game. Often use #ifdefs for this, but we can also
use Service and Decorator to a cleaner effect.

*Show AudioLogger class that decorates AudioService by logging and
then forwarding call to wrapped instance.*

*Show how code registering service can conditionally wrap in logging
when desired.*

## See Also
*Compare to Singleton.*