#include <iostream>

#include "common.h"

enum Joystick
{
  DIR_NONE,
  DIR_LEFT,
  DIR_RIGHT
};

class Controller
{
public:
  static Joystick getJoystickDirection()
  {
    // Determine which direction the user is currently
    // pressing on the joystick...
    return DIR_NONE;
  }
};

class Sprite
{
};

class Graphics
{
public:
  void draw(Sprite& sprite, int x, int y)
  {
    // Draw the sprite at the given position...
  }
};

class Volume
{
};

class World
{
public:
  void resolveCollision(Volume& volume, int& x, int& y, int& velocity)
  {
    // Determine what the hero is colliding with and
    // modify position and velocity if needed...
  }
};

namespace Motivation
{
  bool collidingWithFloor() { return false; }
  static const int INVISIBLE = 1;
  int getRenderState() { return INVISIBLE; }
  void playSound(int sound) {}
  static const int HIT_FLOOR = 1;
  
  void temp()
  {
    //^gordian
    if (collidingWithFloor() && (getRenderState() != INVISIBLE))
    {
      playSound(HIT_FLOOR);
    }
    //^gordian
  }
}

namespace Monolithic
{
  //^1
  class Bjorn
  {
  public:
    Bjorn()
    : velocity_(0),
      x_(0), y_(0)
    {}

    void update(World& world, Graphics& graphics)
    {
      // Apply user input to hero's velocity.
      switch (Controller::getJoystickDirection())
      {
        case DIR_LEFT:
          velocity_ -= WALK_ACCELERATION;
          break;
          
        case DIR_RIGHT:
          velocity_ += WALK_ACCELERATION;
          break;
          //^omit
        case DIR_NONE: break; // Do nothing.
          //^omit
      }

      // Modify position by velocity.
      x_ += velocity_;
      world.resolveCollision(volume_, x_, y_, velocity_);

      // Draw the appropriate sprite.
      Sprite* sprite = &spriteStand_;
      if (velocity_ < 0)
      {
        sprite = &spriteWalkLeft_;
      }
      else if (velocity_ > 0)
      {
        sprite = &spriteWalkRight_;
      }

      graphics.draw(*sprite, x_, y_);
    }

  private:
    static const int WALK_ACCELERATION = 1;

    int velocity_;
    int x_, y_;

    Volume volume_;

    Sprite spriteStand_;
    Sprite spriteWalkLeft_;
    Sprite spriteWalkRight_;
  };
  //^1
}

namespace SplitInputComponent
{
  class Bjorn
  {
  public:
    int velocity;
    int x, y;
  };

  //^2
  class InputComponent
  {
  public:
    void update(Bjorn& bjorn)
    {
      switch (Controller::getJoystickDirection())
      {
        case DIR_LEFT:
          bjorn.velocity -= WALK_ACCELERATION;
          break;
          
        case DIR_RIGHT:
          bjorn.velocity += WALK_ACCELERATION;
          break;
          //^omit
        case DIR_NONE: break; // Do nothing.
          //^omit
      }
    }

  private:
    static const int WALK_ACCELERATION = 1;
  };
  //^2
}

namespace SplitAIComponent
{
  class Bjorn;

  class InputComponent
  {
  public:
    void update(Bjorn& bjorn);
  };

  //^3
  class Bjorn
  {
  public:
    int velocity;
    int x, y;

    void update(World& world, Graphics& graphics)
    {
      //^4
      input_.update(*this);
      //^4

      // Modify position by velocity.
      x += velocity;
      world.resolveCollision(volume_, x, y, velocity);

      // Draw the appropriate sprite.
      Sprite* sprite = &spriteStand_;
      if (velocity < 0)
      {
        sprite = &spriteWalkLeft_;
      }
      else if (velocity > 0)
      {
        sprite = &spriteWalkRight_;
      }

      graphics.draw(*sprite, x, y);
    }

  private:
    InputComponent input_;

    Volume volume_;

    Sprite spriteStand_;
    Sprite spriteWalkLeft_;
    Sprite spriteWalkRight_;
  };
  //^3
}

namespace Components
{
  class Bjorn
  {
  public:
    int velocity;
    int x, y;
  };

  class InputComponent
  {
  public:
    void update(Bjorn& bjorn) {}
  };

  //^5
  class PhysicsComponent
  {
  public:
    void update(Bjorn& bjorn, World& world)
    {
      bjorn.x += bjorn.velocity;
      world.resolveCollision(volume_,
          bjorn.x, bjorn.y, bjorn.velocity);
    }

  private:
    Volume volume_;
  };
  //^5

  //^6
  class GraphicsComponent
  {
  public:
    void update(Bjorn& bjorn, Graphics& graphics)
    {
      Sprite* sprite = &spriteStand_;
      if (bjorn.velocity < 0)
      {
        sprite = &spriteWalkLeft_;
      }
      else if (bjorn.velocity > 0)
      {
        sprite = &spriteWalkRight_;
      }

      graphics.draw(*sprite, bjorn.x, bjorn.y);
    }

  private:
    Sprite spriteStand_;
    Sprite spriteWalkLeft_;
    Sprite spriteWalkRight_;
  };
  //^6
}

namespace ComponentBjorn
{
  class Bjorn;

  class InputComponent
  {
  public:
    void update(Bjorn& bjorn) {}
  };

  class PhysicsComponent
  {
  public:
    void update(Bjorn& bjorn, World& world) {}
  };

  class GraphicsComponent
  {
  public:
    void update(Bjorn& bjorn, Graphics& graphics) {}
  };

  //^7
  class Bjorn
  {
  public:
    int velocity;
    int x, y;

    void update(World& world, Graphics& graphics)
    {
      input_.update(*this);
      physics_.update(*this, world);
      graphics_.update(*this, graphics);
    }

  private:
    InputComponent input_;
    PhysicsComponent physics_;
    GraphicsComponent graphics_;
  };
  //^7
}

namespace AbstractInput
{
  class Bjorn
  {
  public:
    int velocity;
    int x, y;
  };

  //^8
  class InputComponent
  {
  public:
    virtual ~InputComponent() {}
    virtual void update(Bjorn& bjorn) = 0;
  };
  //^8

  //^9
  class PlayerInputComponent : public InputComponent
  {
  public:
    virtual void update(Bjorn& bjorn)
    {
      switch (Controller::getJoystickDirection())
      {
        case DIR_LEFT:
          bjorn.velocity -= WALK_ACCELERATION;
          break;
          
        case DIR_RIGHT:
          bjorn.velocity += WALK_ACCELERATION;
          break;
          //^omit
        case DIR_NONE: break; // Do nothing.
          //^omit
      }
    }

  private:
    static const int WALK_ACCELERATION = 1;
  };
  //^9
}

namespace AbstractInputBjorn
{
  class Bjorn;

  class InputComponent
  {
  public:
    void update(Bjorn& bjorn)
    {
    }
  };

  class PlayerInputComponent : public InputComponent {};

  class PhysicsComponent
  {
  public:
    void update(Bjorn& bjorn, World& world)
    {
    }
  };

  class GraphicsComponent
  {
  public:
    void update(Bjorn& bjorn, Graphics& graphics)
    {
    }
  };

  //^10
  class Bjorn
  {
  public:
    int velocity;
    int x, y;

    Bjorn(InputComponent* input)
    : input_(input)
    {}

    void update(World& world, Graphics& graphics)
    {
      input_->update(*this);
      physics_.update(*this, world);
      graphics_.update(*this, graphics);
    }

  private:
    InputComponent* input_;
    PhysicsComponent physics_;
    GraphicsComponent graphics_;
  };
  //^10

  //^12
  class DemoInputComponent : public InputComponent
  {
  public:
    virtual void update(Bjorn& bjorn)
    {
      // AI to automatically control Bjorn...
    }
  };
  //^12

  void makeBjorn()
  {
    {
      //^11
      Bjorn* bjorn = new Bjorn(new PlayerInputComponent());
      //^11
      use(bjorn);
    }

    {
      //^13
      Bjorn* bjorn = new Bjorn(new DemoInputComponent());
      //^13
      use(bjorn);
    }
  }
}

namespace BaseGameObject
{
  class GameObject;

  class InputComponent
  {
  public:
    virtual ~InputComponent() {}
    virtual void update(GameObject& obj) = 0;
  };

  class PlayerInputComponent : public InputComponent
  {
  public:
    virtual void update(GameObject& obj)
    {
    }
  };

  //^14
  class PhysicsComponent
  {
  public:
    virtual ~PhysicsComponent() {}
    virtual void update(GameObject& obj, World& world) = 0;
  };

  class GraphicsComponent
  {
  public:
    virtual ~GraphicsComponent() {}
    virtual void update(GameObject& obj, Graphics& graphics) = 0;
  };
  //^14

  //^15
  class GameObject
  {
  public:
    int velocity;
    int x, y;

    GameObject(InputComponent* input,
               PhysicsComponent* physics,
               GraphicsComponent* graphics)
    : input_(input),
      physics_(physics),
      graphics_(graphics)
    {}

    void update(World& world, Graphics& graphics)
    {
      input_->update(*this);
      physics_->update(*this, world);
      graphics_->update(*this, graphics);
    }

  private:
    InputComponent* input_;
    PhysicsComponent* physics_;
    GraphicsComponent* graphics_;
  };
  //^15

  //^16
  class BjornPhysicsComponent : public PhysicsComponent
  {
  public:
    virtual void update(GameObject& obj, World& world)
    {
      // Physics code...
    }
  };

  class BjornGraphicsComponent : public GraphicsComponent
  {
  public:
    virtual void update(GameObject& obj, Graphics& graphics)
    {
      // Graphics code...
    }
  };
  //^16

  //^17
  GameObject* createBjorn()
  {
    return new GameObject(new PlayerInputComponent(),
                          new BjornPhysicsComponent(),
                          new BjornGraphicsComponent());
  }
  //^17
}

namespace DirectComponentRef
{
  class BjornPhysicsComponent
  {
  public:
    bool isOnGround() { return true; }
  };

  class GameObject
  {
  public:
    int velocity;
    int x, y;
  };

  //^18
  class BjornGraphicsComponent
  {
  public:
    BjornGraphicsComponent(BjornPhysicsComponent* physics)
    : physics_(physics)
    {}

    void Update(GameObject& obj, Graphics& graphics)
    {
      Sprite* sprite;
      if (!physics_->isOnGround())
      {
        sprite = &spriteJump_;
      }
      else
      {
        // Existing graphics code...
        //^omit
        sprite = NULL;
        //^omit
      }

      graphics.draw(*sprite, obj.x, obj.y);
    }

  private:
    BjornPhysicsComponent* physics_;

    Sprite spriteStand_;
    Sprite spriteWalkLeft_;
    Sprite spriteWalkRight_;
    Sprite spriteJump_;
  };
  //^18
}

namespace ComponentMessaging
{
  //^19
  class Component
  {
  public:
    virtual ~Component() {}
    virtual void receive(int message) = 0;
  };
  //^19

  //^20
  class ContainerObject
  {
  public:
    void send(int message)
    {
      for (int i = 0; i < MAX_COMPONENTS; i++)
      {
        if (components_[i] != NULL)
        {
          components_[i]->receive(message);
        }
      }
    }

  private:
    static const int MAX_COMPONENTS = 10;
    Component* components_[MAX_COMPONENTS];
  };
}
