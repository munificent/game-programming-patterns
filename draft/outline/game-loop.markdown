^title Game Loop
^section Sequencing Patterns
^outline

## Intent

*Simulate the progression of time and run smoothly independent of user input.*

## Motivation

- old programs used to be batch: ran and quit
- then came interactive programs. these ran forever, which meant loop
- if you write gui app, the os has event loop
  - you receive ui event, handle it, and return to event loop
  - js is a great example
  - you don't see loop, but it's there
  - when app isn't handle user input, it isn't doing anything, just sitting
    there
  - this can work for simple games (minesweeper, solitaire)
- game loop
  - games are living breathing worlds. need to keep simulating independent of
    user input. don't wait.
  - even turn based games often have animation that needs to keep going even
    when user isn't doing anything
  - solution is game does loop itself. polls for input instead of blocking

### Two Kinds of Time

- two kinds of time: user's wall clock time. game's simulation time (ticks)
- exchange rate between them: how much *work* can the cpu handle in a given
  amount of real time. old games could hard code exchange rate.
- modern hardware is too flexible: run on range of devices, multitasking,
  battery life, etc.
- need to dynamically calculate exchange rate

- out of scope
  - networking and coordinating time
  - threading: sim on one, render on another, etc.

## The Pattern

A **game loop** runs continuously until the game is exited. Each turn of the
loop, it **reads user input**, then **updates the game state**, then **displays
the game**. It will also keep track of the passage of time so that it can
control the **rate of gameplay**.

## When to Use It

One of the few things common to almost every game. You will almost always use
it.

Counterexample is non-realtime games that rely on OS GUI. If you write a
Minesweeper clone, may not to code a game loop. Still there, though: it's the
OS's event loop.

may not write it yourself. if using game engine, engine will usually provide it. this is one of the main differences between a "library" and an "engine" or "framework". with engine, it owns the main loop and calls your code. lib is other way. game loop is drive shaft of engine.

will likely even use this for turn-based games since animation, vfx, etc. is
realtime. a turn-based game is just a realtime game where the enemy doesn't
move until after the player does.

## Keep in Mind

- need to coordinate with os' event loop
- consider different hardware capabilities game will run on. the more known
  they are, the simpler your game loop
- running as fast as possible on mobile can harm battery life

## Sample Code

### Ye Olde Game Loope

    main()
      while (notDeadYet)
        readInput()
        updateGame()
        render()

fine in atari days where you knew exact hardware you were running on and could
own entire cpu.

bad these days.

### Fixing the Framerate

    <FixedFramerate>

- if machine is fast enough game runs at reliable speed
- game still runs at right speed if machine is faster
- battery-life friendly
- game slows down if machine is too slow

### Decouple Update and Render Rate

    <FixedUpdateFramerate>

- uses max power
- updates with fixed time step
- can get into spiral of death
- doesn't play more smoothly on better hardware

### Interpolated Rendering

    <FixedUpdateFramerateInterpolated>

## Design Decisions

### How does it adapt to fast hardware?

### How does it adapt to slow hardware?

### Does power consumption or CPU usage matter?

- if the loop has sleep(), battery-friendly. if not, optimizes game experience

## See Also

http://gafferongames.com/game-physics/fix-your-timestep/
http://www.koonsolo.com/news/dewitters-gameloop/
http://en.wikipedia.org/wiki/Game_programming
http://active.tutsplus.com/tutorials/games/understanding-the-game-loop-basix/
http://www.mvps.org/directx/articles/writing_the_game_loop.htm
http://entropyinteractive.com/2011/02/game-engine-design-the-game-loop/
http://www.nuclex.org/articles/3-basics/5-how-a-game-loop-works
