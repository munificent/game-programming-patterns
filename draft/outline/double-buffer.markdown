^title Double Buffer
^theme Sequencing
^outline

## Intent
*Enable a series of sequential operations to appear to occur 
simultaneously.*

## Motivation

*   computers are essentially serial: they do one thing at a time.
    if your game has mountains in the distance, trees closer, and a
    couple of monsters roaming on screen, those are all drawn one at
    a time each frame. if the user saw them drawn like that, illusion
    of a coherent scene would be broken. instead, the whole series of
    rendering calls needs to appear to happen instantly.

### How A Graphics System Works

*   monitor refreshes every 60th of a second regardless of what
    program is doing. it needs to know what to display
*   answer is a framebuffer: a chunk of ram that represents the values
    of each pixel
*   if you write to the framebuffer while the screen is refreshing 
    you'll get tearing
*   even if you time it right, unless you can draw 
    *everything* in one v-blank, the user would see your program draw
    stuff little bits at a time

### What's Behind the Curtain

*   analagous to a play. to change the stage setting, they close the
    curtain. this way the audience doesn't see the change. they just
    see Scene 1, Curtain, Scene 2.
*   better than watching stagehands drag props around, but the 
    audience still has to stare at a curtain.
*   imagine this: behind the audience, we set up a projector and a
    digital camera. we point the camera at the stage. right at the
    end of scene 1, the camera takes a picture. the curtain (white)
    drops and right then the projector turns on and projects the
    picture onto the curtain.
*   to the audience, they never saw the curtain at all. scene 1 just
    froze. the entire time the stagehands are setting up scene 2,
    the audience is looking at the *previous* scene, frozen in time.

### Back To The Graphics

*   that's how graphics work. there are *two* framebuffers: one is
    what the audience sees, and one is where the stagehands are
    building the next scene. when the stagehands are done, the buffers
    are *swapped* and the new one becomes the current scene, and the
    old one is now offscreen and can be drawn on.
*   do this once every thirtieth of a second and the user sees smooth
    animation.
*   this is the basic idea behind double buffering: you have two work
    areas, the current one and the next one. the next one is worked
    on serially. meanwhile, all external systems only see the
    unchanging current one. once the new one is done, a quick switch
    is done and now it's current. this frees up the old current one
    to become the new work area.

## The Pattern

A **buffered class** encapsulates a **buffer**: a piece of state that
can be modified. This buffer is edited incrementally, but we want all
outside code to see the edit as a single atomic change. To do this,
the class keeps *two* instances of the buffer, a **next buffer** and a 
**current buffer**.

When information is read *from* a buffer, it is always from the
*current* buffer. When information is *written* to a buffer, it occurs
on the *next* buffer. When the changes are complete, a **swap**
operation swaps the next and current buffers instantly so that the new
buffer is now publicly visible. The old current buffer is now
available to be reused as the new next buffer.

## When to Use It

*   need to modify state incrementally

*   other stuff will be accessing that state during that time

*   want the modification to appear instantaneous or simultaneous

## Keep in Mind

*   doubles the amount of memory needed for state

## Sample Code

### A Simple Graphics System

*   a bit hard to show a typical sample because it relies on the
    video hardware calling into our code while we're doing something.
    this will be a bit hand-wavey.

*   we'll be showing a very simple graphics system that lets you draw
    rectangles onto a canvas. first, canvas:
    
    canvas class
        clear()
        drawRect(x, y, width, height)
        getFrameBuffer();

*   drawRect() does what you think. getFrameBuffer(); gets the raw
    memory for the canvas. this is what the video device needs to
    render.
    
*   wrapping this is a scene class. it's job is to draw all of the
    stuff we expect to see on screen.
    
    scene class
        mCanvas
        draw()
            bunch of canvas drawRect calls
        getCanvas()
    
*   so, every frame, the game tells the scene to draw. it clears the
    canvas and then draws a bunch of rectangles.
    
*   if we just went with this, we'd have tearing problems or at the
    very least would see the scene in the middle of being drawn. so,
    double-buffering:
    
    scene class
        mCurrentCanvas*, mNextCanvas*
        draw()
            draws to mNextCanvas
            calls swap at end
        swap()
            swaps pointers
        getCanvas()
            gets mCurrentCanvas
            
*   now scene has two canvases. the publicly visible one is never
    drawn directly onto. this solves our tearing problems.
    
    there's another private canvas that is the working area. draw
    draws onto that. then, at the very end, it calls swap(). swap
    just switches the two pointers, making the newly drawn canvas now
    the public one. also, the old public one is now hidden, so we can
    reuse it as our new work area.

### Not Just For Graphics

*   what you saw was the canonical double buffer use case, but far 
    from the only one. to give you an idea of how the pattern is
    generally useful, let's cover another example that couldn't be
    more removed from graphics: ai.
    
#### Artificial Unintelligence

*   example has three slapstick comedian actors. each frame, they get
    update call. during that, can send messages and respond to 
    messages. update should be simultaneous: order that actors update
    should not matter.
    
    actor class
        update() = 0
        mWorld
        receiveMessage(int message)
            mMessage = message
        getMessage
    
*   all of the actors in the game are held in a stage object, so we
    can keep track of them
    
    stage class
        actors[]
        update()
            update each actor
        send(int actor, int message)
            actors[actor].message = message
            
*   our example actors interact

    class Moe
        update()
            if !done
                world->send(LARRY, SLAP)
                done = true
                
    class Larry
        update()
            if message == SLAP
                world->send(CURLY, EYE_POKE)
    
    class Curly
        update()
            if message == EYE_POKE
                print "ow!"
    
#### Roll the Film

*   let's say our actor collection on the stage is ordered like this:
    Moe, Larry, Curly
    
*   when we update, we update Moe first. he sends the slap to Larry.
    next we update Larry. since he got the slap, he sends the eye
    poke to curly. finally we update curly, who receives the poke and
    cries out. this all happens in one frame.
    
*   now lets re-order the collection to Curly, Larry, Moe. the first
    update, Curly does nothing since he hasn't gotten a message. next
    Larry does nothing. finally Moe slaps Larry. the *next* frame,
    curly still does nothing, but Larry gets the slap Moe sent on the
    last frame and pokes Curly. on the third frame, Curly finally
    gets the poke and shouts.
    
*   so we get visibly different behavior, just by reordering. goes
    against requirement that update is simultaneous.
    
#### Buffered Messages
    
*   solve by double-buffering messages. actor:

    actor class
        update() = 0
        mWorld
        receiveMessage(int message)
            mNextMessage = message
        getMessage
            return mCurrentMessage
        swap()
            mCurrent = mNext
            mNext = NONE
    
*   now when an actor receives message, during update, it will go into
    the next buffer, which won't be looked at until the next frame.
    
*   after the stage updates() all actors, it swaps the buffers on them
    all.
    
*   *show that actors now behave same regardless of collection order
    
## Design Alternatives

### How are the Buffers Swapped?

*   swap pointers

    *   quickest way to swap

    *   may not work for systems that expect "current" buffer to be in
        a certain place in memory
        
    *   existing data on next buffer will be from two frames ago, not
        last frame.
    
*   copy buffer data

    *   can take time. need to ensure nothing is looking at buffers
        while this is happening
        
    *   means data on next buffer is never more than one frame old

### What is the Granulatiry of the Buffer?

*   in render example, the entire buffer is a single object that there
    are two instances of. in actor example, each actor has its own
    tiny double buffer.
    
*   monolithic

    *   simplifies swapping. don't need to interate through collection
        doing incremental swap. usually faster.
        
*   distributed

    *   keeps "double-buffer" implementation detail encapsulated 
        within the object that needs it
        
    *   swapping can be slower. must tell each object to swap.
    
        may be able to optimize by having a global "swap index":
        
        class actor
            messages[2]
            static sCurrentIndex
            static sNextIndex
            
            static swap
                swap current and next indices
        
            getMessage() return messages[current]
            receiveMessage() messages[next] = value
            
## See Also

*   unix pipeline model allows processes to send information to each
    other reliably even though they run in parallel. to do so,
    information on pipeline is queued. sender adds to queue, receiver
    reads from it. the queue acts as buffer to decouple processes
    from each other. double buffer pattern is basically a simplified
    version of this where the queue is always two objects deep.
    
*   SwapBuffers() in opengl, EndDraw() in XNA
