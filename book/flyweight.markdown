^title Flyweight
^section Design Patterns Revisited

*Sorry, this chapter hasn't been written yet!*


http://sourcemaking.com/design_patterns/flyweight

"Extrinsic state is stored or computed by client objects, and passed to the Flyweight when its operations are invoked."

- in other words, can make flyweight operations context-free by passing in context.

- some overlap with type object. both patterns delegate some of object's definition
  to other shared object.

http://javapapers.com/design-patterns/flyweight-design-pattern/
"Flyweight is used when there is a need to create high number of objects of almost similar nature. High number of objects consumes high memory and flyweight design pattern gives a solution to reduce the load on memory by sharing objects. It is achieved by segregating object properties into two types intrinsic and extrinsic."


- pattern is sort of about saving memory, but memory is cheap
  - however, cache is not


gof:
"Facilitates the reuse of many fine grained objects, making the utilization of large numbers of objects more efficient."


- got refers to "instrinsic" and "extrinsic" state. prefer "contextual" and "context-independent".

- common place where pattern is used in games
  - explain instanced rendering
  - key concepts: separate out data specific to single object in world (transform)
    from data it has in common with others (mesh)
  - then reuse the latter
  - talk about how rendering api supports instanced rendering directly: hardware
    understands flyweight pattern.
- then say, that's general idea, here's other application
- tiles or other places where you'd use an enum and lots of switches
- can instead do a pointer
- talk about perf of pointer chasing, field lookup, virtual method invoke
- discuss using flyweight both for data and for behavior

- flyweights almost always immutable: if you change it, everything that uses it
  will change

- create them upfront or as needed? if latter, think about threading

- compare to string interning
