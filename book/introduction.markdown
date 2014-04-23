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
up the courage to even try it, but it was irresistible: the title
above the listing was "Tunnels and Trolls." We had no idea
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
head. Instead of just reading about "How to Program in
C++," I started trying to find books about how to *organize*
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

Unfortunately, "hiding" was often a good description.
There were gems buried in the code, but many people walked right over
them. I watched coworkers struggle to reinvent good solutions when
examples of exactly what they needed were nestled in the same codebase
they were standing on.

That problem is what this book aims to solve. I dug up and polished
the best patterns I've found in games and presented them here so that
we can spend our time inventing things instead of *re*-inventing them.

## What's in Store

There are already dozens of game programming books out there. Why
write another?

Most game programming books I've seen fall into one of two categories:

* **Domain-specific books.** These narrowly-focused books give you a deep dive on some specific aspect of game development. They'll teach you about 3D graphics, real-time rendering, physics simulation, artificial intelligence, or audio. These are the areas that many game programmers specialize in as their career progresses.

* **Whole-engine books.** In contrast, these try to span all of the different parts of an entire game engine. They compose them together to build a complete engine suited to some specific genre of game, usually a 3D first-person shooter.

I like both of these kinds of books, but I think they leave some gaps. Books specific to a domain rarely tell you how that chunk of code interacts with the rest of the game. You may be a wizard at physics and rendering, but do you know how to tie them together gracefully?

The second category covers that, but I find it often too monolithic and too genre-specific. Especially with the rise of mobile and casual gaming, we're in a period where lots of different genres of games are being created. We aren't all just cloning Quake anymore. Books that walk you through a single engine aren't a good fit when *your* game doesn't fit that mold.

Instead, what I'm trying to do here is more <span name="carte">*à la carte*</span>. Each of the chapters in this book is an independent idea that you can apply to your code. This way, you can mix and match them in a way that works best for the game *you* want to make.

<aside name="carte">

Another example of this *à la carte* style is the widely beloved [*Game Programming Gems*](http://www.satori.org/game-programming-gems/) series.

</aside>

## How it Relates to Design Patterns

Any programming book with <span name="alexander">"Patterns"</span> in its name clearly
bears a relationship to the classic *Design Patterns: Elements of
Reusable Object-Oriented Software* by Erich Gamma, Richard Helm, Ralph
Johnson and John Vlissides (ominously called the "Gang of
Four").

<aside name="alexander">

*Design Patterns* itself was in turn inspired by a previous book. The idea of crafting a language of patterns to describe open-ended solutions to problems comes from [*A Pattern Language*](http://en.wikipedia.org/wiki/A_Pattern_Language), by Christopher Alexander (along with Sarah Ishikawa and Murray Silverstein).

Their book was about architecture (like *real* architecture with buildings and walls and stuff), but they hoped others would use the same structure to describe solutions in other fields. *Design Patterns* is the Gang of Four's attempt to do that for software.

</aside>

By calling this book "Game Programming Patterns," I
certainly don't intend to imply that the Gang of Four's book is
<span name="nongames">inapplicable</span> to games. On the contrary: the second part of this book revisits
many of the patterns first described in *Design Patterns*, but with an
emphasis on how they can be applied to game programming.

Conversely, I think this book is applicable to non-game software too. The patterns here are useful in many business applications, but I think games make for more engaging examples. Do you really want to read yet another book about employee records and bank accounts?

Instead of trying to refute *Design Patterns*, I look at this book as
an *extension* of it. While the patterns introduced here are useful in other software, I think they're particularly well-suited to engineering challenges encountered most commonly in games:

*   Time and sequencing are often a core part of a game's
    architecture. Things must happen in the right order, and at the
    right time.

*   Development cycles are highly compressed and a number of
    programmers need to be able to rapidly build and iterate on a rich
    set of different behavior without stepping on each other's toes or
    leaving footprints all over the codebase.

*   After all of this behavior is defined, it starts interacting.
    Monsters bite the hero, potions are mixed together, and bombs
    blast enemies and friends alike. Those interactions must happen
    without the codebase turning into an intertwined hairball.

*   And, finally, performance is critical in games. Game developers
    are in a constant race to see who can squeeze the most out of
    their platform. Tricks for shaving off cycles can mean the
    difference between an A-rated game and millions of sales, or
    dropped frames and angry reviewers.

## How to Read the Book

The book is broken into three broad parts. The first introduces and frames the book. It's the chapter you're reading now along with the [next one](architecture-performance-and-games.html).

The second part, ["Design Patterns Revisited"](design-patterns-revisited.html), goes through a handful of patterns from the Gang of Four book. With each one, I give my spin on it and how I think it relates to game programming.

Finally, the last part is the real meat of the book. It presents thirteen new design patterns that I've found in games. They're grouped into four sections: [Sequencing Patterns](sequencing-patterns.html), [Behavioral Patterns](behavioral-patterns.html), [Decoupling Patterns](decoupling-patterns.html), and [Optimization Patterns](optimization-patterns.html).

Each pattern within these sections is described using a consistent
structure so that you can use this book as a reference and quickly
find what you need:

* The **Intent** section provides a snapshot description of the pattern
in terms of the problem it intends to solve. This is first so that you
can hunt through the book quickly to find a pattern that will help you
with your current struggle.

* The **Motivation** section describes an example problem that we will
be applying the pattern to. Unlike concrete algorithms, a pattern is
usually formless unless applied to some specific problem. Teaching a
pattern without an example is like teaching baking without mentioning
dough. This section provides the dough that the later sections will
bake.

* The **Pattern** section distills the essence of the pattern out of the
previous example. If you want a dry textbook description of the
pattern, this is it. It's also a good refresher if you're familiar with a pattern already and want to make sure you don't forget an ingredient.

* So far, the pattern has only been explained in terms of a single
example. But how do you know if *your* problem is a good one to use
the pattern on? The **When to Use It** section provides some
guidelines on when the pattern is useful, and when it's best avoided.
The **Keep in Mind** section points out implications that using the
pattern has on your program.

* A pattern is a template for a solution. Each time you use it, you'll
likely implement it differently. The next section, **Design
Decisions**, covers the different ways you might render the abstract
pattern into hard code.

* This is a whole lot of exposition and not a lot of code to look at.
The **Sample Code** remedies that by walking through an example
implementation or two. If, like me, you need concrete examples to
really *get* something, this is your section.

* To wrap it up, there's a short **See Also** section that shows how
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

In particular, the code is not written in "modern" -- C++11 or newer --
style. It does not use the standard library and rarely uses templates.
This makes for "bad" C++ code, but I hope that by keeping it
stripped down, it will be more approachable to people coming from C,
Objective-C, Java, and other languages.

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
