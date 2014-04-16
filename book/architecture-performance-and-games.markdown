^title Architecture, Performance, and Games
^section Introduction

Before we plunge headfirst into a pile of patterns, I thought it might help to give you some larger context about how I think about software architecture and how it applies to games. It may help you understand the rest of this book better. If nothing else, when you get dragged into an <span name="ammo">argument</span> about how much design patterns and software architecture suck or are awesome, it will give you some ammo to use.

<aside name="ammo">

Note that I didn't presume which side you're taking in that fight. Like any arms dealer, I have wares to offer to all combatants here.

</aside>

## What is Software Architecture?

<span name="won't">If</span> you read this book cover to cover, you won't come away knowing the linear algebra behind 3D graphics, or the calculus behind game physics. It won't show you how to alpha-beta prune your AI's search tree or simulate a room's reverberation in your audio playback.

<aside name="won't">

Wow, this paragraph would make a terrible ad for the book.

</aside>

Instead, it's about the code *between* all of that. It's less about writing code than it is about *organizing* it. Every program has *some* organization, even if it's just "jam the whole thing into `main()` and see what happens", so I think it's more interesting to talk about what makes for *good* organization. How do we tell a good architecture from a bad one?

I've been mulling this question over for about five years. Of course, like you, I have an intuition about good design. We've all suffered through codebases so bad the best you could hope do for them is take them out back and put them out of their misery. Most us created quite a few of those as we learned to program.

A lucky few have had the opposite experience, a chance to work with beautifully designed code. The kind of codebase that feels like a perfectly appointed luxury hotel festooned with concierges waiting eagerly on your every whim. What's the difference between the two?

### What is *good* software architecture?

For me, good design means that when I make a change, it's as if the entire program was crafted in anticipation of it. I can solve a task with just a few choice function calls that slot in perfectly, leaving not the slightest ripple on the placid surface of the code.

That sounds pretty, but it's not exactly actionable. "Just write your code so that changes don't disturb its placid surface." Right.

Let me try to break that down a bit more. The first key piece is that *design is about change*. Someone has to be modifying the codebase. If <span name="zen">no one</span> is touching the code -- whether because it's perfect and complete, or so wretched no one will sully their text editor with it -- its design is irrelevant. The measure of a design is how easily it accommodates changes. With no changes, it's a runner who never runs a race.

<aside name="zen">

There's some Zen koan in here somewhere. "If a program falls in the woods and no one reads the source, does it compile to a sound?"

</aside>

### How do you make a change?

Before you can change the code to add a new feature, or fix a bug, or whatever reason caused you to fire up your editor, you have to understand what the existing code is doing. You don't have to know the whole program, of course, but you need to <span name="ocr">load</aside> all of the pieces of it that are relevant to your problem into your primate brain.

<aside name="ocr">

It's weird to think that that is literally an OCR process.

</aside>

We tend to gloss over this step, but it's often the most time-consuming part of programming. If you think paging some data from RAM into disk is slow, try paging it into a simian cerebrum over a pair of optical nerves.

Once you've got all of the right context into your wetware, you think for a bit and figure out your solution. There can be a lot of back and forth here, but often this is relatively straightforward. Once you understand the problem and the parts of the code it touches, the actual coding is sometimes nearly mechanical.

You beat your meaty fingers on the keyboard for a while until the right colored lights appear on screen and you're done, right? Not just yet! Before you write <span name="tests">tests</span> and send it off for code review, you often have some clean up to do.

<aside name="tests">

Did I say "tests"? Oh, yes, I did. Some of a game's codebase is hard to write unit tests for, but a large fraction of it is perfectly testable.

I won't get on a soapbox here, but I'll ask you to consider doing more automated testing if you aren't already. Don't you have better things to do than manually test stuff over and over again?

</aside>

You jammed a bit more code into your game, but you don't want the next person to come along to trip over the wrinkles you left throughout the source. Unless the change is minor, there's usually a bit of reorganization to do to make your new code integrate seamlessly with the rest of the program. If you do it right, the next person to come along won't be able to tell when any line of code was written.

In short, when you have a change to make, you:

1. Load the relevant parts of the program into your head.
2. Figure out and implement the change.
3. Reorganize and clean up so the change fits in seamlessly.
4. Rinse, lather, repeat.

### How can decoupling help?

I think much of software architecture focuses on step one. Loading code into neurons is so painfully slow that it pays to find strategies to reduce the volume of it. This book has an entire section on [*decoupling* patterns](decoupling-patterns.html), and a large chunk of *Design Patterns* is about the same idea.

You can define "decoupling" a bunch of ways, but I think if two pieces of code are coupled, it means you can't understand one without understanding the other. If you *de*couple them, you can reason about either side independently. That's great because if one of those pieces is relevant to your problem, you only need to load it into your monkey brain and not the other half too.

To me, this is a key goal of software architecture: *minimize the amount of knowledge you need to have in-cranium before you can make progress.*

Steps two and three come into play too, of course. Another definition of decoupling is that a *change* to one piece of code doesn't necessitate a change to another. We obviously need to change *something*, but the less coupling we have, the less that change ripples throughout the rest of the game.

## At What Cost?

This sounds great, right? Decouple everything and you'll be able to code like the wind. Each change will just mean touching one or two select methods and you can dance across the surface of the codebase leaving nary a shadow on it.

This feeling is exactly why people get excited about abstraction, modularity, design patterns, and software architecture. A well-architected program really is a joyful experience to work in, and everyone loves having more productive developers. And good architecture makes a *huge* difference in productivity. It's hard to overstate how profound an effect it can have.

But, like all things in life, it doesn't come free. Good architecture takes real time, effort, and discipline. Every time you make a change or implement a feature, you have to work hard to integrate it gracefully into the rest of the program. You have to take great care to both <span name="maintain">organize</span> the code well and *keep* it organized throughout the thousands of little changes that make up a development cycle.

<aside name="maintain">

The second half of this -- maintaining your design -- deserves special attention. I've seen many programs start out beautiful and then die a death of a thousand cuts as programmers add "just one tiny little hack" to it over and over again.

Like gardening, it's not enough to put in new plants. You must also weed and prune.

</aside>

You have to think about which parts of the program should be decoupled, and introduce abstractions at those points. Likewise, you have to determine where extensibility should be engineered in so future changes are easier to do.

People get really exciting about this. They envision future developers (or just their future self) stepping into the codebase and finding it open-ended, powerful and just beckoning to be extended. They imagine the One Game Engine To Rule Them All.

But this is where it starts to get tricky. Whenever you add a layer of abstraction or a place where extensibility is supported, you're *speculating* that you will need that flexibility later. You're adding code and complexity to your game that takes time to develop, debug, and maintain.

That effort pays off if you guess right and end up touching that code later. But predicting the future is *hard* and when that modularity doesn't end up being helpful, it quickly becomes actively harmful.

When people get overzealous about this, you get a codebase whose architecture has spiraled out of control. You've got interfaces and abstractions everywhere. Plug-in systems, abstract base classes, virtual methods galore and all sorts of extension points.

It takes you forever to trace through all of that scaffolding to find some real code that does something. When you need to make a change, sure, there's probably an interface there to help, but good luck finding it. In theory, all of this decoupling means you have less code to understand before you can extend it, but the layers of abstraction themselves end up filling up your mental scratch disk.

Codebases like this are what turn people *against* software architecture, and design patterns in particular. It's easy to get so wrapped up in the code itself that you lose sight of the fact that you're trying to ship a *game*. This tarpit sucks in countless developers who spend years working on an "engine" without ever figuring out what it's an engine *for*.

## Performance and Speed

There's another critique of software architecture and abstraction that you hear sometimes, especially in game development: that it hurts your game's performance. It's a valid observation.

Many patterns that make your code more flexible rely on virtual dispatch, interfaces, pointers, messages and <span name="templates">other mechanisms</span> that all have at least some runtime cost.

<aside name="templates">

One interesting counter-example is templates in C++. Template metaprogramming can sometimes give you the abstraction of interfaces without any penalty at runtime.

There's a spectrum of flexbility here. When you write code to call a concrete method in some class, you're fixing that class at *author* time -- you've hard-coded which class you call into. When you go through a virtual method or interface, the class that gets called isn't known until *runtime*. That's much more flexible, but implies some runtime overhead.

Template metaprogramming is somewhere between the two. There, you make the decision of which class to call at *compile time* when the template is instantiated.

</aside>

There's a reason for this. A lot of software architecture is about making your program more flexible. It's about making it take less effort to change it. That means encoding fewer assumptions in the program. You use interfaces so that your code works with *any* class that implements it instead of just the one that does today. You use <a href="observer.html" class="gof-pattern">observers</a> and <a href="event-queue.html" class="pattern">messaging</a> to let two parts of the game talk to each other so that tomorrow it can easily be three or four.

But performance is all about assumptions. The art of optimization thrives on concrete limitations. Can we safely assume we'll never have more than 256 enemies? Great, we can pack an ID into a single byte. Will we only call a method on one concrete type here? Good, we can statically dispatch or inline it. Are all of the entities going to be the same class? Great, we can make a nice <a href="data-locality.html" class="pattern">contiguous array</a> of them.

This doesn't mean flexibility is bad, though! It lets us change our game quickly, and *developer* speed is absolutely vital for getting to a fun experience. No one, not even Will Wright, can come up with a balanced game design on paper. It demands iteration and experimentation.

The faster you can try out ideas and see how they feel, the more you can try. The more you try, the more likely you are to find something great. Even after you've found the right mechanics, you need plenty of time for tuning. A tiny imbalance can wreck the fun of a game.

There's no easy answer here. Making your program more flexible so you can prototype faster will have some performance cost. Likewise, optimizing your code will make it less flexible.

My experience, though is that it's a easier to make a fun game fast than it is to a fast game fun. One compromise is to keep the code flexible until the design settles down and then tear out some of the abstraction later to improve your performance.

## The Good in Bad Code

That brings me to the next point which is that there's a time and place for different styles of coding. Much of this book is about making maintainable, clean code, so my alliegence is pretty clearly to doing things the "right" way., but there's a lot of value in sloppy code too.

Writing well-architected code takes careful thought, and that translates to time. Moreso, *maintaining* a good architecture over the life of a project takes a lot of discipline and effort. You have to treat your codebase like a good camper does their campsite: always try to leave it a little better than you found it.

This is good when you're going to be living in and working on that code for a long time. But, like I mentioned earlier, game design requires a lot of experimentation and exploration. Especially early on, it's common to write code that you *know* you'll throw away.

If you just want to find out if some gameplay idea plays right at all, spending time architecting it beautifully means burning more time before you can get it on screen and get some feedback. If it ends up not working, that time spent making the code elegant goes to waste when you delete it.

Prototyping -- slapping together code that's just barely functional enough to answer a design question -- is a perfectly legitimate programming practice. There is a very large caveat, though. If you write throwaway code, you *must* ensure you're able to throw it away. I've seen bad managers play this game time and time again:

> Boss: "Hey, we've got this idea that we want to try out. Just a prototype, so don't feel you need to do it right. How quickly can you slap something together?"

> Dev: "Well, if I cut lots of corners, don't test it, don't document it, and it has tons of bugs, I can give you some temp code in a few days."

> Boss: "Great!"

*A few days pass...*

> Boss: "Hey, that prototype is great, can you just spend a few hours cleaning it up a bit now and we'll call it the real thing?"

You need to make sure the people using the <span name="throwaway">throwaway</span> code understand that even though it kind of looks like it works, it *cannot* be maintained and must be *rewritten*. If there's a *chance* you'll end up having to keep it around, you may have to just defensively write it well.

<aside name="throwaway">

One trick to ensuring your prototype code doesn't get turned into real code is to write it in another language than your game uses. That way you *have* to rewrite it before it can end up in your actual game.

</aside>

## Striking a Balance

We have a few forces in play:

1. We want nice architecture so the code is easier to understand.
2. We want fast runtime performance.
3. We want to get features done quickly right now.

These forces are at least partially in opposition. Good architecture improves productivity over the long term, but maintaining it means every change requires a little more effort to keep things clean.

The implementation that's quickest to write is rarely the quickest to *run*. Instead, optimization takes significant engineering time. Once it's done, it tends to calcify the codebase: highly optimized code is very inflexible and very difficult to change.

There's always pressure to get today's work done today and worry about everything else tomorrow. But if cram in features as quickly as we can, our codebase will amass a pile of hacks, bugs, and inconsistencies that will drain our future productivity away.

There's no simple answer here, just trade-offs. Based on the feedback I get, this disheartens a lot of people. Especially for novices who just want to make a game, it's intimidating to hear, "There is no right answer, just different kinds of wrong ones."

But, to me, this is exciting! Look at any field that people dedicate careers to mastering, and in the center you will almost always find a set of intertwined constraints. After all, if there was an easy answer, it would be boring. There's no point is spending your life's passion on a solved problem. You don't hear of someone's distinguished career in <span name="ditch">ditch digging</span>.

<aside name="ditch">

Maybe they do; I didn't research that analogy. For all I know there could be avid ditch digging hobbyists, ditch digging conventions, and a whole subculture around it. Who am I to judge?

</aside>

In fact, to me this has much in common with games themselves. A game like chess can never be mastered because all of the pieces are so perfectly balanced against one another. This means you can spend your life exploring the vast space of viable strategies.

## Simplicity

Lately, I feel like if there is any tactic that eases these constraints, it's *simplicity*. In my code today, I try very hard to write the cleanest, most direct solution to the problem. The kind of code where after you read it, you understand exactly what it does and you can't imagine any other possible solution.

I aim to get the data structures and algorithms right (in about that order) and then go from there. I find if I can keep things simple, there's less code overall. That means less code to load into my head in order to change it.

It often runs fast because there's simply not as much overhead and not much code to execute. (This certainly isn't always the case though. `while(true) {}` isn't much code, but it won't be finishing any time soon either.)

However, note that I'm saying <span name="simple">simple code</span> takes less time to *read*, not to *write*. You'd think it would be true for both, but the opposite is more often the case. To get to an elegant minimal solution, you have to spend more time distill the code down to its essence.

<aside name="simple">

Blaise Pascal famously ended a letter with, "I would have written a shorter letter, but I did not have the time."

Closer to home, I'll note that every time I revise a chapter in this book, it gets shorter. Some chapters are tightened by 20% by the time they're done.

</aside>

We're rarely presented with a clean simple problem. Instead, it's a pile of use cases. You want the X to do Y when Z, but W when A, and so on. In other words, a long list of different example behaviors.

The solution that takes the least mental effort is to just code up those use cases one at a time. If you look at novice programmers, that's what they often do: they churn out reams of conditional logic for each case they've considered.

But there's nothing elegant in that, and code like that tends to fall over when presented with something even slightly different that the exact examples the coder had in mind. When we think of elegant solutions, what we often have in mind is a *general purpose* one: a small bit of logic that still correctly covers all of our use cases.

Finding that is a bit like pattern matching or solving a puzzle. It takes effort to see through the scattering of exemplary use cases to the hidden order underlying them all. It's a great feeling when you pull it off.

## Get On With It, Already

Almost everyone skips the introductory chapters, so I congratulate you on making it this far. I don't have much in return for your patience, but I'll offer up a few bits of advice that I hope may be useful to you:

* Abstraction and decoupling make evolving your program faster and easier, but don't waste time doing it unless you're confident the code in question needs that flexibility.

* <span name="think">Think</span> about and design for performance throughout your development cycle, but put off the low-level nitty gritty optimizations that lock assumptions into your code as late as possible.

<aside name="think">

Trust me, two months before shipping is *not* when you want to start worrying about that nagging little "game only runs at 1 FPS" problem.

</aside>

* Move quickly to explore your game's design space, but don't go so fast that you leave a mess behind you. You'll have to live with it, after all.

* If you are going to ditch code, don't waste time making it pretty. Rock stars trash hotel rooms because they know they're going to check out the next day.

* But, most of all, **if you want to make something fun, have fun making it!**
