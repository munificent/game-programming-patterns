^title Game Loop
^section Sequencing Patterns

## Intent

*Simulate the smooth progression of time independently of user input
and processor speed.*

## Motivation

If there is one pattern this book couldn't possibly live without, it's this one. When you think "patterns" and "game programming", the game loop is the quintessential example. Almost every game has one, no two are exactly alike, and relatively few programs outside of games use them.

<aside>

Ada Lovelace and Rear Admiral Grace Hopper had beards too.

</aside>

To see what they're useful for, let's take a quick trip down memory lane. In the olden days of computer programming when everyone had beards, programs worked like your washing machine: you put a program in, you got results out. Done. These were *batch mode* programs: once the work was done, the program stopped.

You still see these today, though we thankfully don't have to write them on punch cards any more. Shell scripts, command line programs, and even the little Python script that turns a pile of Markdown into this book are all batch mode programs.

### Let's Talk

But not too long after that, programmers realized they were lonely and wanted to have live conversations with their machines. *Interactive* programs were born. Some of the first interactive programs were games:

<aside>

This is [Colossal Cave Adventure](http://en.wikipedia.org/wiki/Colossal_Cave_Adventure), the first adventure game.

</aside>

    YOU ARE STANDING AT THE END OF A ROAD BEFORE A SMALL BRICK
    BUILDING . AROUND YOU IS A FOREST. A SMALL
    STREAM FLOWS OUT OF THE BUILDING AND DOWN A GULLY.

    > GO IN
    YOU ARE INSIDE A BUILDING, A WELL HOUSE FOR A LARGE SPRING.

<aside>

Many programming languages still have an interactive mode like this.

</aside>

Early interactive programs worked like a dialogue. The program would say something. Then it would patiently wait for your command. Then it would respond. Back and forth. When it was your turn to give it a command, it sat there with infinite patience doing pretty much nothing. Something like:

<aside>

Doesn't the `while (true)` mean it loops forever? How does a player quit the game?

In some real game loops, this would actually be more like `while (!done)` where `done` is some variable that you set to `true` in order to exit the game. Others may just call `exit()` directly. For simplicity's sake, I've left all of that out.

</aside>

^code 1

Compared to batch mode programs, the `while` loop is the big difference here. That's what let programs keep running as long as the user wanted. Now we have something like a primordial "game loop". But things changed a lot when graphic user interfaces came onto the scene shortly after the PC revolution.

### Event Loops

Most applications aren't batch mode any more. A word processor would be pretty pointless if it just exited immediately every time you ran it. So the loop must still be there somewhere, but if you're an application programmer, you rarely see it. Instead, it's handled by the operating system or GUI framework. Somewhere deep in their bowels is something like:


^code 2

These "events" will be user interactions: a button was clicked, the mouse was moved, a key was pressed, you get the idea. In your application code, you wire up *event handlers*. You specify chunks of code that should run when an event occurs.

<aside>

This is a bit of a simplification. Most event loops also have "idle" events you can handle. That's lets you intermittently do stuff when nothing is happening. It's good enough for a blinking cursor in your word processor, or maybe a progress bar, but it's not precise enough for games.

</aside>

If you look though, that loop is pretty much like the old text adventure game loop. It still *blocks* waiting for user input. Your favorite word processor spends most of its time just sitting idle, waiting for you to click or type something.

You *can* write games this way, at least very simple ones. Think Minesweeper or solitaire. But once you start wanting animation, sound and all of the other fun stuff that makes a game a game, that's not going to fly anymore.

### A Living Breathing World

The key difference between games and most other software here is that most games keep doing stuff even when the user isn't providing input. If you just sit staring at the screen, the game doesn't grind to a halt. Animations keep animating. Visual effects dance and sparkle. The music keeps playing.

**TODO: Finish rolling this into this section:**

This is the first key part of a real game loop: it *processes* user input, but it doesn't *wait* for it. The loop always keeps spinning. Event-based code has a *push*-based model for handling user input. For example, if the game character jumps when you press a button, the logic looks like:

1. The OS detects a button press.
2. It calls the event dispatcher.
3. That calls the event handler we've wired up.
4. We make the main character jump.

This has a few problems. First, this could happen at any time. Maybe you're in the middle of rendering the frame or processing enemy AI. This could let the player act out of turn. Second, you don't have any control over the *rate* of these events. If you aren't careful, the player could get more moves in than the enemies just by flooding the game with events.

To avoid this, in games user input is usually processed using a pull model:

1.  The game updates all of the entities.
2.  When it gets to a player-controlled entity, we see if the button is
    currently pressed.
3.  If it is, we make the main character jump.

The difference here is that we respond to the input when it's most convenient for *us*, not when the OS starts yelling at us.

**TODO: Make a segue here.**

### A World Out of Time

<aside>

The common terms for one crank of the game loop are "tick" or "frame".

</aside>

Our game now has a loop that will just keep spinning on its own. Great. But that leads to the obvious question: how *fast* does it loop? Each turn through the game loop, you'll be updating the state of the game world by some amount. From the perspective of an inhabitant of the game world, the clock has sounded one tiny "tick" and they've moved one tiny unit into the future. Game time has advanced.

The game world has its own internal clock, but the *player* also has a sense of time. *Real* time, with clocks and seconds and calendars and sundials. If you think of the turning of the game loop as one axle and the turning of the human player's real clock hands as another, there's some set of gears between them. There's a conversion between game time and real time.

This rate, say "frames per second" determines how fast the game runs. When it's slow, game characters slog through molasses. When it's fast, it turns into a Buster Keaton movie.

There are two gears between the real time and game time clocks that control that rate. The first is *how much work has to be done each frame*. If you've got complex physics, a bunch of game objects, and lots of graphic detail, all of that is going to keep your CPU and GPU busy for a while, and it will take that much longer for the frame to complete. You control this by deciding how complex your game is.

The other gear is *how fast the hardware itself runs.* Faster chips can churn through more work in the same amount of time. Multiple cores, GPUs, dedicated audio hardware, and the OS's scheduler all effect how much you can get done in one tick.

### Seconds Per Second

Early on in video games, that second gear was actually fixed. If you were writing a game for the NES or Apple IIe, you knew *exactly* what CPU your game was running on and you could (and did) code specifically for that. That meant you only had one gear to worry about: how much work you did each tick.

<aside>

This is why old PCs used to have "[turbo](http://en.wikipedia.org/wiki/Turbo_button
)" buttons. New PCs were faster and couldn't play old games because the game would run too fast. Turning the turbo button *off* would slow the machine down and make the game playable.

</aside>

Early videogames were carefully coded to do just enough work each frame so that the game played at the speed that the developers wanted. But if you tried to play that same game on a faster or slower machine, then all of the sudden the game would speed up or slow down.

These days, though, almost no developer has the luxury of knowing exactly what hardware their game will run on. Instead, the game has to intelligently adapt to a wide variety of hardware. This is the other key job of your game loop: it keeps the game playing at a consistent speed despite differences in the underlying hardware.

## The Pattern

A **game loop** runs continuously until the game is exited. Each turn of the
loop, it **processes user input** without blocking, and **updates the game
state**. It keeps track of the passage of time so that it can **control the
rate of gameplay**.

## When to Use It

Using a pattern when it isn't neeeded is often as bad as not using a pattern where it is, so this section is usually useful as a guard against over-enthusiasm. Every game doesn't need every pattern under the sun.

<aside>

For me, this is the difference between an "engine" and a "library".
With libraries you own the main game loop and call into the library.
With engines, it usually has the main loop and calls into your code.

</aside>

But this pattern is a bit different. I can say with pretty good confidence that you probably *will* use this pattern. If you're using a game engine, you probably won't write it yourself, but it's still there.

If you're making a turn-based game, you may think you won't need a full game loop. Instead you can rely on the OS's event loop. You may get away with this for very simple games. But in most turn-based games, even though the *game state* doesn't advance without user input, the *visual* and *audible* state of the game usually does. Idle animation and music keep running even when it's "waiting" for you to take your turn.

In other words, unless you're:

* making a simple turn-based game without animation
* using an engine that does it for you

then odds are you'll have a game loop.

## Keep in Mind

### This is the core loop of your game

<aside>

Made up statistics like this are why "real" engineers like mechanical and
electrical don't take us seriously.

</aside>

Seems obvious, I know, but code in the game loop will be running constantly. They say a program spends 90% of its time in 10% of the code. Your game loop and the stuff it calls directly will be in that 10%. Think carefully about performance and make sure you profile when you make changes to your core loop.

### You may need to coordinate with the platform's event loop

If you're building your game on top of an OS or platform that has a graphic UI and event loop built in, then you have *two* application loops in play. You'll need to coordinate them.

Sometimes, you can just take control and make your loop the main one. For example, if you're writing a game against the venerable Windows API, your `main()` can just have a game loop. Inside, you can call `PeekMessage()` to also handle and dispatch events from the OS. Unlike `GetMessage()`, `PeekMessage()` doesn't block waiting for user input so your game loop will keep cranking.

Other platforms don't let you opt out of the event loop as easily. If you're writing a game that runs in a web browser, the event loop is deeply built into browser's execution model. There, the event loop will be your one loop and you'll use it as your game loop too. You'll call something like `requestAnimationFrame()` to let the event loop call your every tick.

### Think about the *range* of handware your game runs on

**TODO: Move to design decisions?**

As we'll see below, you can code a game loop that adapts to different hardware speeds, but it can only adapt so far. If your game needs to run on an 8MHz 8086, no amount of clever coding in your game loop is going to get a detailed 3D fluid simulation running in realtime.

<aside>

Somewhere, right now, some crazy German demoscene hacker is proving me wrong on this.

</aside>

The design of your game itself will need to take into account the range of hardware you plan to run on. This will affect all aspects of the design, everything from how detailed your graphics are, to how the physics work, to how big levels are and how many characters there are on them.

You'll need to think about it coming from both directions too. You'll have to answer both "how does my game run on the worst hardware?" *and* "how does it run on the best?" If you only worry about the first, you'll end up with a game that looks cheap and unimpressive on powerful gear. If you only worry about the second, users with slower machines may find the game unusable.

This chapter will just be about the game loop itself and how it adapts to the game's running speed, but there are also often other places in the game that can and will need to adapt to the hardware. Things like lowering graphic detail, reducing the number of visual effects, etc. all help you get your game on as many devices as possible, but are out of scope here.

### You may need to think about battery life

This section wouldn't have been here five years ago. Games ran on things
plugged into walls. But with the advent of smart phones, laptops, mobile gaming,  the odds are good you do care about this now. A game that runs beautifully but turns your phone into a space heater that runs out of power in thirty minutes is not a game that makes people happy.

Now you may need to think about making your game look great, but also use as little CPU as possible. There will likely be an *upper* bound to performance where you let the CPU sleep if you've done all the work you need to do in a frame.

On the other hand, if you're making a console or PC game, you often want to cram
as much scale and cinematic drama into your game as you can. An idle CPU means
a wasted opportunity to make a game bigger, better, and more exciting than the
competition.

## Sample Code

For all this setup, the code for a game loop is actually pretty straightforward. We'll walk through a couple of different variations and go over their good and bad points. Since the point of a game loop is just to handle timing, the sample code here will call into a bunch of over fictitious methods that aren't presented here. Just imagine your engine's actual functionality there.

### Run Run As Fast You Can

Let's start with the simplest possible game loop:

^code 3

This is what your loop looked like if you were writing a Commodore 64 game, but it's not too far removed from modern game loops. Three key pieces are there already.

First, it polls user input without blocking. Then it calls `update()`. This advances the game simulation one step. It runs AI and physics (usually in that order) and handles any player-controlled characters. We do this *after* `pollInput()` so that we can have the game respond to user input in the same frame in which it was received. Finally, we call `render()` to draw the game and show the new game state to the user.

This wins the simplicity battle, but that's about all it wins. The main problem here is you have little control over how fast the game runs. On a fast machine, that loop will spin so fast users won't be able to see what's going on. On a slow machine, the game will crawl. If you have a part of the game that's content-heavy or does more AI or physics, the game will actually play slower there.

I'm showing you this code just to give you some context, but this is rarely what you want.

### Draw and Wait

There's a simple fix for this problem:

  1. Store the current real-world time.

  2. Do all of your work.

  3. Wait until it's time for the next frame.

<aside>

    1000 ms / FPS = ms per frame

</aside>

Say you want your game to run at 60 FPS. That gives you about 16 milliseconds per frame. As long as you can reliably do all of your game processing and rendering in less than that time, you can run at a steady frame rate. It looks a bit like this:

^code 4

The `sleep()` here will make sure the game doesn't run too *fast* if it takes less time to process the frame. That solves half the problem of the previous "run as fast as you can" loop. It's also a bit more battery-friendly, since it gives control back to the OS every time it calls `sleep()`.

What it *doesn't* help with your game running too *slowly*. If it takes longer than 16 ms to update and render the game, your sleep time will be *negative*. If we had computers that could travel back in time, lots of things would be easier, but we don't.

**TODO: talk about spiral of death**

Instead, the game will just start to slow down. A crude fix is to just do less work each frame: cut down on the graphics and razzle dazzle, or dumb down the AI. But that impacts the quality of gameplay for all users, even ones on fast machines. If you're making a relatively simple game, you may not have this problem, and the above loop might be fine. Otherwise, we'll have to do something a bit more complex...

### Fluid Time Step

Let's try something a bit more sophisticated. The problem we have basically boils down do:

  1. Each frame, we update the game by a fixed amount of time.

  2. It takes a certain amount of real time to do that update.

  3. If step two takes longer than step one, the game slows down.

One obvious fix then, is to not fix the game time step when we update. If it takes longer than 16 ms to process a game frame, then the game can't keep up it if only advances 16 ms of *game* time each frame.

But if each update advances a larger chunk of game time, we can update the game less frequently and it will still keep up. They call this a *variable* or *fluid* time step. It looks like:

^code 5

Each frame, we keep determine how much *real* time passed since the last game update (`elapsed`). When we update the game state, we pass that in. The engine is then responsible for advancing the game world forward by that amount of time.

For example, if a bullet is shooting across the screen, instead of moving it by a fixed distance, the engine will scale that by the elapsed time. As the frame rate goes down, the game engine moves things in larger increments to compensate. That bullet will get across the screen in the *same* amount of *real* time, even if it's ten or twenty ticks of game time.

On its surface, it seems like this accomplishes our goals:

 *  The game plays at a consistent rate on different hardware.
 *  Players with faster machines are rewarded with smoother gameplay.

This is better, but still not good. The main problem is that making each frame of game time based on the system clock makes the game engine unpredictable, non-deterministic, and often leads to really nasty bugs.

Here's a couple of examples of what can go wrong. Let's say we've got a two-player networked game and Fred has some beast of a gaming machine while George is using his grandmother's antique PC. We've got this little bullet flying across both of their screens. On Fred's machine, the game is running super fast, so each time step is tiny. We cram like 50 game ticks in the second it takes the bullet to cross the screen. Poor George can only fit about 5 frames in.

This means the physics engine is going to update the bullet's position 50 times for Fred but only 5 times for George. Most games are using floating point numbers for this, and those numbers are *approximate* and subject to rounding error. Each time you add two floating point numbers, the answer you get back can be a bit off. Fred's machine is doing ten times as many operations, so he'll accumulate a bigger error than George. This means *their bullets will end up in different places*. Ouch!

It gets worse. On George's machine, the bullet only has five frames to get across the screen, so each frame it's moving a pretty large distance. If a wall is narrow enough, it could be on one side in one frame and all the way through it on the next. On Fred's machine, the bullet will stop at the wall, but on George's, it passes right through.

<aside>

This problem is called *tunnelling.* Physics engines that do *swept collision* can handle this, but are slower and more complex to implement. Trade-offs, trade-offs, trade-offs.

</aside>

That's a particularly obvious examples of physics going haywire, but more subtle problems will crop of too if you have a variable time step. Game physics is a delicate art of emulating reality within the confines of a system that uses approximations for efficiency. These systems are carefully tuned and damped to avoid blowing up (think objects literally launching themselves in the air with completely crazy velocities) when errors in the approximations make the underlying equations goes sideways. That tuning is based in part on how much time it expects to elapse each frame. Change that, and your careful damping can fail to keep the system stable.

I'm harping on physics here, but variable time steps are complex for other aspects of the game engine. Imagine writing AI code for an enemy aiming a gun and trying to take into account how far ahead of a moving target it needs to aim when you aren't even sure what point in time it will be in the next frame.

The one part of the engine that usually isn't affected much by this is rendering. Since the rendering engine (more or less) just captures an instant in time, it usually doesn't care too much about how much time has advanced each frame. As long as the physics and AI systems put everything in the right place, it will render them wherever they are now.

### Decouple Update and Render Rate

We can use this fact to our advantage. We'll update the game using a fixed time step because that makes everything simpler and more stable for physics and AI.

Think about it like this: At the beginning of the game loop, a certain amount of real time has elapsed since the last turn. This is how much game time we need to simulate in order to for the game's "now" to catch up with the user's "now". Once it's caught up, we render and show it to the user. The code looks a bit like:

^code 6

There's a few pieces here:

^code 7

At the beginning of each frame, we calculate how much real time passed. We add this to `lag`. That measures how far behind in time the game is compared to the real world.

^code 8

We keep updating the game one fixed chunk at a time until we're as close to being caught up as we can be. Note that the time step here isn't the *visible* frame rate any more. `MS_PER_UPDATE` is just the *granularity* we use to update the game. We can render both faster or slower than that.

You'll tune this for your game. The shorter this step is, the more processing time it will take to catch up to real time. The longer it is, the choppier the gameplay is. Ideally, you want it pretty short, often faster than 60 FPS, so that the game simulates with high fidelity on fast machines.

But be careful not to make it *too* short. You need to make sure the time step is greater than the time it takes to process an `update()`, even on the slowest hardware. Otherwise, your game simply can't catch up no matter how hard it tries.

Fortunately, this is usually pretty easy. The trick is that we've yanked *rendering* out of that loop. That frees up a bunch of CPU time.

Once the game's time is caught up to the player's, we finally ready to render it.

^code 9

Then we start it all over again. This is certainly more complex, but it's a big improvement over out simpler loops. We've got a fixed time step which makes the game state deterministic and predictable. The visible framerate goes up on fast machines, but on slow machines the game still plays at the right speed.

### Interpolated Rendering

There's one issue we're left with, and that's residual lag. We update the game at a fixed time step, but we *render* at arbitrary points in time. This means that from the user's perspective, the game will often display at a point in time in the between two updates.

Here's a timeline:

      update   update   update   update   update   update   update   update
         |        |        |        |        |        |        |        |
    ------------------------------------------------------------------------>
         |                 |             |         |                  |
       render            render        render    render             render


As you can see, we update at a nice tight fixed interval. Meanwhile, we render
whenever we can. It's less frequent than updating, and isn't steady either.
Both of those are OK. The nasty part is that we don't always render right at the
point of updating. Look at the third render time. It's right between two updates:

       update   update
          |        |
    ...----------------
               |
             render

Imagine a bullet is flying across the screen. On the first update, it's on the left side. The second update moves it to the right side. The game is rendered at a point in time right between those two updates, so the user will expect to see that bullet right in the middle of the screen. With our current implementation, it will still just be on the left side. The end result is that motion will look jagged or stuttery.

At the point that we're rendering, we actually know *exactly* how far between update frames we are: it's stored in `lag`. We bail out of the update loop when it's less than the update time step, but not when it's *zero*. That leftover amount? That's how far into the next frame we are.

When we go to render, we'll tell it how far into the next frame we are:

^code 10

The renderer knows each game object *and its current velocity*. Say that bullet is at (20, 300) and is moving right at 400 pixels per frame. If we are halfway between frames, then we'll end up passing 0.5 to `render()`. So it draws the bullet half a frame ahead, at (220, 300). Ta-da, smooth motion.

Of course, it may turn out that that interpolation is wrong. When we actually calculate the next frame, we may discover the bullet hit an obstacle or slowed down or something. We rendered its position interpolated between where it was on the last frame and where we *think* it will be on the next frame. But we don't know that until we've actually done the full update with physics and AI.

So the interpolation is a bit of a guess and sometimes ends up wrong. Fortunately, though, those kinds of corrections aren't usually noticeable by a player. At least, they're less noticeably than the stuttering you get if you don't interpolate at all.

## Design Decisions

### Do you own the game loop, or does the platform?

The first decision you'll make is whether or not you explicitly have a game loop. Actually, it's probably clearer to say the first decision that will be made for you. If you're writing a game to run on a browser, you pretty much *can't* write your own classic game loop. The browser's event-based nature precludes it. Likewise, if you're using an existing game engine, you will probably rely on its game loop instead of rolling your own.

* **Use the platform's event loop:**

    * *It's simple.* You don't have to worry about writing and optimizing the
        core loop of the game.

    * *It plays nice with the platform.* Since the platform was designed with
      this in mind, it usually interfaces nicely with it. You don't have to
      worry about explicitly giving the host time to process its own events.

    * *You lose control over timing.* The platform will call your code as it
      sees fit. If that's not as frequently, or as smoothly as you'd like,
      too bad. Worse, most application event loops weren't designed with games
      in mind and usually *are* slow and choppy.

* **Use a game engine's event loop:**

    * *You don't have to write it.* Writing a game loop can get pretty tricky,
      especially if things like threading come into play. Since that core code
      gets executed every frame, minor bugs or performance problems can have a
      large impact on your game. Having a solid event loop is one of the reasons
      to consider using an existing engine.

    * *You don't get to write it.* Of course, the flip side to that coin is the
      loss of control if you *do* have needs that aren't a perfect fit for the
      engine.

* **Write it yourself:**

    * *Total control.* You can do whatever you want with it. You can design it
      specifically for the needs of your game.

    * *Have to interact with the platform yourself.* Application frameworks and
      operating systems usually expect to have a slice of time to process events
      and do other work periodically. If you own your app's core loop, it won't
      get any. You'll have to explicitly hand off control periodically to make
      sure the framework doesn't hang or get confused.

### How does it adapt to different hardware?

<aside>

Game-making seems to be part of human nature, because every time we've built a
machine that can do computing, one of the first things we've done is made games
on it. The PDP-1 was a 2 kHz machine with only 4,096 words of memory yet Steve Russell and co. managed to create Spacewar! on it.

</aside>

A game loop has two key pieces: non-blocking user input, and adapting to the
passage of time. The former is usually pretty straightforward. Most of the magic
is in how you deal with time. There are a near infinite number of platforms that
games can run on and any single game may run on quite a few. How it adapts to that is key.

* **Fixed time step with no synchronization:**

    This was our first sample code. You just run the game loop as fast as you
    can.

    *   *It's simple*. This is its main (well, only) virtue.

    *   *Game speed is directly affected by hardware and game complexity.* And
        its main vice is that if there's any variation, it will directly affect
        the game speed.

* **Fixed time step with synchronization:**

    The next step up on the complexity ladder is running the game at a fixed
    time step, but adding a delay or synchronization point at the end of the
    loop to keep the game from running too fast.

    *   *Still quite simple.* It's only one line of code more than the
        probably-too-simple-too-actually-work example. In most game loops, you
        will likely do synchronization *anyway*. You will probably [double
        buffer](double-buffer.html) your graphics and synchronize the buffer
        flip to the refresh rate of the display.

    *   *It's power-friendly.* This is a surprisingly important consideration
        for mobile games. You don't want to kill the user's battery
        unnecessarily. By simply sleeping for a few milliseconds instead of
        trying to cram ever more processing into each tick, you let the CPU idle
        a bit which saves power.

    *   *The game doesn't play too fast.* This fixes half of the speed concerns
        of a fixed loop.

    *   *The game can play too slow.* If it takes too long to update and render
        a game frame, playback will slow down. Because this style doesn't
        separate updating from rendering, it's likely to hit this sooner than
        more advanced options. Instead of just dropping *rendering* frames to
        catch up, this will just slow down.

* **Variable time step:**

    I'll put this in here as an option in the solution space, with the caveat
    that most game developers I see recommend against doing it. Still, it's good
    to keep track of *why* it's a bad idea, lest we forget.

    *   *Adapts to playing both too slowly and too fast.* If the game can't
        keep up with real time, it will just take larger and larger time steps
        until it does.

    *   *Makes gameplay non-deterministic and unstable.* And this is the is the
        real problem, of course. Physics and networking in particular become
        much harder with a variable time step.

* **Fixed update time step, variable rendering:**

    The last option we covered in the sample code is the most complex but also
    the most adaptable. It updates with a fixed time step, but can drop
    *rendering* frames if it needs to to catch up to the player's clock.

    *   *Adapts to playing both too slowly and too fast.* As long as the game
        can *update* in real time, the game won't fall behind. If the player's
        machine is top-of-the-line, it will respond with a smoother gameplay
        experience.

    *   *It's more complex.* The main downside is there is a bit more going on
        in the implementation. You have to tune the update time step to be both
        as small as possible for the high-end, while not being too slow on the
        low end.

    *   *Updating can starve the renderer.* This style only renders once the
        game state is caught up to the player's time. The update time step tends
        to be smaller than your game's actual framerate on mid-level hardware.
        The smaller the time step, though, the more time the game needs to spend
        updating and the less time it has to render. The end result can be even
        more dropped frames on low-end machines than you'd get with a simpler
        loop and a longer update time step.

## See Also

* The classic article on game loops if Glenn Fiedler's [Fix Your Timestep](http://gafferongames.com/game-physics/fix-your-timestep/). This chapter wouldn't
    be the same without it.

* Witters' article on [game loops](http://www.koonsolo.com/news/dewitters-gameloop/) is a close runner-up.
