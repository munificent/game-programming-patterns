^title Introduction

In fifth grade, my friends and I were given access to a little unused
classroom housing a couple of very beat up TRS-80s. Hoping to inspire
us, a teacher found a printout of some simple BASIC programs for us
to tinker with.

The audiocasette drives on the computers were broken, so any time we
wanted to run one of these programs we'd have to carefully type it in
all over <span name="short">again</span>. This led us to prefer programs that were only a few
lines long:

    10 PRINT "BOBBY IS RADICAL!!!"
    20 GOTO 10

<aside name="short">

This is learning the value of short, simple code the hard way.

</aside>

Even so, the process was fraught with peril. We didn't know *how* to
program, so a tiny syntax error was impenetrable to us. If the program
didn't work, which was often, we started over from scratch.

At the back of the stack of pages was a real monster: a program that
took up several dense pages of code. It took a while before we worked
up the courage to even try it, but it was irresistable: the title
above the listing was &ldquo;Tunnels and Trolls.&rdquo; We had no idea
what it did, but it sounded like a game, and what could be cooler than
a computer game that you programmed yourself?

We never did get it running, and after a year we moved out of that
classroom. (Much later when I actually knew a bit of BASIC, I realized
that it was just a character generator for the table-top game, and not
a game in itself.) But the die was cast -- from here on out, I
wanted to be a game programmer.

When I was in my teens, my family got a Macintosh with QuickBASIC and
later THINK C. I spent almost all of my <span name="snakes">summer vacations</span> hacking
together game programs. Learning on my own was slow and painful. I'd
get something up and running quickly -- maybe a map screen, or a
little puzzle -- but as I added features it got harder and harder.
Once I couldn't fit the entire program in my head anymore, it was
doomed.

<aside name="snakes">

A good chunk of my time was also spent catching snakes and turtles in
the swamps of southern Louisiana. If it wasn't so blisteringly hot
outside, there's a good chance this would be a herpetology book
instead of a programming one.

</aside>

At first, the challenge was just getting something on screen. Now it
became figuring out how to write programs bigger than would fit in my
head. Instead of just reading about &ldquo;How to Program in
C++,&rdquo; I started trying to find books about how to *organize*
programs.

Fast-forward several years and a <span name="friend">new friend</span> hands me a book: *Design
Patterns: Elements of Reusable Object-Oriented Software*. Finally! The
book I'd been looking for since I was a teenager. I read it cover to
cover in one sitting. I still struggled with my own programs, but it
was such a relief to see that other people struggled too, and came up
with solutions. I felt like I finally had a couple of *tools* to use
instead of just my bare hands.

<aside name="friend">

This was the first time I'd met a friend of my best friend, and five minutes after meeting him, I sat down on his couch and spent the next few hours completely ignoring him and reading. I'd like to think my social skills have improved at least a little since then.

</aside>

In 2001, I landed my dream job: software engineer at Electronic Arts.
I would be a real game programmer! I couldn't wait to get a look at
some real games and see how the pros put them together. How did they
build an enormous game like Madden Football when it clearly was too
big to fit in anyone's head? What was the architecture like? How did
they decouple physics and rendering? How did the AI code interact with
the animation? How did they abstract out audio to work across multiple
platforms?

Cracking open the source code was a humbling and surprising
experience. There was so much brilliant code in graphics, AI,
animation, and visual effects. We had people who knew how to squeeze
every last cycle out of a CPU, and put it to good use. Stuff I didn't
even know was *possible*, these people did before lunch.

But the *architecture* that this brilliant code hung from was often an
afterthought. They were so focused on *features* that there wasn't
much attention left for the code between the code. Coupling was rife
between modules. New features were often bolted onto the codebase
wherever they could be made to fit. To my disillusioned eyes it looked
like many programmers, if they ever cracked open *Design Patterns* at
all, never got past <a class="pattern"
href="singleton.html">Singleton</a>.

Of course, it wasn't really that bad. I'd imagined game programmers
sitting in some ivory tower covered in whiteboards, calmly discussing
architectural minutia for weeks on end. The reality was that the code
I was looking at was written by people working under intense
deadlines, on platforms where every CPU cycle counted. They did the
best they could, and as I gradually realized, their best was often
very good. The more time I spent working on game code, the more bits
and pieces of absolute brilliance I found hiding under the surface.

Unfortunately, &ldquo;hiding&rdquo; was often a good description.
There were gems buried in the code, but many people walked right over
them. I watched coworkers struggle to reinvent good solutions when
examples of exactly what they needed were nestled in the same codebase
they were standing on.

That problem is what this book aims to solve. I dug up and polished
the best patterns I've found in games and presented them here so that
we can spend our time inventing things instead of *re*-inventing them.

## What's in Store
There are already dozens of game programming books out there. Why
write another? I'll explain by analogy.

Imagine a game codebase as a house. Graphics and sound are the
appliances and fixtures: a nice chandelier, kitchen sink, big picture
windows. This is the kind of stuff you can find in other books. These
things matter, of course, but this book aims at something a bit more
humble and fundamental: the framing. Other books can teach you about
windows and fixtures, faucets and tubs. What I hope to cover in this
book is the joist and the arch, plumbing and wiring -- techniques
that will let you build an elegant and maintainable structure out of a
few simple parts. Use them to frame the house, then crack open those
other books to build it out.

It's easy to <span name="tragedy">underappreciate</span> this. After all, friends won't look at
your home and say, &ldquo;Wow, nice wiring!&rdquo; But without it, I
can guarantee they will notice when you flip the lightswitch and your
beautiful chandelier doesn't turn on.

<aside name="tragedy">

This is the tragedy of software architecture: the better it is, the
less it's noticed. All people will notice are the features that were
enabled by it. As the Tao Te Ching says: *When the Sage's work is
done, the people say, &ldquo;We did it all by ourselves!&rdquo;*

</aside>

## How it Relates to Design Patterns

Any programming book with &ldquo;Patterns&rdquo; in its name clearly
bears a relationship to the classic *Design Patterns: Elements of
Reusable Object-Oriented Software* by Erich Gamma, Richard Helm, Ralph
Johnson and John Vlissides (ominously called the &ldquo;Gang of
Four&rdquo;).

By calling this book &ldquo;Game Programming Patterns,&rdquo; I
certainly don't intend to imply that the Gang of Four's book is
<span name="nongames">inapplicable</span> to games. On the contrary: Part Two of this book revisits
many of the patterns first described in *Design Patterns*, but with an
emphasis on how they can be applied to game programming.

<aside name="nongames">

Conversely, I think this book is applicable to non-game software too.
The patterns in it can be put to good use in all kinds of software.
Games are just where I see them encountered the most, and provide a
wellspring of good examples of their use.

</aside>

Instead of trying to refute *Design Patterns*, I look at this book as
an *extension* of it. You can think of *Design Patterns* as *The Joy
of Cooking*. Regardless of what course you're preparing, there will be
a recipe in there for you. But if you've decided to really focus on
baking, a book dedicated to desserts and pastries can take things
further.

That's this book. Part Three, the largest part of the book, covers a
set of new patterns that I've found used frequently (and often
exclusively) in games. Game software is different from other domains
in many ways, and these patterns address those differences:

*   Time and sequencing are often a core part of a game's
    architecture. Things must happen in the right order, and at the
    right time.

*   Development cycles are highly compressed and a number of
    programmers need to be able to rapidly build and iterate on a rich
    set of different behavior without stepping on each others' toes or
    leaving footprints all over the codebase.

*   After all of this behavior is defined, it starts interacting.
    Monsters bite the hero, potions are mixed together, and bombs
    blast enemies and friends alike. Coordinating that kind of mass
    constant communication is rare outside of games. If you have two
    documents open in your word processor, they don't talk to each
    other very often.

*   And, finally, performance is critical in games. Game developers
    are in a constant race to see who can squeeze the most out of
    their platform. Tricks for shaving off cycles can mean the
    difference between an A-rated game and millions of sales, or
    dropped frames and angry reviewers.

## How To Read This

This book is structured like a recipe book because it essentially *is*
one. The chapter you're reading now introduces the book and explains
how the rest of the material works. It tells you how to follow a
recipe, prepare the ingredients, and make sure you don't leave the
oven on.

The next section revisits some of the patterns from *Design Patterns*
that I feel are particularly relevant to game programming. It will
show you some pitfalls to avoid, and some clever tricks to get the
most out of them.

Following that are the recipes: the new patterns themselves. They are
grouped into several themes. Each theme describes two opposing forces
encountered when building a game. The patterns within that theme are
tools you can use to find a balance between those forces.

For example, the &ldquo;Communicating&rdquo; theme is for patterns
that enable parts of a game to communicate with each other. The
primary force is that game objects *must* interact: bullets must
collide with enemies, the hero must be able to play heroic sounds. The
opposing force is that decoupling and isolating code makes it easier
to understand, modify, and test. The patterns in this theme let
modules communicate with each other, but in prescribed ways that
control the coupling.

Each pattern within these themes is described using a consistent
structure so that you can use this book as a reference and quickly
find what you need.

The **Intent** section provides a snapshot description of the pattern
in terms of the problem it intends to solve. This is first so that you
can hunt through the book quickly to find a pattern that will help you
with your current struggle.

The **Motivation** section describes an example problem that we will
be applying the pattern to. Unlike concrete algorithms, a pattern is
usually formless unless applied to some specific problem. Teaching a
pattern without an example is like teaching baking without mentioning
dough. This section provides the dough that the later sections will
bake.

The **Pattern** section distills the essence of the pattern out of the
previous example. If you want a dry textbook description of the
pattern, this is it. It's also a good refresher if you're familiar with a pattern already and want to make sure you don't forget an ingredient.

So far, the pattern has only been explained in terms of a single
example. But how do you know if *your* problem is a good one to use
the pattern on? The **When to Use It** section provides some
guidelines on when the pattern is useful, and when it's best avoided.
The **Keep in Mind** section points out implications that using the
pattern has on your program.

A pattern is a template for a solution. Each time you use it, you'll
likely implement it differently. The next section, **Design
Decisions**, covers the different ways you might render the abstract
pattern into hard code.

This is a whole lot of exposition and not a lot of code to look at.
The **Sample Code** remedies that by walking through an example
implementation or two. If, like me, you need concrete examples to
really *get* something, this is your section.

To wrap it up, there's a short **See Also** section that shows how
this pattern relates to others both in this book and in *Design
Patterns*. This will give you a clearer picture of how a pattern lives
in the larger pattern ecosystem.

## How the Book is Formatted

Each <span name="aside">pattern</span> has a set of parts -- the ingredients in its recipe.
Every implementation will include these parts, so it's important to
note them all. The first time one is mentioned, it will be in **bold
face**.

<aside name="aside">

Asides, extra information, and dumb jokes are placed in asides. Feel
free to skip them if you want.

</aside>

Code samples are shown using a `monospace font`.

When another pattern in this book is mentioned, it will be
capitalized. Following that will be the page number where the pattern
is defined, like <a class="pattern" href="object-pool.html">Object Pool</a>. Patterns described by the Gang of Four
in *Design Patterns* will also be capitalized, but instead of a page
number, will be shown like: <a class="gof-pattern" href="http://www.c2.com/cgi/wiki?DecoratorPattern">Decorator</a>.

## About the Sample Code

Code samples in this book are in C++, but that isn't to imply that
these patterns are only useful in that language, or that C++ is a
better language for them than others. Any OOP language will work fine.

I chose C++ for a couple of reasons. Most importantly, it's currently
the most popular language for programming commercially shipped games.
It is the *lingua franca* for this industry. Moreso, the C syntax that
C++ is based on is also the basis for Java, C#, and many other
languages. With a little bit of effort, a programmer using any of
these languages should be able to understand the code samples here.

The goal of this book is *not* to teach you C++. The samples here are
kept as simple as possible and don't represent good C++ style or
usage. Read the code samples for the idea being expressed, not the
code expressing it.

To avoid wasting space on code you've already seen or that isn't
relevant to the pattern, code will sometimes be omitted in examples.
When this occurs, an ellipsis will be placed in the sample to show
where the missing code goes.

Consider a function that will do some work and then return a value.
The pattern being explained is only concerned with the return value,
and not the work being done. In that case, the sample code will look
like:

^code update

## Where to Go From Here

Patterns are a constantly changing and expanding part of software
development. This book continues the process started by the Gang of
Four of documenting and sharing the software patterns they saw, and
that process will continue after the ink dries on these pages.

You are a core part of that process. As you develop your own patterns
and refine (or refute!) the patterns in this one, you contribute to
the software community. If you have suggestions, corrections, or other
feedback about what's in here, please get in touch.
