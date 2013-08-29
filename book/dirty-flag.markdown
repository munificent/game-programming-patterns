^title Dirty Flag
^section Optimizing Patterns

## Intent

*Avoid unneeded work by deferring it until the result is needed.*

## Motivation

- scene graph
  - most games have scene graph
  - has all objects in world
  - each node in the graph usually has transform: translate, scale and rotate
  - scene graph usually hierarchical -> objects can have parent
  - when object has parent, that means parent transform applies to it too
  - imagine world has pirate ship, which has crow's nest, which has pirate,
    which has parrot on shoulder
  - each is node in graph
  - each node's transform is relative to parent.
  - parrot's position is in pirate coords, pirate is in crow's nest coords, etc.
  - this is good because when the ship moves, the nest moves automatically,
    and the pirate, and the parrot.

- world coord
  - but to actuall draw parrot at right place in world, need to determine absolute
    world coordinates
  - won't get into detail, but do this by calculating transform matrices.
  - to get parrot's position, walk parent chain concatenating matrices. doing
    that isn't magic, but takes a little cpu juice.

- moving
  - when anything in the parrot's parent chain (including parrot) moves, parrot's
    position has to be recalculated: move the ship and the parrot moves too,
    so its world coord has to be update
  - simplest approach is to calc everything on render
    - every frame, walk entire scene graph recursively starting at top of
      hierarchy and calculate world pos for each node right before we render
    - since calc from scratch, don't have to track motion
    - slow! end up recalculating world coords for stuff every frame that
      aren't moving!
    - not good enough. obvious answer is cache world coord.
  - each node keeps local transform and world transform.
  - when object moves, update world transform too
  - but! remember hierarchy!
  - also have to walk child tree and recalc all of their nodes too
  - now imagine that all in one frame, the ship gets tossed on the ocean a bit, the crow's nest rocks in the wind, the pirate leans to the edge, and the parrot hops onto his head.
  - what happens:

    1. update ship local transform
    2. update ship world transform
    3. update nest world transform
    4. update pirate world transform
    5. update parrot world transform
    6. update nest local transform
    7. update nest world transform
    8. update pirate world transform
    9. update parrot world transform
    10. update pirate local transform
    11. update pirate world transform
    12. update parrot world transform
    13. update parrot local transform
    14. update parrot world transform

  - whoa. note that we recalculate the parrot's world transform 4 times before
    we render it once. that means first three are total wasted work. ditto the
    2 extra times we calculate the pirate pos and the 1 time the nest.

- problem is multiple node changes can invalidate a single node's world
  transform and we recalculate it immediately.
- solution is to decouple invalidating node from recalculating it.
- that lets us defer slow recalculation until after all transform changes are
  done
- that in turn lets us collapse multiple invalidations into a single recalc
- dirty flag
  - way we do this is by adding a flag to node that says "my world coord is
    invalid" (because local coord changed). call this "dirty".
  - whenever we move node, just set flag to true, but don't touch world coord.
  - then to all node movements for frame.
  - once they're all done, right before we render, walk tree
  - if we get to a node where flag is set, update world transform then *once*,
    regardless of how many times flag was set.
  - (when we recurse into child nodes, if the parent was dirty, implies they
    are too.)
  - if the flag *isn't* set, don't touch world coord. it's already fine.
- this gives two-fold advantage:
  - avoids recalculation on nodes that didn't change
  - avoids redundant calculation on nodes that changed more than once.
  - other minor bonus: if nodes are removed before rendering, never bother
    calculating at all!
  - in other words, world coords are only calculated right before they are
    actually needed
- in example, key pieces are:
  - have two set of data, primary and derived. here, primary is local trans
    for all nodes, derived is world coords.
  - derived data is calculated in some expensive process from primary data.
  - in this case, lots of matrix multiplies up the parent chain.
  - to minimize number of times we calculate derived data, use flag to track
    when its out of sync. allow it to go out of sync for as long as possible
    then only recalc once at last minute when derived data is actually needed
  - ensures you don't do any work that isn't needed

<!--
**TODO: renamed chapter, update this:**

OK, let's get two things out of the way first:

1.  Yes, the name of this chapter does make me giggle every time. I have the
    sense of humor of a ten-year-old boy.

2.  This is actually the widely-used name of this pattern.
    <span name="google">Google it yourself.</span> Apparently I'm not alone in
    my juvenile disposition. Another name for the same pattern is
    "dirty flag", but they are about equally commonly used. The Wikipedia
    article is ["dirty bit"](http://en.wikipedia.org/wiki/Dirty_bit) (though
    it's about a more specific use of the term).

<aside name="google">

You might want to wait until you're home from work before Googling it.

</aside>

Back to the matter at hand, let's talk about city-building games. I spent an
inordinate amount of my childhood in slavish devotion to Will Wright's masterpieces, so for this chapter, let's imagine we're making an homage to the classic mayoral role-playing game.

The player can build his city however he desires. As the city grows and develops, he occassionally is granted trophies and other rewards by his loving citizens. The player can bring up a trophy screen whenever he wants to bask in the light of his accomplishments. Here's where things get just a little bit tricky.

Every time the player gets a new trophy, the entire trophy case needs to be rearranged from scratch to generate a <span name="packing">maximally pleasing</span> arrangement. (You can imagine the virtual mayor spending hours lovingly doing this himself when he should be proofreading zoning agreements.) For our only-slightly-contrived example, let's pretend that that calculation is actually pretty time-consuming.

<aside name="packing">

Arranging the trophies is kind of like the [classic bin-packing problem](http://en.wikipedia.org/wiki/Bin_packing), which is NP-hard, so this being time-consuming isn't totally outside the bounds of reason.

</aside>

We could calculate the trophy arrangement every time the mayor gets a new trophy. That makes sure the trophy case is always up-to-date when they go to look at it. But, in practice, the player rarely goes to that screen. If they get *two* trophies between going to that screen, then we'll spend time generating the first arrangement, only to discard it without the player ever seeing it when the next trophy is awarded.

Instead, we could calculate the arrangement every time they enter the trophy room. But doing that is redundant if they go to the trophy room multiple times without having earned any new trophies. We'll just calculate the same arrangement each time.

The obvious solution is:

1. When the mayor gets a new trophy, we set a special "need trophy arrangement" flag.

2. When the player enters the trophy room, we check that flag. If it's set, we calculate the trophy arrangement and clear it. Otherwise, we use the previously calculated arrangement, which is still up to date.

The "need trophy arrangement" flag is the *dirty bit*. It's a single bit of data that indicates whether the current trophy arrangement is "dirty", or out of sync with the mayor's current set of trophies.

This is one of the two <span name="graphics">main uses</span> for this pattern in a nutshell. We'll cover the other related use a little farther down the chapter.

<aside name="graphics">

Back in the day when I was just a little game dev, this pattern used to have one particularly common application: rendering. Before GPUs, when games did all of their rendering in software, sprite engines were optimized to only redraw portions of the screen that actually changed.

The would maintain *dirty regions* that tracked which portions of the play area had been modified and needed re-rendering. These days with hardware-accelerated graphics, that technique is rarely needed anymore.
-->

</aside>

## The Pattern

A set of **primary data changes** over time. A set of **derived data** is determined from this using some **complex calculation or synchronization**. A **"dirty" flag** tracks when the derived data is out of sync with the primary data. It is **set when the primary data changes**. When the derived data is requested **it is calculated only when the flag is set and the flag is cleared.** Otherwise, the previous **cached derived data** is used.

## When to Use It

Compared to some other patterns in this book, this one solves a pretty specific problem. Also, like most optimization patterns, you should only reach for it when you have a performance problem big enough to justify the added code complexity. Added up, this means you probably won't use this often.

Dirty bits are used to solve two related problems: *calculation* and *synchronization*. What they have in common is:

1. A set of data that changes over time.
2. Another set of data that's somehow derived from that.
3. The process of going from 1 to 2 is "heavyweight".

When you use this pattern for calculation, like in the example in the motivation section, it's because there is actual complex computation involved that you want to avoid. With synchronization, it's more often that the derived data is *somewhere else* -- either on disk or over the network on another machine -- and simply getting it from point A to point B is slow.

In either case, it turns out that doing that heavyweight work is often for naught. Every time the primary data changes, you have to ditch the previous derived data and generate it again to take into account the new stuff. If the primary data changes *again* before you ever actually use the data derived from it, then there's no point in doing that work.

That brings in a third requirement for this pattern: *the primary data has to change more often than the derived data is used*. If you always need the derived data every single time the primary data is modified, there's no way to avoid doing the work to generate it, and this pattern can't help you.

There's one final softer fourth requirement. It's implied here that you can't easily *incrementally* update the derived data when the primary data changes. This pattern comes into play with the derived data is only calculated from *the primary data* and not from *the previous derived data*.

For example, if the mayor's trophy room just lined the trophies up one after the other in the order they were awarded, this pattern wouldn't be helpful. Each time he got a trophy, we can calculate the position *of just that trophy* and then add it to the end of the row. Doing this would require using the previous derived data (the other trophy positions) and that means we'd be best off updating the trophy positions *each time he gets a new trophy*.

In constrast, this pattern comes into play when the old derived data is totally discarded and calculated from scratch based on the primary data. In that case, we can *skip* updates without things getting out of sync since the previous derived data isn't used anyway.

All of this makes it sound like this pattern would never actually be useful, but over time you'll likely find a place here or there where it's a good fit. Grepping your average game codebase for the word "dirty" will often turn something up, and it almost always refers to this pattern.

## Keep in Mind

Once you have convinced yourself this pattern is a good fit, there are a few wrinkled that can cause you some discomfort.

### Deferring until the result is needed can cause a noticeable pause

In its simplest form, this pattern <aside name="gc">defers</aside> some slow work until the result is actually needed. But when the result is needed, it's often needed *right now* and the reason we're using this pattern to begin with is because calculating that result is slow!

In our example, if we wait until the player enters the trophy room to calculate the trophy positions, there may be a visible delay before the screen appears while the game is busy playing interior decorator. Sometimes, the pause is small enough that this isn't a problem.

When it is a problem, you'll often refine this pattern. There's actually a continuum of when to do the work ranging from "the second the primary data changes" all the way to "only at the last second when the derived data is needed". In between those two points are a range of options where you defer the work *somewhat* but still eventually kick it off before the result is actually needed.

When your fancy text editor auto-saves a backup "in the background" every few minutes, that's basically the trade-off it's making. The frequency that it auto-saves -- every few minutes to once an hour -- is picking a point on the continuum that balances not losing too much work when a crash occurs against not thrashing the file system too much by saving all the time.

<span name="gc">

This is mirrors the different garbage collection strategies in systems that automatically manage memory. Reference counting systems free memory the second its no longer needed, but suffer by spending CPU time updating ref counts eagerly every time references are changed.

Simple garbage collectors defer reclaiming memory until it's really needed. But the cost here is the dreaded "GC pause" that can freeze your entire app until the GC is done walking the heap.

In between the two are increasingly common more complex systems like deferred ref-counting and incremental GC that reclaim memory less eagerly than pure ref-counting but more eagerly than stop-the-world collectors.

</span>

### You have to make sure to set the flag *every* time the state changes

Since the derived data is calculated from the primary data, it's essentially a cache. Whenever you have cached data in memory, the trickiest aspect of it is <span name="cache">*cache invalidation*</span>: correctly knowing when the cache is out of sync with its source data. In this pattern, that means correctly setting the dirty bit when *any* primary data changes.

<aside name="cache">

Phil Karlton famously said, "There are only two hard things in Computer Science: cache invalidation and naming things."

My favorite variation is, "There are only two hard things in Computer Science: cache invalidation, naming things, and off-by-one errors."

</aside>

Miss it in one place, and your program will incorrectly use stale derived data. This leads to confused players and very hard to track down bugs. When you use this pattern, you'll have to be very careful that any code that modifies the primary state also sets the dirty bit.

One way to mitigate this is by encapsulating modifications to the primary data behind some interface. If anything that can change the state goes a single simple interface, you can set the dirty bit there and rest assurred that it won't go out of sync.

### You will have to keep the previous derived data in memory

When the game needs the derived data, it checks to see if the dirty flag is set. If it *isn't* set, then it uses the previously calculated data. This is obvious, but that does imply that you have to keep that derived data around in memory in case you end up needing it later.

If you weren't using this pattern, another option would be just calculate the derived data on the fly right when you need it, and then discard it when you're done. That avoids the expense of keeping it cached in memory. The cost doing this is calculation: you'll have to do that calculation *every* time you need it.

Like many optimizations, then, you can look at this pattern as trading off memory for space. In return for keeping the previously calculated data in memory, you avoid having to recalculate it when it's imput data hasn't changed. This trade-off makes sense when the calculation is slow and memory is cheap. When you've got more time than memory on your hands, it may be better to just calculate it as needed and avoid this pattern.

## Sample Code

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

## Design Decisions

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

* This pattern is increasingly common outside of games in client-side MVC web frameworks like [Angular](http://angularjs.org/) which use dirty flags to track which data has been changed in the browser and needs to be pushed up to the server.

* Physics engines keep track of which objects are resting and which are in motion. Since a resting body won't move until an impulse is applied to it, they don't need any processing until they get touched. This "is resting" bit is essentially a dirty flag to track which objects have had forces applied and need to have their physics resolved.
