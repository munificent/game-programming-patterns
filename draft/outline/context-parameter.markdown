^title Context Parameter
^theme Behaving
^outline

## Intent
*Define behavior in terms of operations on a given context object
passed to the method.*

## Motivation
Each entity defines a Render() function to draw the entity. In a
naïve implementation, would call low-level render API directly.
But that couples entity code to entire render API, some of which may
be too low-level.

Worse, in order to render, need to know right state. For example,
local transform of entity, camera position, which buffer to render to,
etc.

We need a way to bundle this contextual information and have entity
implement its rendering in terms of that context.

## The Pattern
**Base class** defines a **contextual operation** as an abstract
method that takes a **context object**. Context object maintains state
needed to perform operations, and exposes **basic operations** that
use it. Derived **concrete class** implements the contextual operation
by calling methods on the context object.

## When to Use It
* When the context and operations needed aren't already available
  either globally (see Service) or from the base class itself (see
  Subclass Sandbox).

* When the context needed to perform an operation is not owned by the
  class defining the operation. (In our example, Entity doesn't own
  the render state.)

## Keep in Mind

## Design Decisions

## Sample Code

## See Also

*   Sandbox works similarly, but used protected methods in base class
    instead of passed in object.