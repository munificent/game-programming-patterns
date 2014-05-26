^title Decoupling Patterns

Once you get the hang of a programming language, writing code to do what you
want is actually pretty easy. What's hard is writing code that's easy to adapt
when what you want *changes*. Rarely do we have the luxury of a perfect feature
set before we've fired up our editor.

A powerful tool we have for making change easier is *decoupling*. When we say
two pieces of code are "decoupled", we mean a change in one usually doesn't
require a change in the other. When you change some feature in your game, the
fewer places in code you have to touch, the easier it is.

The [Component pattern](component.html) decouples different domains in your game from each other within a single entity that has aspects of all of them. [Event Queues](event-queue.html) decouple two objects communicating with each other, both statically and *in time*. The [Service Locator pattern](service-locator.html) lets code access a facility without being bound to the code that provides it.

## The Patterns

* [Component](component.html)
* [Event Queue](event-queue.html)
* [Service Locator](service-locator.html)
