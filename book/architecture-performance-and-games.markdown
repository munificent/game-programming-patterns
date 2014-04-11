^title Architecture, Performance, and Games
^section Introduction

This chapter frames the rest of the book. I thought it might be helpful to give you some larger context about how I think about software architecture and how it applies to games. It may help you understand the rest of this book better. When you find yourself dragged into an argument about whether design patterns and software architecture suck or are awesome, it will give you some <span name="ammo">ammo</span>.

<aside name="ammo">

Note that I didn't presume which side you're on in that fight. Like any arms dealer, I've got weapons for all combatants here.

</aside>

However, like all frames, this chapter isn't the painting itself. If you just want some concrete information you can apply immediately to become a better programmer, I won't be offended if you skip this. Well, not much.

## What is Software Architecture?

<span name="won't">If</span> you read this book cover to cover, you won't come away knowing the linear algebra behind 3D graphics, or the calculus behind game physics. It won't show you how to alpha-beta prune your AI's search tree or simulate a room's reverberation in your audio playback.

<aside name="won't">

Wow, this paragraph would make a terrible ad for the book.

</aside>

Instead, it's about the code *between* all of that. It's less about writing code than it is about *organizing* code. We've got a bunch of terms to talk about. "Software architecture" and "design" are the big ones but "abstraction", "modularity", and a slew of others some more tied to fad methodologies than others all gravitate around the same idea.

Every program has *some* organization, even if it's just "jam the whole thing into `main()` and see what happens", so it's a lot more interesting to talk about what makes *good* architecture. How do we tell a good design from a bad one?

I've been mulling this question over pretty much the entire time I've been writing this book. Of course, we all have a solid intuition about good design. Everyone has had experiences dealing with *bad* design. We've all suffered through codebases so bad the best you can do for them is take them out back and put them out of their misery. Most us created quite a few of those as we learned to program.

A lucky few have had the opposite experience, a chance to work with beautifully designed code. The kind of codebase that feels like a perfectly appointed luxury hotel just waiting for you to take off your shoes and settle in.

What's the <span name="brace">difference</span> between the two?

<aside name="brace">

We all know the most important property is "does the program use the indentation style I happen to prefer?"

</aside>

For me, good design means that when I make a change, it's as if the entire program was crafted in anticipation of that change. I barely have to write a line of code and it slots in perfectly, solving my task while leaving not the slightest ripple on the placid surface of the code.

That sounds pretty, but it's not exactly actionable. "Just write your code so that changes don't disturb its placid surface." Right. Let me try to break that down a bit more. The first key piece is that *design is all about change*.

Someone has to be modifying the codebase. If <span name="zen">no one</span> is touching the code -- whether because it's perfect and complete, or so wretched no one will touch it -- its design doesn't matter. If no one is cracking open the source, its design is irrelevant.

<aside name="zen">

There's some Zen koan in here somewhere. "If a program falls in the woods and no one reads the source, does it make a sound?"

</aside>

Before you can change the code to add a new feature, or fix a bug, or whatever reason caused you to fire up your editor, you have to understand what the existing code is doing. You don't have to know the whole program, of course, but you need to <span name="ocr">load</aside> all of the pieces of it that are relevant to your problem into your primate brain.

<aside name="ocr">

It's weird to think that that is literally an OCR process.

</aside>

We tend to gloss over this part, but it's often the most time-consuming part of programming. If you think paging some data from RAM into disk is slow, try paging it into a simian cerebrum over a pair of optical nerves.

Once you've got all of the right context into your wetware, you think for a bit and figure out your solution. Most of the time, this is relatively straightforward. Understanding the problem and the parts of the code it touches is most of the work. Once that's done, the actual coding is sometimes nearly mechanical.

You beat your meaty fingers on the keyboard for a while until the right colored lights appear on screen and you're done, right? Not just yet! Before you write <span name="tests">tests</span> and send it off for code review, you often have some clean up to do.

<aside name="tests">

Did I say tests? Yes, I did. Some of a game's codebase is hard to write unit tests for, but a large fraction of it perfectly testable.

</aside>

You jammed a bit more code into your program, but you don't want the next person to come along to trip over the wrinkles you left throughout the code. Unless the change is minor, there's usually a bit of reorganization to do to make your new code integrate seamlessly with the rest of the code. If you do it right, the next person to come along won't be able to tell when a given line of code was written.

In short:

1. Come up with some change to make.
2. Load the relevant parts of the program into your head.
3. Figure out and implement the change.
4. Reorganize and clean up so the change fits in seamlessly.
5. Rinse, lather, repeat.

I think much of software architecture focuses on step two. Because the process of loading code into neurons is so painfully slow, it pays to come up with ways to minimize that. This book has an entire section on [*decoupling* patterns](decoupling-patterns.html), and a large chunk of *Design Patterns* is about the same idea.

You can define it a bunch of ways, but I like to think coupling between two pieces of code means you can't understand one without understanding the other. If you can decouple them, you can reason about each side independently. That's great because if one of those pieces is relevant to your problem, you only need to load it into your monkey brain and not both halves.

To me, this is a key goal of software architecture: *reduce the amount of knowledge you need to have in-cranium before you can make progress.*

Step three comes into play too, of course. Another definition of decoupling is that a *change* to one piece of code doesn't necessitate a change to another. We obviously need to change *something*, but the less coupling we have, the less that change ripples throughout the rest of the game.

## At What Cost?

This sounds great, right? Decouple everything and you'll be able to code like the wind. Each change will just mean touching one or two select points in the code base and you can dance across the surface of the codebase leaving nary a shadow on it.

This feeling is exactly why people get excited about abstraction, modularity, design patterns, and software architecture. A well-architected program really is a joyful experience to work in, and everyone loves having more productive developers. And good architecture makes a *huge* difference in productivity. It's hard to overstate how profound an effect it can have.

But, like all things in life, it doesn't come free. Good architecture takes real time, effort, and discipline. Every time you make a change or implement a feature, you have to work hard to integrate it gracefully into the rest of the program. You have to take great care to both organize the code well and *keep* it organized throughout the thousands of little changes that make up a development cycle.

You have to think about which parts of the program can be decoupled, and introduce abstractions at those points. Likewise, you have to determine where extensibility should be engineered in so future changes are easier to do.

People get really exciting about this. They envision future developers (or just their future self) stepping into the codebase and finding it open-ended, powerful and just beckoning to be extended. They imagine the One Game Engine To Rule Them All.

But this is where it starts to get tricky. Whenever you add a layer of abstraction or a place where extensibility is supported, you're speculating that you will need that abstraction or extensibility later.

Abstraction and extensibility add code and complexity to your game that take time to develop, debug, and maintain. That effort may pay off if you guess right and end up touching that code later. But predicting the future is *hard* and when that modularity doesn't end up being helpful, it quickly becomes actively harmful.

When people get overzealous about this, you get a codebase whose architecture has spiraled out of control. You've got interfaces and abstractions everywhere. Plug-in systems, abstract base classes, virtual methods galore and all sorts of extension points.

It takes you forever to trace through all of that scaffolding to find some real code that does something. When you need to make a change, sure, there's probably an interface there to help, but good luck finding it.

Codebases like this are what turn people *against* software architecture, and design patterns in particular. It's easy to get so wrapped up in the code itself that you lose sight of the fact that you're trying to ship a *game*. This tarpit sucks in countless hobby developers who spend years working on an "engine" without ever figuring out what it's an engine *for*.

## Performance and Speed

There's another critique of software architecture and abstraction that you hear sometimes, especially in game development: that it hurts your game's performance. It's a true observation.

Many patterns that make your code more flexible rely on virtual dispatch, interfaces, pointers, messages and <span name="templates">other mechanisms</span> that all have at least some runtime cost.

<aside name="templates">

One interesting counter-example is *templates*. Template metaprogramming can sometimes give you the abstraction of interfaces without any penalty at runtime.

</aside>

There's a reason for this. A lot of software architecture is about making your program more flexible. It's about making it take less effort to change it. That means encoding fewer assumptions in the program. You use interfaces so that your code works with *any* class that implements it instead of just the one that does today. You use <a href="observer.html" class="gof-pattern">observers</a> and <a href="event-queue.html" class="pattern">messaging</a> to let two parts of the game talk to each other so that tomorrow it can easily be three or four.

But performance is all about assumptions. The art of optimization thrives on concrete limitations. Can we safely assume we'll never have more than 256 enemies? Great, we can pack an ID into a single byte. Will we only call a method on one concrete type here? Good, we can inline it. Are all of the entities going to be the same class? Great, we can make a nice <a href="data-locality.html" class="pattern">contiguous array</a> of them in place.

This doesn't mean flexibility is bad, though! Good architecture lets us change our game quickly, and *developer* speed is absolutely vital for getting to a fun experience. No one, not even Will Wright, can come up with a balanced game design on paper. It needs constant iteration and experimentation.

The faster you can try out ideas and see how they feel, the more you can try. The more you try, the more likely you are to find something great. Even once you've found some fun mechanics, you need plenty of time for tuning to get it balanced and fun. Even a tiny imbalance can wreck the fun of a game.

There's no easy answer here. Making your program more flexible so you can prototype faster will have some performance cost. Likewise, optimizing your code will make it less flexible.

I will say that it's a lot easier to optimize the performance of a fun game than it is to get a bad game that's highly optimize and figure out how to make it fun. Sometimes the best solution may be to keep the code flexible until the design settles down and then tear out some of the abstraction later.

## The Good in Bad Code

That brings me to the next point which is that there's a time and place for different kinds of code. Much of this book is about making maintainable, clean code, so my allegience is pretty clearly on doing things the "right" way. But there's a lot of value in sloppy code too.

Writing well-architected code takes careful thought, and that translates to time. Moreso, *maintaining* a good architecture over the life of a project takes a lot of discipline and effort. You have to treat your codebase like a good camper does their campsite: always try to leave it a little better than you found it.

This is good when you're going to be living in and working on that code for a long time. But like I mentioned earlier, game design takes a lot of experimentation and exploration. Especially early on, it's common to write code that you *know* you'll throw away.

If you just have a gameplay idea to try out, spending time architecting it beautifully is a waste if it doesn't pan out. Worse, it takes more time before you can even try out the idea and see if it works.

Prototyping, slapping together some barely working code that's just functional enough to answer a design question is a perfectly solid programming practice. It's all about writing the right code for the context you're working in. Don't waste time on beautiful code that will get deleted tomorrow.

There is a very large caveat here though. If you write throwaway code, you must ensure you're able to throw it away. I've seen bad managers play this game time and time again:

Boss: "Hey, we've got this idea that we want to try out. Just a prototype, so don't feel you need to do it right. How quickly can you slap something together?"

Dev: "Well, if I cut lots of corners, don't test it, don't document it, and it has tons of bugs, I can give you some temp code in a few days."

Boss: "Great!"

*A few days pass...*

Boss: "Hey, that prototype is great, can you just clean it up a bit now and make it the real thing?"

You need to make sure the people using the <span name="throwaway">throwaway</span> code understand that even though it kind of looks like it works, it *cannot* be maintained and must be *rewritten*. If there's a *chance* you'll end up having to keep it around, you may have to just defensively write it well.

<aside name="throwaway">

One trick to ensuring your prototype code doesn't get turned into real code is to write it in another language than your game uses. That way you *have* to rewrite it before it can end up in your actual game.

</aside>

## Striking a Balance

It seems we have a few forces in play:

1. We want decoupling and abstraction so that working in the codebase is easy.
2. We want fast runtime performance.
3. We want to get features done quickly right now.

These forces are at least partially in opposition. Good architecture improves productivity over the long term, but maintaining it means every change requires a little more love afterwards to keep things clean.

The fastest way to implement something is rarely the fastest at runtime. Instead, optimization takes significant engineering time. Once it's done, it tends to calcify the codebase: highly optimized code is very inflexible and very difficult to change.

There's no simple clear answer here, just constraints and trade-offs. Based on the feedback I get, this is frustrating or disheartening to a lot of people. Especially to novices who just want to make a game, it's scary to hear "there is no right answer, just your choice of partially wrong ones."

But, to me, this is exciting. Look at any field that people dedicate entire passionate careers to mastering, and in the center you will almost always find a set of intertwined constraints. After all, if there was an easy answer, it would be boring. No one talks about their long distinguished career in <span name="ditch">ditch digging</span>.

<aside name="ditch">

Maybe they do; I didn't research that analogy. For all I know there could be avid ditch digging hobbyists, ditch digging conventions, and a whole subculture around it. Who am I to judge?

</aside>

In fact, it's like a well-designed game in many ways. Because these constraints are so well-balanced against each other, the game of programming can be something you spend a lifetime mastering. I'm nearing my second decade of professional programming and every day I still feel like I'm learning something new.

## Simplicity

Lately, I feel like if there is any tactic that at least partially solves all of these constraints, it's *simplicity*. In my coding today, I try very hard to write the simplest, most direct solution to the problem. The kind of code where after you've written it, what it does is obvious and you can't imagine it being done any other way.

I try to get the data structures and algorithms right (in about that order) and then go from there. I find if I can keep things simple, there's less code overall. That means less code to load into my head in order to change it.

It often runs fast because there's simply not as much overhead and not much code to execute. (This certainly isn't always the case though. `while(true) {}` isn't much code, but it won't be finishing any time soon either.)

However, note that I'm saying <span name="simple">simple code</span> takes less time to *read*, not to *write*. You'd think it would be true for both, but the opposite is more often the case. The simpler the solution, the longer it takes to distill.

<aside name="simple">

Blaise Pascal famously ended a letter with, "I would have written a shorter letter, but I did not have the time."

Closer to home, I'll note that every time I revise a chapter in this book, it gets shorter. Some chapters are tightened by 20% by the time they're done.

</aside>

When you're presented with a problem, it's usually in the form of a pile of use cases. You want the X to do Y when Z, but W when A, and... Just a long list of different example behaviors you have in mind.

To solve that problem elegantly, you need to find a generalization that is simple and yet still correctly covers all of those cases. It's a great feeling when you pull it off, but, again like game design, you often have to discard a bunch of failed attempts in the process. It's much like science where you have the data but you may go through a number of hypotheses before you find a simple theory that fits.

So before we get to the real meat of this book and start talking about some patterns, I'll leave you with a few bits of advice:

* Abstraction and decoupling make evolving your program faster and easier, but avoid doing it in areas that change infrequently and don't need that flexibility.

* Sometimes you sacrifice flexibility in order to optimize for performance. Prefer doing that later in your development cycle (but <span name="think">*think*</span> about performance throughout it).

<aside name="think">

Trust me, two months before shipping is *not* when you want to start worrying about that nagging little "game only runs at 1 FPS" problem.

</aside>

* It's vital to iterate quickly to explore your game's design space, but don't go so fast that you leave a mess behind you. You'll have to live with it, after all.

* If you are going to ditch code, don't waste time making it pretty. Rock stars trash hotel rooms because they know they're going to check out the next day.

* But, most of all, <span name="fun">I think</span> **the best way to make something fun is to have fun making it.**

<aside name="fun">

I'd like to think this book itself is an example of that.

</aside>
