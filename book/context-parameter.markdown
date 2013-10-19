^title Context Parameter
^section Communicating Patterns

## Intent

*one sentence*

## Motivation

- key principle of oop design is controlling access to state
- since most behavior is instance methods, if you control access to an instance, control access to behavior too.
- lot of programming is about not writing code that does the wrong thing
- by carefully controlling which objects a piece of code can get to, can ensure
  can't do the wrong thing

- flip side of coin is that in order for code to do what it does need to do,
  needs access to objects
- if a method needs some object, has couple of options
  - can create one itself
  - can make it global or a singleton
  - can make it a field
  - can pass it in as an arg

- creating one doesn't always work. often need some pre-existing object that's
  already wired up with rest of program [ex...]
- don't make it global. read singleton chapter and come back.
- leaves two options, making it a field, and passing it in

- making it a field is heavyweight
  - makes each object bigger
  - have to make sure have access to object when you create it
  - if it can change, have to make sure you change it at the right time

- for things that don't conceptually feel like part of object, really prefer
  not to make a field for it. if it's just a thing it *uses* or *needs*, think
  arg is better fit

- (one exception is a "parent" field. when have one object that owns other,
  often helpful for child to be able to get back to parent.)

- this chapter is about one flavor of object that often need that you can avoid
  making a field for.

- as name implies, it's about parameters that provide some larger "context" a method needs.

- [example...]

- for all that setup, this is a very small, simple, pattern.
  - won't blow your mind
  - probably already using it, often with name "context"
  - but i think it's helpful to write stuff like this down
  - helps crystallize thinking about it
  - makes it easier to be deliberate about using it

## The Pattern

one para with **some bold stuff**

## When to Use It

few paras

## Keep in Mind

### This is a thing to keep in mind

few paras, then repeat for sections

## Sample Code

[show class that has method that takes context param, and surrounding code that invokes it. probably rendering engine.]

## Design Decisions

intro

### A question?

intro

* **Option 1:**

    * *Point.* sentences

## See Also

* bullets
