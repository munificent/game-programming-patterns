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
      void setPosition(int x, int y) {}
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
          skeleton.setPosition(x, 20);
        }

        // Patrol left.
        for (int x = 100; x < 0; x--)
        {
          skeleton.setPosition(x, 20);
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

        skeleton.setPosition(x, 20);

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
}

#endif
