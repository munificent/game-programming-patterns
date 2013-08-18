//
//  update-method.h
//  cpp
//
//  Created by Bob Nystrom on 8/6/13.
//  Copyright (c) 2013 Bob Nystrom. All rights reserved.
//

#ifndef cpp_update_method_h
#define cpp_update_method_h

namespace UpdateMethod
{
  namespace Motivation
  {
    struct Entity
    {
      int x;
      void shootLightning() {}
    };

    void refreshGame() {}

    void justPatrol()
    {
      Entity skeleton;

      //^just-patrol
      while (true)
      {
        // Patrol right.
        for (int x = 0; x < 100; x++)
        {
          skeleton.x = x;
        }

        // Patrol left.
        for (int x = 100; x > 0; x--)
        {
          skeleton.x = x;
        }
      }
      //^just-patrol
    }

    void patrolInLoop()
    {
      //^patrol-in-loop
      Entity skeleton;
      bool patrollingLeft = false;
      int x = 0;

      // Main game loop:
      while (true)
      {
        if (patrollingLeft)
        {
          x--;
          if (x == 0) patrollingLeft = false;
        }
        else
        {
          x++;
          if (x == 100) patrollingLeft = true;
        }

        skeleton.x = x;

        // Handle user input and render game...
      }
      //^patrol-in-loop
    }

    void statues()
    {
      //^statues
      // Skeleton variables...
      Entity leftStatue;
      Entity rightStatue;
      int leftStatueDelay = 90;
      int rightStatueDelay = 80;

      // Main game loop:
      while (true)
      {
        // Skeleton code...

        if (leftStatueDelay-- == 0)
        {
          leftStatueDelay = 90;
          leftStatue.shootLightning();
        }

        if (rightStatueDelay-- == 0)
        {
          rightStatueDelay = 80;
          rightStatue.shootLightning();
        }

        // Handle user input and render game...
      }
      //^statues
    }
  }
  
  namespace KeepInMind
  {
    struct Entity
    {
      void setPosition(int x, int y) {}
      void shootLightning() {}
      void update() {}
    };

#define MAX_ENTITIES 10
    
    void refreshGame() {}

    void skipAdded()
    {
      int numObjects = 0;
      Entity* objects[MAX_ENTITIES];
      //^skip-added
      int numObjectsThisTurn = numObjects;
      for (int i = 0; i < numObjectsThisTurn; i++)
      {
        objects[i]->update();
      }
      //^skip-added
    }

    void skipRemoved()
    {
      int numObjects = 0;
      Entity* objects[MAX_ENTITIES];

      //^skip-removed
      for (int i = 0; i < numObjects; i++)
      {
        objects[i]->update();
      }
      //^skip-removed
    }
  }

  namespace SampleCode
  {
    //^entity-class
    class Entity
    {
    public:
      Entity()
      : x(0), y(0)
      {}
      
      virtual void update() = 0;

    protected:
      double x;
      double y;
    };
    //^entity-class

    //^game-world
    class World
    {
    public:
      World()
      : numEntities_(0)
      {}

      void gameLoop();
      
    private:
      Entity* entities_[MAX_ENTITIES];
      int numEntities_;
    };
    //^game-world

    //^game-loop
    void World::gameLoop()
    {
      while (true)
      {
        // Handle user input...
        
        // Update each entity.
        //^update-component-entities
        for (int i = 0; i < numEntities_; i++)
        {
          entities_[i]->update();
        }
        //^update-component-entities

        // Physics and rendering...
      }
    }
    //^game-loop

    //^skeleton
    class Skeleton : public Entity
    {
    public:
      Skeleton()
      : patrollingLeft_(false)
      {}
      
      void update()
      {
        if (patrollingLeft_)
        {
          x--;
          if (x == 0) patrollingLeft_ = false;
        }
        else
        {
          x++;
          if (x == 100) patrollingLeft_ = true;
        }
      }

    private:
      bool patrollingLeft_;
    };
    //^skeleton

    //^statue
    class Statue : public Entity
    {
    public:
      Statue(int delay)
      : frames_(0),
        delay_(delay)
      {}

      void update()
      {
        if (frames_-- == 0)
        {
          shootLightning();
          
          // Reset the timer.
          frames_ = delay_;
        }
      }

    private:
      int frames_;
      int delay_;
      
      void shootLightning()
      {
        // Shoot the lightning...
      }
    };
    //^statue
  }
  
  namespace ForwardToDelegate
  {
    class Entity;

    class Entity
    {
    public:
      Entity* state_;
      void update();
    };

    //^forward
    void Entity::update()
    {
      // Forward to state object.
      state_->update();
    }
    //^forward
  }
}

#endif
