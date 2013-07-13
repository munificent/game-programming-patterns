^title Introduction

When I was in fifth grade, my friends and I were given access to a
"resource center" center, a little unused classroom with a couple of
very beat up TRS-80s. A helpful teacher, hoping to inspire us, found a
printout of a bunch of simple BASIC programs and gave it to us.

> I learned the value of short, simple code the hard way.

Every morning, we'd carefully type in one that caught our eye. We had
to retype them every day since the audiocasette drives on the
computers were broken. Of course, this led us to prefer the programs
that were only a few lines long:

    10 PRINT "BOB IS SUPER RAD"
    20 GOTO 10

Even then, the process was fraught with peril. We didn't know *how*
to program, so a tiny syntax error was impenetrable to us. We'd often
have to start over from scratch.

But at the back of the stack of pages was a real monster: a program
that took up several dense pages of code. It took a while before we
worked up the courage to even try it, but it was irresistable: it was
called "Tunnels and Trolls". We had no idea what it did, but it sure
as hell sounded like a game, and I wanted to see it running so bad I
could taste it.

We never did get it running, and after a year, we moved out of that
resource center. Much later when I actually knew a bit of BASIC, I
realized that it was just a character generator for the table-top
game, and not a game in itself. But the damage was already done. From
here on out, I wanted to be a game programmer.

> A good chunk of my time was also spent catching snakes in the
> swamps of southern Louisiana. If it wasn't so blisteringly hot
> outside, there's a good chance this would be a herpetology book
> instead of a programming one.

My family later got a Macintosh with QuickBASIC and later THINK C on
it. I spent almost all of my free time hacking together game programs.
Learning on my own was slow and painful, but rewarding too. I'd get
something up and running quickly: maybe a map screen, or a little
puzzle. But as I added features it got harder and harder. Once I
couldn't fit the entire program in my head anymore, it was doomed.

This became a major motivation: how could I learn to write programs
bigger than would fit in my head? Instead of just reading about "How
to Program in C++", I started trying to find books about how to
*organize* programs. Sadly, the New Orleans B. Dalton bookstore wasn't
exactly a hotbed of software engineering resources.

Fast-forward several years, and a friend hands me a new book: Design
Patterns. Finally! The book I'd been looking for since I was a
teen ager. I read it cover to cover in one sitting. I still struggled
with my own programs, but it was such a relief to see that other
people were running into the same problems, and coming up with
solutions. I felt like I finally had a couple of tools to use
instead of just my bare hands.

In 2001, I finally got my dream job: software engineer at Electronic
Arts. I was going to be a real game programer! I couldn't wait to get
a look at some real game codebases and see the beautiful software
architecture they were using. Finally, I could see what a huge
shipping game codebase looked like. How could they make an enormous
game like Madden Football when it clearly was too big to fit in
anyone's head? What did they think of the Decorator pattern? Was
Observer as popular as I thought it would be?

> Here's a simple set of rules to determine if Singleton is the best
> solution to your problem:
> 1. No, it isn't.

What I saw surprised me. In many ways, the codebases weren't any
better at the architectural level than some of my own programs. Sure,
the *features* were incredible: great graphics, gameplay, audio.
Brilliant optimizations, clever use of the platform. But the
*architecture* that that brilliant code hung off of was often little
more than a pile of global variables. Coupling was so bad you often
couldn't even *define* modules, much less isolate them. As far as I
could tell, our engineers had read Design Patterns, but stopped after
the Singleton chapter.

As time went on and codebases got bigger, I started seeing
improvement. I worked on a bunch of different games, some newer some
older. I started to see patches of architectural brilliance, and among
those isolated patches, I started seeing patterns. Many of them were
patterns I'd seen described in Design Patterns, but some I'd never
encountered outside of games. My coworkers were burning
overtime just to cram tiny features into their awful legacy codebase
and I wanted to shake them and say, "Hey! Look over here! Look how
they did this! See how much easier things could be?"

This is what this book is about. It's my best attempt to harvest the
most useful patterns I've seen in game codebases and put them in an
easy to digest form so that hopefully my friends in the industry can
take the best of what other games are doing and apply it to their own
work.

## What's In Store
There are already dozens of game programming books out there. What's
different about this one? I'll explain by analogy.

Imagine a game codebase as a house. Graphics and sound are the
appliances and fixtures: a nice chandelier, kitchen sink, big picture
windows. This is the kind of stuff you can find in other books. It
matters, of course, but this book aims at something a bit more humble
and fundamental: the framing. Other books can teach you about windows
and fixtures, faucets and tubs. What I hope to cover in this book is
the joist and the arch, plumbing and wiring. Techniques that will let
you build an elegant and maintainable structure out of as few parts as
possible. Use them to frame the house, then consult those other books
to flesh it out.

> This is the tragedy of software architecture: the better it is, the
> less it's noticed. All people will notice the features that were
> enabled by it. As the Tao To Ching says: "When the Sage's work is
> done, the people say, "We did it all by ourselves!"

At the end of the day, people won't look at your home and say,
"Wow, nice wiring!" But without it, I can guarantee they'll notice
when your beautiful chandelier doesn't turn on.
   
## How it Relates to Design Patterns
> In my opinion, a more important relationship is to Christopher
> Alexander's original A Pattern Language. It's about houses-and-
> buildings architecture, not software architecture, but there's
> plenty of insight in there on how to build things that humans love
> for any creative person to learn from.

Any programming book with "Patterns" in its name clearly bears a
relationship to the classic Design Patterns: Elements of Reusable
Object-Oriented Software by Erich Gamma, Richard Helm, Ralph Johnson
and John Vlissides (usually referred to simply as the Gang of Four).

By calling this book "Game Programming Patterns", I certainly don't
intend to imply that the Gang of Four's book doesn't apply. On the
contrary: I believe any programmer doing architecture-level work
should be familiar with Design Patterns, regardless of the domain they
code for. I look at this book as an extension of Design Patterns. 

You can think of Design Patterns as The Joy of Cooking. Regardless of
what you're preparing, there will be recipes in there for you. But if
you've decided to focus on baking, this book on desserts and pastries
is a helpful addition. If you can only buy one book, buy theirs. Not
only will it serve you in greater staid than this one, but this book
often presumes at least a passing familiarity with this.

However, once you've got that on your bookshelf, I hope that I can
provide some more insight. Games are different from other software in
many ways. Time and sequencing are often a core part of a game's
architecture. Things must happen in the right order, and at the right
time. Most software's concept of time is only "the faster the better".

Games often use random generation to provide a varied play experience,
where your average commercial program will never once call `rand()`.
There's some interesting implications of software that can create
novelty on its own, and other patterns won't help there.

Once a game world is created or loaded, games must maintain and mutate
this complicated interacting world over time. In games, monsters may
run into each other, items may be picked, and bombs can blast enemies
and friends alike. Coordinating that kind of interaction is rare
outside of games. If you have two documents open in your word
processor, they don't talk to each other very much.

This book will cover patterns that help untangle some of the issues
unique to games. We'll also cover some patterns that are in the Gang
of Four's book, but with an emphasis on how they're relevant to games.
If you've read their book but had trouble applying it to your work in
games this book may help.

> *A Pattern Language* language covers human habitation from the
> largest to the smallest scale. An early pattern tells you where to
> place the cities in your country, useful advice for those of us
> terraforming new planets. A later pattern talks about how mismatched
> chairs are more appealing than a room full of identical furniture,
> explaining in real human terms why we love coffeeshops with funky 
> thrift store sofas.

Another difference is that this book is not just *design* patterns.
Some of the patterns will be smaller in scale than the architectural
patterns that the Gang of Four restricted themselves too. This, I
believe, follows more closely to Alexander's idea of a pattern
*language* that has patterns at all scales from the micro to the
macro.

## How To Read This
This book is essentially structured like a recipe book, because in
every way that counts, it *is* one.

This chapter introduces you to the material, and explains how the
rest of the book works. Its job is to provide context for the
patterns, how they relate to other books out there, and how it can be
applied to your coding. It tells you how to follow a recipe, prepare
the ingredients, and make sure you don't leave the oven on.

Following that is the recipes of the cookbook: the patterns
themselves. The patterns are grouped into several themes (courses?).
Each theme describes two opposing forces encountered when building a
game. The patterns within that theme are tools you can use to find a
balance between those forces.

For example, the "Communicating" theme is for patterns related
to how parts of a game communicate with each other. One force is that
game objects *must* interact: bullets must collide with enemies, the
hero must be able to play heroic sounds. At the same time, coupling is
generally bad and makes maintenance harder. The patterns in these
sections give you systems to allow modules to communicate with other
but in narrow ways that minimize the impact of the coupling.

Within these themes are the patterns. Each pattern is described in a
fairly fixed structure. This is so you can treat this book as a
reference and quickly find what you need.

Each pattern starts with an introduction. This describes the problem
that the pattern solves. This is first so that you can hunt through
the book quickly to find a pattern that will help you with your
current struggle. It's basically the sales pitch.

If you've bought the pitch, the next section describes each step and
part of the pattern. Because patterns must always be *applied* to
some problem to be used, they are described here in terms of an
example usage. If you're comfortable with your programming language,
this section may be all you need.

Following this is a longer implementation section. This takes the
example from the previous section and walks through an implementation
of the essence of the pattern. If this is the first time you've
encountered the pattern, this section will help you lock it into your
mind.

At this point, you've got the pattern down. The remaining sections
expand on that knowledge and provide more context. Every design choice
is a trade-off, and the implications section will try to make sure you
understand both the benefits and costs of choosing this pattern. The
applications section shows common places in a codebase where the
pattern is used. Related patterns shows how this pattern ties in with
other patterns. Some patterns have a couple of common refinements you
see in practice that aren't part of the core pattern, but are useful
in their own right. For patterns that have these, the last section
will lay them out.

## How the Book is Formatted
> Asides, extra information, and dumb jokes are placed in sidebars.
> Feel free to skip them if you want.

Since this is a fairly high-level book, I kept the formatting as
simple as possible. Code samples are shown using a `monospace font`.

Each pattern has a set of core components, the ingredients in its
recipe. Every implementation of the pattern will include these
components, so its important to note them all. The first time these
components are described they will be in **bold face**.

<p class='note'>
need real page number here
</p>

When another pattern in this book is mentioned, it will be
capitalized. Following that will be the page number where the pattern
is defined, like Service (123). Patterns described by the Gang of Four
in *Design Patterns* will also be capitalized, but instead of a page
number, will be shown like Decorator (GoF).
      
## About the Sample Code
Code samples in this book are in C++. This is not at all intended to
imply that these patterns are only useful in that language. Almost all
of them can be put to good use in any OOP language.

I chose C++ for a couple of reasons. Most importantly, it's currently
the most popular language for programming commercially shipping games.
It is the linqua franca for this industry. Moreso, the core C syntax
C++ is based on is also the basis for Java, C#, and many other
languages. With a little bit of effort, a programmer in any of these
languages should be able to understand the code samples here.

Keep in mind that the goal of this book is *not* to teach you C++.
The samples here are kept as simple as possible and don't represent
good C++ style or usage. Read the code samples for the idea being
expressed, not the code expressing it.

To avoid wasting space on code you've already seen or that isn't
relevant to the pattern, code will sometimes be omitted in examples.
When this occurs, an ellipsis will be placed in the sample to show
where the missing code goes.

Consider a function that will do some work and then return a value.
The pattern being explained is only concerned with the return value,
and not the work being done. In that case, the sample code will look
like:

    bool Update()
    {
        // do work...
        
        return IsDone();
    }

## Where to Go From Here
This book is an example of the fact that patterns are a constantly
changing and expanding part of software development. The content in
this book comes from harvesting ideas from other codebases out there,
and that process will continue well beyond when this is printed.

You are a core part of that process. As you develop your own patterns
and refine (or refute!) the patterns in this one, you contribute to
the software community. If you have suggestions, corrections, or other
feedback about what's in here, please get in touch.

