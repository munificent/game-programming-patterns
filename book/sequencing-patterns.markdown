^title Sequencing Patterns

Videogames, even simple ones like solitaire are exciting in large part because they take us somewhere else. For a few minutes (or, let's be honest with ourselves, often much longer), we become inhabitants of a virtual world. Creating these worlds is one of the supreme delights of beng a game programmer.

One aspect that most of these game worlds feature is *time*: the artificial world lives and breathes on its own cadence. As builders of these game worlds, we must invent time and fashion the gears that drive our game world's great clocks.

The patterns in this section are tools for doing just that. A [Game Loop](game-loop.html) is the central axle that your game's clock spins on. Object can hear the ticking of that clock and act in turn by implementing [Update Methods](update-method.html). We can hide the computers sequential and incremental nature nature behind a facade of well-defined snapshots of moments in time using [Double Buffering](double-buffer.html), so that the entire world appears to update simultaneously.

## The Patterns

* [Game Loop](game-loop.html)
* [Update Method](update-method.html)
* [Double Buffer](double-buffer.html)
