^title Dirty Flag
^section Optimizing Patterns

## Intent

*Avoid unneeded work by deferring it until the result is needed.*

## Motivation

Let's talk about names. There's two common monikers often slapped on this pattern: "dirty flag" and "dirty bit". The latter is a more common term, but often has a <span name="specific">more specific meaning</span>. It also makes me giggle because I have the sense of humor of a ten-year-old. So I went with the former, despite massive temptation otherwise.

<aside name="specific">

In particular, the self-same [Wikipedia article](http://en.wikipedia.org/wiki/Dirty_bit) uses the term to describe an OS-level application of this pattern where it uses a dirty bit to tell which memory pages are out of sync with the disk.

</aside>

Where were we? Oh, right, making a game. Most games or game engines have something called a *scene graph*. This is a big data structure that contains all of the visible objects in the world. It's used by the renderer to determine where to draw stuff on screen. You commonly hear it associated with 3D games, but 2D games often have something similar.

At its simplest, a scene graph is just a pile of objects. Each object has a model or sprite some other basic visual component, and a <span name="transform">*transform*</span>. The transform describes the object's position, rotation, and scale in the world. To move an object around in the world, we just have to change the transform.

<aside name="transform">

The mechanics of *how* this transform is stored and manipulated is unfortunately out of scope for this chapter. The comically abbreviated summary is that in a 3D game, the transform is 4x4 matrix. Applying one transform onto another (for example translating and then rotating an object) just requires doing a matrix multiple on the the matrix for each transform.

A proof of the correctness of that is left as an exercise for the reader.

</aside>

When the renderer goes to draw an object, it takes its geometry, applies the transform to get it to the right place in the world, and then renders it there. If we just had a scene *bag* and not a scene *graph* that would be it and life would be simple. However, most scene graphs are <span name="hierarchical">*hierarchical*</span>.

An object in the graph may have a *parent* object that it is anchored to. In that case, its transform is relative to the paren't position, and isn't its absolute position in the world.

For example, imagine your game world has a pirate ship at sea. On the pirate ship is an object for the crow's nest. In that crow's nest is an object for the pirate. Attached to the pirate is a parrot. The ship's local transform will position it in the sea. The crow's nest transform position it on the ship, and pirate's positions it in the nest, etc.

**TODO illustrate**

This way, when a parent object moves, its children move along with it. If we change the local transform of the ship, the crow's nest, pirate, and parrot all ride along with it. It would be a total headache if we had to manually adjust transforms of everything on the ship when it moved to keep everything from sliding off of it.

But to actually draw the parrot on screen, we need to know its absolute transform in world coordinates. To clarify thing, we'll call the transform we already mentioned the object's *local transform*. That's the one that's relative to its parent. To render an object, we need to know its *world transform*.

Calculating an object's world transform is pretty straightforward: you just walk its parent chain starting at the root all the way down to the object, concatenating transforms as you go. In other worlds, the parrot's world transform is:

    ship local transform * nest local transform * pirate local transform * parrot local transform

In the degenerate case where the object is at the top of the scene graph and has no parent, that means its local and world transforms are equivalent. There's nothing magical here. Concatenating two transforms is just a matrix multiply, which is just a handful of floating point arithmetic. However, we do have to calculate the world transform of every object in the world every frame, so this code is definitely on the hot path where performance is critical.

Where it gets tricky is when an object moves. The parrot's world coordinates are based on the local coordinates of its entire parent chain. That means if any of those local transforms change, the parrot's world transform will change too. Every time a parent moves all of its children move too, recursively.

The simplest approach to handle that fact is to just calculate world transforms on fly when we render. Each frame, we recursively traverse the scene graph starting at the top of the hierarchy. For each object, we calculate its world coordinate right then and render the object.

But this is terribly wasteful of our precious CPU juice! Many if not most of the objects in the world are *not* moving every frame. Think of all of the static geometry that makes up the level. Calculating their world coordinates each frame is a waste when it's the same every time.

The obvious answer is to *cache* the world transform. In each object, we'll store its local transform and the world transform that's derived from it and its parents. When we render, we just use the precalculated world transform. If the object never moves, that's always up to date and everything's happy.

When an object does move, the obvious approach is to just recalculate its world transform right then. But don't forget the hierarchy! If a parent moves, we'll have to recalculate its world transform *and all of its children's, recursively*.

Now imagine some busy gameplay. In a single frame, the ship gets tossed on the ocean, the crow's nest rocks in the wind, the pirate leans to the edge, and the parrots hops onto his head. In this one frame, we've changed four local transforms. With our current approach of eagerly recalculating world transforms, why ends up happening?

    1. update ship local transform
    2. calculate ship world transform
    3. calculate nest world transform
    4. calculate pirate world transform
    5. calculate parrot world transform
    6. update nest local transform
    7. calculate nest world transform
    8. calculate pirate world transform
    9. calculate parrot world transform
    10. update pirate local transform
    11. calculate pirate world transform
    12. calculate parrot world transform
    13. update parrot local transform
    14. calculate parrot world transform

We only have four objects, but we end up doing *ten* world transform calculations, each of which is itself an iterative walk up the parent chain. Notice that most of those world calculations are completely pointless. We end up calculating the parrot's world transform *four* times, but the first three just get discarded and overridden by later ones. This won't do.

The problem is that when a single object moves, it invalidates the world transform of several objects: itself and all of its children. Flipping that around, it means any object's world transform is dependent on several local transforms. Since we recalculate immediately each time one of those local transforms changes, when a number of local transforms change in one frame, we end up calculating the world transform for the same object multiple times.

The solution is to decouple changing local transforms from updating the world transforms. This would let us change a bunch of local transforms in a single batch and then recalculate the affected world transform *once* when all of those modifications are done, right before we need it to render.

We do this by adding a boolean field to each node in the graph. When the local transform changes, we set this flag to true. When we need the object's world transform to render, we check the flag first. If it's set, we calculate the world transform then and clear the flag. In other words, the flag represents "is the world transform out of date". For reasons that aren't entirely clear, the traditional name for this "out-of-dateness" is "dirty". Hence: a dirty flag.

Applying that pattern to our earlier example where everything moves in the same frame yields:

    1. update ship local transform
    2. update nest local transform
    3. update pirate local transform
    4. update parrot local transform
    5. calculate ship world transform
    6. calculate nest world transform
    7. calculate pirate world transform
    8. calculate parrot world transform

Which is obviously the best you could hope to do. As you can see, the pattern gives us a few advantages:

1. It collapses modifications to multiple local transforms along an object's parent chain into a single recalculation on the object.
2. It avoids doing any recalculation on objects that didn't move.
3. And a minor bonus: if any objects end up getting removed before they are rendered, we won't bother calculating their world transform at all.

In other words, this pattern lets us only calculate the world transform when it's actually needed and when it's actually changed.

## The Pattern

A set of **primary data changes** over time. A set of **derived data** is determined from this using some **time-consuming process**. A **"dirty" flag** tracks when the derived data is out of sync with the primary data. It is **set when the primary data changes**. When the derived data is requested, if the flag is set **the processing is done then and the flag is cleared.** Otherwise, the previous **cached derived data** is used.

## When to Use It

Compared to some other patterns in this book, this one solves a pretty specific problem. Also, like most optimization patterns, you should only reach for it when you have a performance problem big enough to justify the added code complexity. Added up, this means you probably won't use this often.

Dirty flags are used to solve two related problems: *calculation* and *synchronization*. What they have in common is:

1. A set of data that changes over time.
2. Another set of data that's somehow derived from that.
3. The process of going from 1 to 2 is time-consuming or otherwise expensive.

In our scene graph example, the process is time-consuming because there's just a lot of math to perform. When using this pattern for synchronization, it's more often that the derived data is *somewhere else* -- either on disk or over the network on another machine -- and simply getting it from point A to point B is what's expensive.

In either case, it turns out that doing that heavyweight work is often for naught. Every time the primary data changes, you have to ditch the previous derived data and generate it again to take into account the new stuff. If the primary data changes *again* before you ever actually use the data derived from it, then there was no point in doing that work.

That brings in another requirement for this pattern: *the primary data has to change more often than the derived data is used*. If you always need the derived data every single time the primary data is modified, there's no way to avoid doing the work to generate it each time, and this pattern can't help you.

There's one final softer requirement. It's implied here that you can't easily *incrementally* update the derived data when the primary data changes. This pattern comes into play with the derived data is only calculated from *the primary data* and not from *the previous derived data*.

Let's say you had a game where the character has a backpack and can only carry so much weight. You need to know the total weight by summing the weights of everything in the backpack. You *could* use this pattern and have a dirty flag for the total weight. Every time you add or remove an item you set the flag. Then when you need the total, you add up all of the items and clear the flag.

But a simpler solution is to just *keep a running total*. When you add and remove an item, just add or remove its weight from the current total. If you can "pay as you go" like this and keep the derived data updated, then that's often a better choice than using this pattern.

All of this makes it sound like this pattern is never actually useful, but over time you'll likely find a place here or there where it's a good fit. Grepping your average game codebase for the word "dirty" will often turn something up, and it almost always refers to this pattern.

## Keep in Mind

Once you have convinced yourself this pattern is a good fit, there are a few wrinkles that can cause you some discomfort.

### Deferring until the result is needed can cause a noticeable pause

In its simplest form, this pattern <aside name="gc">defers</aside> some slow work until the result is actually needed. But when the result *is* needed, it's often needed *right now* and the reason we're using this pattern to begin with is because calculating that result is slow!

This isn't a problem in our example because calculating a given node's world coordinates isn't *too* slow. Our goal was to avoid doing it *redundantly*.
But you can imagine other uses of this pattern where the work you're doing is a big monolithic chunk that takes a noticeable amount of time to chew through.

If the game doesn't *start* doing that until right when the player brings up
the screen the shows the result of it, then they will notice the pause.
If this is a problem, you may need to start doing the processing earlier "in
the background" so that at least some of the work is done by the time its
needed.

### You have to make sure to set the flag *every* time the state changes

Since the derived data is calculated from the primary data, it's essentially a cache. Whenever you have cached data in memory, the trickiest aspect of it is <span name="cache">*cache invalidation*</span> -- correctly knowing when the cache is out of sync with its source data. In this pattern, that means correctly setting the dirty flag when *any* primary data changes.

<aside name="cache">

Phil Karlton famously said, "There are only two hard things in Computer Science: cache invalidation and naming things."

</aside>

Miss it in one place, and your program will incorrectly use stale derived data. This leads to confused players and very hard to track down bugs. When you use this pattern, you'll have to be very careful that any code that modifies the primary state also sets the dirty flag. In our scene graph, for example, we have to remember to invalidate not just the object's world transform when its local transform changes, but all of its children too since they depend on it.

One way to mitigate this is by encapsulating modifications to the primary data behind some interface. If anything that can change the state goes a single narrow interface, you can set the dirty bit there and rest assurred that it won't go out of sync.

### You will have to keep the previous derived data in memory

When the game needs the derived data, it checks to see if the dirty flag is set. If it *isn't* set, then it uses the previously calculated data. This is obvious, but that does imply that you have to keep that derived data around in memory in case you end up needing it later.

If you weren't using this pattern, another option would be just calculate the derived data on the fly right when you need it, and then discard it when you're done. That avoids the expense of keeping it cached in memory. The cost doing this is calculation: you'll have to do that calculation *every* time you need it.

Like many optimizations, then, you can look at this pattern as <span name="trade">trading</span> off memory for space. In return for keeping the previously calculated data in memory, you avoid having to recalculate it when it's imput data hasn't changed. This trade-off makes sense when the calculation is slow and memory is cheap. When you've got more time than memory on your hands, it may be better to just calculate it as needed and avoid this pattern.

<aside name="trade">

Conversely, you can consider compression algorithsm as making the opposite trade-off: they optimize space at the expense of needing processing time to decompress.

</aside>

### If you're synchronizing and you defer too long, you risk losing changes

Any program that lets you edit documents keeps track of if you have "unsaved changes", which is a picture-perfect example of this pattern. That little bullet or star in your document's title bar is literally the dirty flag visualized. The primary data is the open document in memory, and the derived data is the file on disk.

Many programs don't save to disk until either the document is closed or the application is exited. That's fine most of the time, but if you accidentally kick the power cable out, there goes your masterpiece.

There's actually a continuum of when to do the work ranging from "the second the primary data changes" all the way to "only at the last second when the derived data is needed". In between those two points are a range of options where you defer the work *somewhat* but still eventually kick it off even before the result is actually needed.

When your fancy text editor auto-saves a backup "in the background" every few minutes, that's basically the trade-off it's making. The frequency that it auto-saves -- every few minutes to once an hour -- is picking a point on the continuum that balances not losing too much work when a crash occurs against not thrashing the file system too much by saving all the time.

<span name="gc">

This is mirrors the different garbage collection strategies in systems that automatically manage memory. Reference counting systems free memory the second its no longer needed, but suffer by spending CPU time updating ref counts eagerly every time references are changed.

Simple garbage collectors defer reclaiming memory until it's really needed. But the cost here is the dreaded "GC pause" that can freeze your entire app until the GC is done walking the heap.

In between the two are increasingly common more complex systems like deferred ref-counting and incremental GC that reclaim memory less eagerly than pure ref-counting but more eagerly than stop-the-world collectors.

</span>

## Sample Code

**TODO: redo**

<!--
Assuming we've met the surprisingly long list of requirements and this pattern does make sense for out problem, let's see how it looks to code it up. The initial problem in the motivation section is pretty obvious to implement. Whenever the mayor gets a new trophy, we set the flag. When the player opens the trophy screen, we check the flag. If it's set, we layout the trophies, clear the flag and continue.

I'm pretty sure you can code that up on your own, so let's leave that as an exercise for the reader. Instead, let's do a different example showing the other main use for this pattern, synchronization. The mayor is doing the best he can, but even he needs help saving his city. <span name="pun">To disk</span>, that is.

<aside name="pun">

If you're reading this awful pun, that means I managed to maintain the bad judgement to keep this in through each revision of this chapter. What was I thinking?

</aside>

Our player is going to spend hours meticulously crafting their city. They probably don't want to just throw it out the window and start over from scratch every time they quit the game, so we'll want to whip up some kind of save-load system.

Obviously, saving the city to disk every time they plant a single mailbox or lightpost would be numbingly slow and cause undo torture to the player's hard disk. A better solution is to save when they quit the game.

<span name="elide"></span>

    void Game::quit()
    {
      city.save();
    }

<aside name="elide">

Like many chapters in this book, I'm eliding the concrete code to actually save the city. Since we're just concerned with the architectural structure here, the implementation details are, well, implementation details.

</aside>

Swell. Now every time the player quits the game, they're beautiful metropolis is lovingly enscribed on the platters of their hard disk, ready and waiting for the next time the game is run. Alas, this isn't a perfect solution.

### Wasting time in a clean city

The problem is that in our awesome game, you can build huge cities. Writing out every street and telephone pole takes a long time. When they player's actually built lots of new stuff, there's not much you can do. But if they just open the city, look around a bit and quit, it's pointless to save the city: nothing's changed and the city on disk is already up to date.

We'll apply this pattern and define a dirty flag for the city:

    class City {
      // Lots of other stuff...
    private:
      bool dirty;
    }

Now when we go to save, we'll first check and see if we have to:

    void City::save()
    {
      // Bail if the city is already up to date.
      if (!dirty) return;

      // Write city to disk...

      // The city on disk is up to date now.
      dirty = false;
    }

### Getting dirty

This is the easy part. The hard part is making sure that flag gets set. Every bit of code that modifies a part of the city's state that is saved to disk needs to make sure to set dirty to true. You can imagine lots of methods like:

    void City::zoneArea(int x, int y, int width, int height) {
      // Set zoning...
      dirty = true;
    }

    void City::placeBuilding(int x, int y, Building building) {
      // Place building...
      dirty = true;
    }

    void City::addRoad(int x, int y, Direction direction) {
      // Add road...
      dirty = true;
    }

If you have a <span name="wide">wide smear</span> of code that modifies state, you're just asking to make a mistake and forget to set the dirty flag somewhere. If you can, it helps to define that code in terms of a much smaller interface that does the low-level modification.

<aside name="wide">

In general, code that *modifies* state is more problematic than code that just *reads* state. Whenever you're trying to understand a chunk of code, you have to think about what state the data is in, how it got that way, and how it can change. When other random code reads that state, it doesn't affect your local understanding of what that code does. But when outside code can *change* the state, now you have to hold that much larger context in your head.

</aside>

For example, in our city-building game, the city is ultimately represented by a grid of tiles. Roads, buildings, and other stuff are all just different kinds of tiles. Given that, we could define a low-level method like:

    void City::setTile(int x, int y, Tile tile)
    {
      // Change tile data...
      dirty = true;
    }

This method makes sure to set the dirty flag. Then the previous high level modification methods can be implemented using it:

    void City::zoneArea(int left, int top, int width, int height)
    {
      for (int x = left; x < left + width; x++)
      {
        for (int y = top; y < top + height; y++)
        {
          setTile(x, y, TILE_ZONE);
        }
      }
    }

    void City::placeBuilding(int x, int y, Building building)
    {
      setTile(x, y, building.tile());
    }

    void City::addRoad(int x, int y, Direction direction)
    {
      Tile tile;
      if (direction == DIR_HORIZONTAL)
      {
        tile = TILE_HOR_ROAD;
      }
      else
      {
        tile = TILE_VERT_ROAD;
      }

      setTile(x, y, tile);
    }

Notice that now none of these methods have to worry about the dirty flag. The more we can push setting the dirty flag into lower-level code, the fewer places we'll have to worry about it, and the less likely we are to forget.

### Dirty parts of town

We're in a pretty good place now, but our game is a bit old school. Why save to *disk* when you can save to the *cloud*. That way, the player can seamlessly play in the same city on their computer, phone and tablet. Exciting!

But this means saving they're changes will take even longer: we'll have to push all of the data for the city over the network. Not only is it slow, it's a waste of bandwidth. Users on limited data plans won't be happy when our game burns through their allotment.

It's time to start thinking about the *granularity* of our dirty state. Right now, we have a single bit for the entire city. It's either completely out of date, or completely up to date. But in practice, *most* of the city is unchanged and only a couple of pieces have been modified.

We can do something more sophisticated by having finer-grained dirty bits. We'll divide the city into *blocks* -- regions of some fixed size -- and associated a dirty flag with each block. Something along the lines of:

    void City::setTile(int x, int y, Tile tile)
    {
      // Change tile data...

      // Mark this block dirty.
      setDirtyFlag(x / BLOCK_SIZE, y / BLOCK_SIZE);
    }

Internally, `City` will keep some kind of array of dirty bits, one for each block. When we go to save the city to the game servers, we just send the dirty blocks:

    void City::save()
    {
      for (int x = 0; x < CITY_SIZE / BLOCK_SIZE; x++)
      {
        for (int y = 0; y < CITY_SIZE / BLOCK_SIZE; y++)
        {
          if (isDirty(x, y))
          {
            // Upload block (x, y) to server...
            clearDirtyFlag(x, y);
          }
        }
      }
    }

There's a small amount of overhead to doing things this way. Because we're only saving pieces of the city, we have to send along a bit of metadata with each block to identify it. That way the server knows which pieces of the city its getting.

The trick then is to tune our granularity. If we make the blocks too small, this additional metadata will add more overhead than the savings we got from not having send the whole city. On the other hand, if we make the blocks too big, we end up sending larger amounts of unchanged data. Like all optimizations, we'll have to tune this based on some empirical data for our specific game.
-->

## Design Decisions

**TODO: revise after changing motivation**

This is a pretty concrete pattern, so it isn't that open-ended. There are only a couple of things to tune with it:

### When is the dirty flag cleaned?

The most basic question you'll have to answer is when to actually do the work you've deferred. You can't defer it *forever*, after all. Here's a few options from least to most eager.

* **If you defer it until the result is needed:**

    * *It avoids doing calculation entirely if the result is never used.* For
        primary data that changes frequently and where the derived data is
        rarely accessed, this can be a huge win.

    * *If the calculation is time-consuming, it can cause a noticeable pause.*
        Postponing the work until the end-user is waiting to see the result can
        affect their gameplay experience. Often, it's fast enough that this
        isn't a problem, but if it is, you'll have to do the work earlier.

* **At well-defined checkpoints:**

    Sometimes there is a point in time or the progression of the game where it's
    natural to do the deferred synchronization or calculation. For example, you
    may want to delay saving the game until the player reaches some kind of
    check point in the level.

    These synchronization points may not be user visible or part of the game
    mechanics. For example, maybe there's a loading screen or a cut scene that
    you can hide the work behind.

    * *You can ensure the time spent doing the work doesn't impact the user
        experience.* Unlike the above option, you can often give something to
        distract the player while the game is busy on other things.

    * *You lose control of when the work actually happens.* This is sort of the
        opposite of the above point. You have micro-scale control over when the
        work happens, and can make sure the game handles it gracefully.

        What you *can't* do is ensure the player actually makes it to the
        checkpoint or meets whatever criteria you've defined. If they get lost
        or the game gets in a weird state, you can end up deferring the work
        longer than you expected.

* **In the background:**

    Like your text editor that auto-saves a backup every few minutes, you can
    do the work on some fixed time interval. Usually, you'll kick off the timer
    on the first modification and the process all of the changes that happened
    between then and when the timer fires. Then you reset and start all over
    again.

    * *You can tune how frequently the work is performed.* Here the timing of
        when we clean the dirty state and perform the work isn't dependent on
        the player requesting some data or reaching some checkpoint, so we can
        ensure it happens as frequently (or infrequently as we want).

    * *You can do more redundant work.* If the primary state only changes a
        tiny amount during the timer's run, and the granularity of our dirty
        flags is too coarse, we'll do work on a bunch of data that hasn't
        changed. If tiny changes trickle in, that timer will constantly be
        running and triggering work over data again and again that hasn't
        changed much.

    * *You can end up throwing away work.* The timer starts at the beginning
        of the first change to the primary data, and fires at some fixed
        interval after that. If the primary data is still being changed (i.e.
        the mayor is still in a zoning frenzy) when the timer goes off, we'll
        do the processing, and then immediately start the timer again and throw
        out what we just did since changes are still coming in.

        If that happens often, it may make sense for the timer to be more
        adaptive. One fix is to reset the timer on *every* change, not just the
        first. This means it will do the processing after a fixed amount of time
        has passed *where the primary state hasn't changed*.

        This helps you avoid pointless work, at the expense of running the risk
        of deferring too long. Imagine a player feverishly building for hours
        on end without a break. The timer will keep getting reset and never
        actually auto-save the city.

    * *You'll need some support for doing work "in the background".*
        Processing on a timer independent of what the player is doing implies
        the player can keep doing whatever that is while the processing is
        going on. After all, if the processing was so fast that we could do it
        synchronously while they played, we wouldn't need this pattern to begin
        with.

        That does mean we'll need threading or some other kind of concurrency
        support so that the work we're doing can happen while the game is still
        responsive and being played. Since the player is also interacting with
        the state that you're processing, you'll need to think about making that
        safe for concurrent modification too.

### How fine-grained is your dirty tracking?

In our first sample, we had a single flag for the entire city. We improved performance at the expense of some complexity and overhead by changing that to track a dirty bit for each block in the city. Depending on your use case, you may have options like that too.

* **If it's more coarse-grained:**

    * *It's simpler.* You don't have to spend as much time determining which
        *part* of the state is dirty. The code for doing the work can do it
        in one monolithic chunk instead of needing to be able to handle
        chewing on smaller isolated pieces of state.

    * *There's less overhead for tracking what is dirty.* You'll spend less
        memory on the dirty flags themselves (though that's almost always
        a trivial expense). When you go to process the dirty state and do
        the work, you'll need less metadata to express what subset of the
        data is dirty.

* **If it's more fine-grained:**

    * *You need to be able to process a subset of the data.* In our trophy
        room example, there'd by no way to split it into several narrower
        dirty flags. Since adding a single trophy can change the entire
        layout, the processing we have to do has to be a single monolithic
        operation that takes all trophies into account.

        In order to do finer-grained dirtiness-tracking, you'll need to make
        sure the work you have to do is amenable to being decomposed like
        that.

    * *You'll less more time working on unchanged data.* A coarse-grained
        dirty flag invalidates a larger swath of data, much of which may be
        unchanged. With finer-grained tracking, you only mark the data
        that's actually different and thus only process actual changes.

## See Also

*   This pattern is increasingly common outside of games in client-side MVC
    web frameworks like [Angular](http://angularjs.org/) which use dirty
    flags to track which data has been changed in the browser and needs to
    be pushed up to the server.

* Physics engines keep track of which objects are resting and which are in motion. Since a resting body won't move until an impulse is applied to it, they don't need any processing until they get touched. This "is resting" bit is essentially a dirty flag to track which objects have had forces applied and need to have their physics resolved.
