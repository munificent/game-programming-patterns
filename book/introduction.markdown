^title Introduction

In fifth grade, my friends and I were given access to a little unused
classroom housing a couple of very beat-up TRS-80s. Hoping to inspire us, a
teacher found a printout of some simple BASIC programs for us to tinker with.

The audio cassette drives on the computers were broken, so any time we wanted to
run some code, we'd have to carefully type it in from scratch. This led us
to prefer programs that were only a few lines long:

<span name="radical"></span>

    10 PRINT "BOBBY IS RADICAL!!!"
    20 GOTO 10

<aside name="radical">

Maybe if the computer prints it enough times, it will magically become true.

</aside>

Even so, the process was fraught with peril. We didn't know *how* to program,
so a tiny syntax error was impenetrable to us. If the program didn't work,
which was often, we started over from the beginning.

At the back of the stack of pages was a real monster: a program that took up
several dense pages of code. It took a while before we worked up the courage
to even try it, but it was irresistible -- the title above the listing was
"Tunnels and Trolls". We had no idea what it did, but it sounded like a game,
and what could be cooler than a computer game that you programmed yourself?

We never did get it running, and after a year, we moved out of that classroom.
(Much later when I actually knew a bit of BASIC, I realized that it was just a
character generator for the table-top game and not a game in itself.) But the
die was cast -- from there on out, I wanted to be a game programmer.

When I was in my teens, my family got a Macintosh with QuickBASIC and later
THINK C. I spent almost all of my <span name="snakes">summer vacations</span>
hacking together games. Learning on my own was slow and painful. I'd get
something up and running easily -- maybe a map screen or a little puzzle --
but as the program grew, it got harder and harder.

<aside name="snakes">

Many of my summers were also spent catching snakes and turtles in the swamps
of southern Louisiana. If it wasn't so blisteringly hot outside, there's a
good chance this would be a herpetology book instead of a programming one.

</aside>

At first, the challenge was just getting something working. Then, it became
figuring out how to write programs bigger than what would fit in my head. Instead
of just reading about "How to Program in C++", I started trying to find books
about how to *organize* programs.

Fast-forward several years, and a <span name="friend">friend</span> hands me a
book: *Design Patterns: Elements of Reusable Object-Oriented Software*.
Finally! The book I'd been looking for since I was a teenager. I read it cover
to cover in one sitting. I still struggled with my own programs, but it was
such a relief to see that other people struggled too and came up with
solutions. I felt like I finally had a couple of *tools* to use instead of
just my bare hands.

<aside name="friend">

This was the first time we'd met, and five minutes after being introduced, I
sat down on his couch and spent the next few hours completely ignoring him and
reading. I'd like to think my social skills have improved at least a little
since then.

</aside>

In 2001, I landed my dream job: software engineer at Electronic Arts. I
couldn't wait to get a look at some real games and see how the pros put them
together. What was the architecture like for an enormous game like Madden
Football? How did the different systems interact? How did they get a single
codebase to run on multiple platforms?

Cracking open the source code was a humbling and surprising experience. There
was brilliant code in graphics, AI, animation, and visual effects. We had
people who knew how to squeeze every last cycle out of a CPU and put it to
good use. Stuff I didn't even know was *possible*, these people did before
lunch.

But the *architecture* this brilliant code hung from was often an
afterthought. They were so focused on *features* that organization went overlooked. Coupling was rife between modules.
New features were often bolted onto the codebase wherever they could be made
to fit. To my disillusioned eyes, it looked like many programmers, if they ever
cracked open *Design Patterns* at all, never got past <a class="pattern"
href="singleton.html">Singleton</a>.

Of course, it wasn't really that bad. I'd imagined game programmers sitting in
some ivory tower covered in whiteboards, calmly discussing architectural
minutiae for weeks on end. The reality was that the code I was looking at was
written by people working to meet intense deadlines. They did the best they
could, and, as I gradually realized, their best was often very good. The more
time I spent working on game code, the more bits of brilliance I found hiding
under the surface.

Unfortunately, "hiding" was often a good description. There were gems buried
in the code, but many people walked right over them. I watched coworkers
struggle to reinvent good solutions when examples of exactly what they needed
were nestled in the same codebase they were standing on.

That problem is what this book aims to solve. I dug up and polished the best
patterns I've found in games, and presented them here so that we can spend our
time inventing new things instead of *re*-inventing them.

## What's in Store

There are already dozens of game programming books out there. Why write
another?

Most game programming books I've seen fall into one of two categories:

* **Domain-specific books.** These narrowly-focused books give you a deep dive
  on some specific aspect of game development. They'll teach you about 3D
  graphics, real-time rendering, physics simulation, artificial intelligence,
  or audio. These are the areas that many game programmers specialize in as
  their careers progress.

* **Whole-engine books.** In contrast, these try to span all of the different
  parts of an entire game engine. They are oriented towards building a
  complete engine suited to some specific genre of game, usually a 3D first-person shooter.

I like both of these kinds of books, but I think they leave some gaps. Books
specific to a domain rarely tell you how that chunk of code interacts with the
rest of the game. You may be a wizard at physics and rendering, but do you
know how to tie them together gracefully?

The second category covers that, but I often find whole-engine books to be too monolithic and too
genre-specific. Especially with the rise of mobile and casual gaming, we're in
a period where lots of different genres of games are being created. We aren't
all just cloning Quake anymore. Books that walk you through a single engine
aren't helpful when *your* game doesn't fit that mold.

Instead, what I'm trying to do here is more <span name="carte">*à la
carte*</span>. Each of the chapters in this book is an independent idea that
you can apply to your code. This way, you can mix and match them in a way that
works best for the game *you* want to make.

<aside name="carte">

Another example of this *à la carte* style is the widely beloved [*Game
Programming Gems*](http://www.satori.org/game-programming-gems/) series.

</aside>

## How it Relates to Design Patterns

Any programming book with <span name="alexander">"Patterns"</span> in its name
clearly bears a relationship to the classic *Design Patterns: Elements of
Reusable Object-Oriented Software* by Erich Gamma, Richard Helm, Ralph Johnson,
and John Vlissides (ominously called the "Gang of Four").

<aside name="alexander">

*Design Patterns* itself was in turn inspired by a previous book. The idea of
crafting a language of patterns to describe open-ended solutions to problems
comes from [*A Pattern
Language*](http://en.wikipedia.org/wiki/A_Pattern_Language), by Christopher
Alexander (along with Sarah Ishikawa and Murray Silverstein).

Their book was about architecture (like *real* architecture with buildings and
walls and stuff), but they hoped others would use the same structure to
describe solutions in other fields. *Design Patterns* is the Gang of Four's
attempt to do that for software.

</aside>

By calling this book "Game Programming Patterns", I'm not trying to imply that
the Gang of Four's book is <span name="nongames">inapplicable</span> to games.
On the contrary: the [Design Patterns Revisited](design-patterns-revisited.html)
section of this book covers many of the patterns from *Design
Patterns*, but with an emphasis on how they can be applied to game
programming.

Conversely, I think this book is applicable to non-game software too. I could
just as well have called this book *More Design Patterns*, but I think games
make for more engaging examples. Do you really want to read yet another book
about employee records and bank accounts?

That being said, while the patterns introduced here are useful in other
software, I think they're particularly well-suited to engineering challenges
commonly encountered in games:

*   Time and sequencing are often a core part of a game's architecture. Things
    must happen in the right order and at the right time.

*   Development cycles are highly compressed, and a number of programmers need
    to be able to rapidly build and iterate on a rich set of different
    behavior without stepping on each other's toes or leaving footprints all
    over the codebase.

*   After all of this behavior is defined, it starts interacting. Monsters
    bite the hero, potions are mixed together, and bombs blast enemies and
    friends alike. Those interactions must happen without the codebase turning
    into an intertwined hairball.

*   And, finally, performance is critical in games. Game developers are in a
    constant race to see who can squeeze the most out of their platform.
    Tricks for shaving off cycles can mean the difference between an A-rated
    game and millions of sales or dropped frames and angry reviewers.

## How to Read the Book

*Game Programming Patterns* is divided into three broad sections. The first
introduces and frames the book. It's the chapter you're reading now along with
the [next one](architecture-performance-and-games.html).

The second section, [Design Patterns Revisited](design-patterns-revisited.html),
goes through a handful of patterns from the Gang of Four book. With each chapter,
I give my spin on a pattern and how I think it relates to game programming.

The last section is the real meat of the book. It presents thirteen
design patterns that I've found useful. They're grouped into four categories:
[Sequencing Patterns](sequencing-patterns.html), [Behavioral Patterns](behavioral-patterns.html), [Decoupling Patterns](decoupling-patterns.html),
and [Optimization Patterns](optimization-patterns.html).

Each of these patterns is described using a consistent structure so that you
can use this book as a reference and quickly find what you need:

* The **Intent** section provides a snapshot description of the pattern in
  terms of the problem it intends to solve. This is first so that you can hunt
  through the book quickly to find a pattern that will help you with your
  current struggle.

* The **Motivation** section describes an example problem that we will be
  applying the pattern to. Unlike concrete algorithms, a pattern is usually
  formless unless applied to some specific problem. Teaching a pattern without
  an example is like teaching baking without mentioning dough. This section
  provides the dough that the later sections will bake.

* The **Pattern** section distills the essence of the pattern out of the
  previous example. If you want a dry textbook description of the pattern,
  this is it. It's also a good refresher if you're familiar with a pattern
  already and want to make sure you don't forget an ingredient.

* So far, the pattern has only been explained in terms of a single example.
  But how do you know if the pattern will be good for *your* problem?
  The **When to Use It** section provides some guidelines on when the pattern
  is useful and when it's best avoided. The **Keep in Mind** section points
  out consequences and risks when using the pattern.

* If, like me, you need concrete examples to really *get* something,
  then **Sample Code** is your section. It walks step by step through a full
  implementation of the pattern so you can see exactly how it works.

* Patterns differ from single algorithms because they are open-ended. Each
  time you use a pattern, you'll likely implement it differently. The next section,
  **Design Decisions**, explores that space and shows you different options to
  consider when applying a pattern.

* To wrap it up, there's a short **See Also** section that shows how this
  pattern relates to others and points you to real-world open source code that
  uses it.

## About the Sample Code

Code samples in this book are in C++, but that isn't to imply that these
patterns are only useful in that language or that C++ is a better language
for them than others. Almost any language will work fine, though some patterns
do tend to presume your language has objects and classes.

I chose C++ for a couple of reasons. First, it's the most popular language for
commercially shipped games. It is the *lingua franca* of the industry. Moreso,
the C syntax that C++ is based on is also the basis for Java, C#, JavaScript,
and many other languages. Even if you don't know C++, the odds are good you
can understand the code samples here with a little bit of effort.

The goal of this book is *not* to teach you C++. The samples are kept as
simple as possible and don't represent good C++ style or usage. Read the code
samples for the idea being expressed, not the code expressing it.

In particular, the code is not written in "modern" -- C++11 or newer -- style.
It does not use the standard library and rarely uses templates. This makes for
"bad" C++ code, but I hope that by keeping it stripped down, it will be more
approachable to people coming from C, Objective-C, Java, and other languages.

To avoid wasting space on code you've already seen or that isn't relevant to
the pattern, code will sometimes be omitted in examples. When this occurs, an
ellipsis will be placed in the sample to show where the missing code goes.

Consider a function that will do some work and then return a value. The
pattern being explained is only concerned with the return value, and not the
work being done. In that case, the sample code will look like:

^code update

## Where to Go From Here

Patterns are a constantly changing and expanding part of software development.
This book continues the process started by the Gang of Four of documenting and
sharing the software patterns they saw, and that process will continue after
the ink dries on these pages.

You are a core part of that process. As you develop your own patterns and
refine (or refute!) the patterns in this book, you contribute to the software
community. If you have suggestions, corrections, or other feedback about
what's in here, please get in touch!
