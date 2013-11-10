^title Structure of Arrays
^section Optimizing Patterns

*Sorry, this chapter hasn't been written yet!*

random notes

- making your data structures smaller (bit-packing, ordering fields to reduce padding) will fit more into a cache line, that in turn directly makes them faster

    - smaller isn't just about saving memory
    - but careful about re-ordering fields since that can interfere with access
      patterns

- avoid loading entire object into cache just for one bit, like:

    every actor:
      if (actor->isActive) actor->update();


benchmarking notes

tried a bunch of different stuff. first benchmark looked super promising:
30% faster to update components when contiguous instead of actor

apparently completely artifact of how code was organized. commenting stuff out
or moving code could nullify or reverse change

(maybe it's an icache miss issue?)

tried other smaller benchmark and cache grind
- update a bunch of actors with settable amount of padding between them
- as padding increases, can see cache misses go up, which is good
- tops off right at around 64 bytes total for an actor which is expected since
  that's the cache line size
- no noticeable performance difference though
- maybe need to do more fake work with data so there's most instructions that
  could be processed during read if didn't have cache stall?

references

* http://msinilo.pl/blog/?p=614
* http://research.scee.net/files/presentations/gcapaustralia09/Pitfalls_of_Object_Oriented_Programming_GCAP_09.pdf
* http://www.microsoft.com/downloads/details.aspx?FamilyId=1643D55A-D252-4717-BC3E-237C2C5295F4&displaylang=en
* http://gamesfromwithin.com/data-oriented-design
* http://dice.se/wp-content/uploads/Introduction_to_Data-Oriented_Design.pdf
* https://docs.google.com/presentation/d/17Bzle0w6jz-1ndabrvC5MXUIQ5jme0M8xBF71oz-0Js/present#slide=id.i0
* http://igoro.com/archive/gallery-of-processor-cache-effects/
