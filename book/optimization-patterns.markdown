^title Optimization Patterns

While the rising tide of faster and faster hardware has lifted most software
above worrying about performance, games are one of the few remaining exceptions.
Players always want richer, more realistic and exciting experiences. Screens are
crowded with games vying for a player's attention -- and cash! -- and the game
that pushes the hardware the furthest often wins.

Optimizing for performance is a deep art that touches all aspects of software.
Low-level coders master the myriad idiosyncrasies of hardware architectures.
Meanwhile, algorithms researchers compete to prove mathematically whose
procedure is the most efficient.

Here, I touch on a few mid-level patterns that are often used to speed up a
game. [Data Locality](data-locality.html) introduces you to the modern
computer's memory hierarchy and how you can use it to your advantage. The [Dirty
Flag](dirty-flag.html) pattern helps you avoid unnecessary computation while
[Object Pools](object-pool.html) help you avoid unnecessary allocation. [Spatial
Partitioning](spatial-partition.html) speeds up the virtual world and its
inhabitants' arrangement in space.

## The Patterns

* [Data Locality](data-locality.html)
* [Dirty Flag](dirty-flag.html)
* [Object Pool](object-pool.html)
* [Spatial Partition](spatial-partition.html)
