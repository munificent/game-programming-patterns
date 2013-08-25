^title Dirty Bits
^section Optimizing Patterns

## Intent

*Avoid unneeded work by deferring it until the result is needed.*

## Motivation

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

</aside>

Miss it in one place, and your program will incorrectly use stale derived data. This leads to confused players and very hard to track down bugs. When you use this pattern, you'll have to be very careful that any code that modifies the primary state also sets the dirty bit.

One way to mitigate this is by encapsulating modifications to the primary data behind some interface. If anything that can change the state goes a single simple interface, you can set the dirty bit there and rest assurred that it won't go out of sync.

- when the flag isn't dirty, you'll use previously calculated data. assumes you're willing to hang on to that then. like many optimizations, trades space for speed. if you can calculate it quickly and need the space, may be better to just calculate it every time.

- don't want to defer too long

## Sample Code

- motivation example was so simple the code would probably be pointless. whenever the notoriety changes or he gets a new scar, the flag is set. when the user brings up the screen, if the flag is set, we render the poster and clear it. done. simple!

- so instead of showing that, let's do the other main use case where dirty flags are used: synchronization. let's save our city... to disk.

- player spends lots of time building city, doesn't want to just trash it when they quit.
- obviously saving to disk every time they change anything would be too slow.
- obvious is to save on quit
- but what if player didn't do anything?
- notice how in word processor, it only asks to save if you have unsaved changes. that's the dirty bit in action!
- we can do same thing
- first, in any code that changes city, also set dirty flag

    ...

- then, when player quits game, check flag and save if dirty

### Too slow

- this works, but takes a long time to save.
- entire city is pretty huge file, and most of it is unchanged most of the time
- would like saving to be faster by only saving parts of city that change
- divide city into blocks
- each block has its own dirty flag, and own region in the file
- when quiting, we check if any flag is dirty
- if so, walk blocks and save dirty ones
- just seek over non-dirty ones
- when city changes, have to remember to mark correct block as dirty
- so, finer grained dirty checking can mean you do less work
- but also means have to do a bit more work checking all of the dirty flags
- and make sure right one is marked dirty at right time

### Game crashes

- saving on quit is fine most of the time, but sometimes game crashes (surely not our code, but other coders aren't so flawless)
- would be nice if all changes weren't lost
- so decide to save in the background
- don't want to save on every change of course. even with blocks, still too hard on disk
- so we'll save periodically.
- whenever block is marked dirty, start timer
- when timer fires, all dirty blocks are saved
- if timer is already running when block is dirty, just leave it
- once we save, clear timer

## Design Decisions

- what is granularity of dirtiness?

  - do you have single flag for entire data structure, or flags for just pieces of it?
  - finer-grained means less work because you only process/transmit the chunks that are
    dirty.
  - but can also adds complexity walking over the data structure to check which parts are
    dirty.

- when is it cleaned?

  - too aggressively defeats the purpose.
  - too lazily can cause pauses or "clumpiness".

  - when needed

    - simplest answer for recalculation: run it when you need the result.
    - avoid recalc if result is never used (or is invalidated before result is
      used.)
    - if calc is slow, can cause a noticeable pause.
    - for synchronization "when needed" isn't always well-defined.

  - at well-defined checkpoints

    - for things like save/load or network sync, this may work best. have
      well-defined points of synchonization.
    - can be based on time ("game saves automatically every 5 min") or location
      ("game is saved every time you enter town")
    - always risk of data loss if something bad happens before reaching checkpoint
    - can be a feature of the game

  - in background

    - when data is marked dirty, start a timer
    - when timer fires, save or calculate and clear
    - if timer is already running when something else becomes dirty, have to
      decide if you want to reset timer or not
      - resetting can give you better data batching: if the data keeps changing
        frequently enough to reset the timer, it may be worth waiting until it
        settles down instead of doing work on volatile data
      - not resetting makes sure you don't delay work indefinitely and drop data
        makes sure data is actually processed

## See Also

- mvc frameworks often use this to track model changes to know what notifications to send to view

- web mvc frameworks use this to know what modifications to view need to be synchronized with server/client.
