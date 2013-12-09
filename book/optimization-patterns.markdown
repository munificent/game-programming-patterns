^title Optimization Patterns

While the rising tide of faster and faster hardware has lifted most software above needing to worry about performance, games are one of the few remaining exceptions. Players always want richer, more realistic and exciting experiences. Screens are crowded with games vying for a player's attention -- and cash! -- and the game that squeezes the most out of the hardware often wins.

Optimizing for performance is a deep art that touches all aspects of software. Low-level coders master the myriad idiosyncracies of hardware architectures. Meanwhile, algorithms researchers compete to prove mathematically whose procedure is the most efficient.

Here, I touch on a few mid-level patterns that are often used to speed up a game. [Data Locality](data-locality.html) introduces you to the modern computer's memory hierarchy and how you can use it to your advantage. The [Dirty Flag](dirty-flag.html) pattern helps you avoid unnecessary computation while [Object Pools](object-pool.html) help you avoid unnecessary allocation. [Spatial Partitioning](spatial-partition.html) speeds up the virtual world and its inhabitants' arrangement in space.

<!--
Videogames are exciting in large part because they take us somewhere else. For a few minutes (or, let's be honest with ourselves, much longer), we become inhabitants of a virtual world. Creating these worlds is one of the supreme delights of being a game programmer.

One aspect that most of these game worlds feature is *time*: the artificial world lives and breathes on its own cadence. As world builders, we must invent time and craft the gears that drive our game's great clock.

The patterns in this section are tools for doing just that. A [Game Loop](game-loop.html) is the central axle that the clock spins on. Objects hear its ticking through [Update Methods](update-method.html). We can hide the computer's sequential nature behind a facade of snapshots of moments in time using [Double Buffering](double-buffer.html), so that the world appears to update simultaneously.
-->

## The Patterns

* [Data Locality](data-locality.html)
* [Dirty Flag](dirty-flag.html)
* [Object Pool](object-pool.html)
* [Spatial Partition](spatial-partition.html)
