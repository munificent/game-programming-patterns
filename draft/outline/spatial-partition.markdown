^title Spatial Partition
^section Optimizing Patterns

## Intent

*Quickly find objects near a certain point or area by storing them in a data
structure that organizes them by location.*

## Motivation

- games inhabit virtual world. that world often mimics the real world.
- one facet of that is things have a position and interact in physical space.
- in other words, you can be near stuff, and stuff can collide.
- obvious example is physics and collision, but other stuff too.
    - for online games, maybe chat messages are only heard by other players near
      you
    - only render things near the player (though visibility culling is a bit
      different)
    - audio system may take into account which objects are nearby
- all of this boils down to, given a location in the game world, what's nearby
- we'll pick collision as example
- for collision, need to see which pairs of objects are touching.
- comparing each pair in whole world is n^2!
- as number of objects increases, gets worse and worse
- think about hash table
    - hash table lets you find object with key in constant time instead of n for
      searching the whole collection
    - what if we could make a hash table where the key is its position?
    - this is basic idea of spatial partition: organize objects in data structure
      based on where they are
    - literal hash table with positions as keys wouldn't work, though. often need
      to find objects *near* a position. hash tables, by design, don't handle
      approximate keys well. they intentionally spread objects out so that a tiny
      key change is a big location change.

## The Pattern

- create data structure where objects are organized based on location in game
  world
- allow efficient queries to find objects at or near a location

## When to Use It

- when objects have position
- doing frequent queries to find nearby objects
- there are a lot of objects

## Keep in Mind

- goal is generally to reduce a n^2 operation to n or a n to 1. if number of
  objects is actually small, may not be worth it.

- if objects can move, they have to be reorganized in the data structure.
  there is some overhead to this (especially with hierarchical spatial
  partitions) so make sure benefits outweight organization

- data structure has some memory overhead. as usual, trading memory for perf.

## Sample Code

- lots of spatial partition data structures. area of active research.
- since we just care about pattern, we'll do the simplest one: a fixed grid.
- rts. lots and lots of units running around battlefield. need to see which
  units are near enough to engage in melee.
- naive solution
    - for each unit
        - for each other unit
            - skip redundant comparisons
            - bounding box test
            - see if distance < minimum
- naive solution is n^2.
- want to have *lots* of units and game is slowing down.
- introducing grid
    - overlay 2d grid overworld. each cell has a fixed size.
    - each cell stores list of units
    - units whose position falls within that cell are in that cell's list
- finding collisions
    - for each unit
        - find its cell
            - for each other unit in cell
                - now same as before
    - still seems to be n^2: two nested loops
    - but since there are many cells, each cell will have fewer units
- neighboring cells
    - if near cell boundary, may be touching units whose centerpoint is in
      neighboring cell. need to take those into account too.

## Design Decisions

- which specific spatial partition to use. lots of options: grid, bsp, etc.
- does it adapt automatically to the set of objects or is it fixed like a grid?
    - fixed is simpler
    - may be faster too
    - adaptable works better for large levels, or with user-authored content
- some are optimized for objects that don't move
- are objects *only* stored in spatial partition or is there a separate direct
  list of them too?
    - former uses less memory. can be slow if you need to visit all objects and
      most of the spatial partition is empty.
- is it hierarchical or not?
    - hierarchical works better when objects are non-evenly distributed ("clumpy")
    - flat is simpler

## See Also

* http://en.wikipedia.org/wiki/Space_partitioning
* http://en.wikipedia.org/wiki/Grid_(spatial_index)
* http://en.wikipedia.org/wiki/Kd-tree
* http://en.wikipedia.org/wiki/Binary_space_partitioning
* http://en.wikipedia.org/wiki/Quad_tree
