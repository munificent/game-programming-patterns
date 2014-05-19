#include <iostream>

#define WHITE 0
#define BLACK 1

//^1
class Framebuffer
{
public:
  Framebuffer() { clear(); }

  void clear()
  {
    for (int i = 0; i < WIDTH * HEIGHT; i++)
    {
      pixels_[i] = WHITE;
    }
  }

  void draw(int x, int y)
  {
    pixels_[(WIDTH * y) + x] = BLACK;
  }

  const char* getPixels()
  {
    return pixels_;
  }

private:
  static const int WIDTH = 160;
  static const int HEIGHT = 120;

  char pixels_[WIDTH * HEIGHT];
};
//^1

namespace Unbuffered
{
  //^2
  class Scene
  {
  public:
    void draw()
    {
      buffer_.clear();

      buffer_.draw(1, 1);
      buffer_.draw(4, 1);
      buffer_.draw(1, 3);
      buffer_.draw(2, 4);
      buffer_.draw(3, 4);
      buffer_.draw(4, 3);
    }

    Framebuffer& getBuffer() { return buffer_; }

  private:
    Framebuffer buffer_;
  };
  //^2

  void InterruptMiddleOfDraw()
  {
    Framebuffer buffer_;

    //^3
    buffer_.draw(1, 1);
    buffer_.draw(4, 1);
    // <- Video driver reads pixels here!
    buffer_.draw(1, 3);
    buffer_.draw(2, 4);
    buffer_.draw(3, 4);
    buffer_.draw(4, 3);
    //^3
  }
}

namespace Buffered
{
  //^4
  class Scene
  {
  public:
    Scene()
    : current_(&buffers_[0]),
      next_(&buffers_[1])
    {}

    void draw()
    {
      next_->clear();

      next_->draw(1, 1);
      // ...
      next_->draw(4, 3);

      swap();
    }

    Framebuffer& getBuffer() { return *current_; }

  private:
    void swap()
    {
      // Just switch the pointers.
      Framebuffer* temp = current_;
      current_ = next_;
      next_ = temp;
    }

    Framebuffer  buffers_[2];
    Framebuffer* current_;
    Framebuffer* next_;
  };
  //^4
}

namespace UnbufferedSlapstick
{
  class Stage;

  //^5
  class Actor
  {
  public:
    Actor() : slapped_(false) {}

    virtual ~Actor() {}
    virtual void update() = 0;

    void reset()      { slapped_ = false; }
    void slap()       { slapped_ = true; }
    bool wasSlapped() { return slapped_; }

  private:
    bool slapped_;
  };
  //^5

  //^6
  class Stage
  {
  public:
    void add(Actor* actor, int index)
    {
      actors_[index] = actor;
    }

    void update()
    {
      for (int i = 0; i < NUM_ACTORS; i++)
      {
        actors_[i]->update();
        actors_[i]->reset();
      }
    }

  private:
    static const int NUM_ACTORS = 3;

    Actor* actors_[NUM_ACTORS];
  };
  //^6

  //^7
  class Comedian : public Actor
  {
  public:
    //^omit
    Comedian() : name_("") {}
    Comedian(const char* name) : name_(name) {}
    //^omit
    void face(Actor* actor) { facing_ = actor; }

    virtual void update()
    {
      //^omit
      if (wasSlapped()) std::cout << name_ << " was slapped" << std::endl;
      //^omit
      if (wasSlapped()) facing_->slap();
    }

  private:
    //^omit
    const char* name_;
    //^omit
    Actor* facing_;
  };
  //^7

  void sample1()
  {
    //^8
    Stage stage;

    Comedian* harry = new Comedian();
    Comedian* baldy = new Comedian();
    Comedian* chump = new Comedian();

    harry->face(baldy);
    baldy->face(chump);
    chump->face(harry);

    stage.add(harry, 0);
    stage.add(baldy, 1);
    stage.add(chump, 2);
    //^8

    //^9
    harry->slap();

    stage.update();
    //^9

    //^10
    stage.add(harry, 2);
    stage.add(baldy, 1);
    stage.add(chump, 0);
    //^10
  }

  void testComedy1(int a, int b, int c)
  {
    std::cout << std::endl << "test" << std::endl;

    Stage stage;

    Comedian* larry = new Comedian("larry");
    Comedian* curly = new Comedian("curly");
    Comedian* shemp = new Comedian("shemp");

    larry->face(curly);
    curly->face(shemp);
    shemp->face(larry);

    stage.add(larry, a);
    stage.add(curly, b);
    stage.add(shemp, c);

    larry->slap();
    for (int i = 0; i < 3; i++)
    {
      std::cout << "update" << std::endl;
      stage.update();
    }
  }

  void testComedy()
  {
    testComedy1(0, 1, 2);
    testComedy1(2, 1, 0);
  }
}

namespace BufferedSlapstick
{
  class Stage;

  //^11
  class Actor
  {
  public:
    Actor() : currentSlapped_(false) {}

    virtual ~Actor() {}
    virtual void update() = 0;

    void swap()
    {
      // Swap the buffer.
      currentSlapped_ = nextSlapped_;

      // Clear the new "next" buffer.
      nextSlapped_ = false;
    }

    void slap()       { nextSlapped_ = true; }
    bool wasSlapped() { return currentSlapped_; }

  private:
    bool currentSlapped_;
    bool nextSlapped_;
  };
  //^11

  //^12
  class Stage
  {
    //^omit
  public:
    void add(Actor* actor, int index) { actors_[index] = actor; }

    //^omit
    void update()
    {
      for (int i = 0; i < NUM_ACTORS; i++)
      {
        actors_[i]->update();
      }

      for (int i = 0; i < NUM_ACTORS; i++)
      {
        actors_[i]->swap();
      }
    }

    // Previous Stage code...
    //^omit
  private:
    static const int NUM_ACTORS = 3;

    Actor* actors_[NUM_ACTORS];
    //^omit
  };
  //^12

  class Comedian : public Actor
  {
  public:
    //^omit
    Comedian() : name_("") {}
    Comedian(const char* name) : name_(name) {}
    //^omit
    void face(Actor* actor) { facing_ = actor; }

    virtual void update()
    {
      //^omit
      if (wasSlapped()) std::cout << name_ << " was slapped" << std::endl;
      //^omit
      if (wasSlapped()) facing_->slap();
    }

  private:
    //^omit
    const char* name_;
    //^omit
    Actor* facing_;
  };

  void sample1()
  {
    Stage stage;

    Comedian* harry = new Comedian();
    Comedian* baldy = new Comedian();
    Comedian* chump = new Comedian();

    harry->face(baldy);
    baldy->face(chump);
    chump->face(harry);

    stage.add(harry, 0);
    stage.add(baldy, 1);
    stage.add(chump, 2);

    harry->slap();

    stage.update();
  }
}

namespace SwapOffset
{
  //^13
  class Actor
  {
  public:
    static void init() { current_ = 0; }
    static void swap() { current_ = next(); }

    void slap()        { slapped_[next()] = true; }
    bool wasSlapped()  { return slapped_[current_]; }

  private:
    static int current_;
    static int next()  { return 1 - current_; }

    bool slapped_[2];
  };
  //^13
}
