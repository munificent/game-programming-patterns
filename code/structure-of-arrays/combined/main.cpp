//
//  main.cpp
//  combined
//
//  Created by Bob Nystrom on 11/17/13.
//  Copyright (c) 2013 Bob Nystrom. All rights reserved.
//

#include <iostream>

#include "utils.h"

static const int NUM_FIELDS = 4;
static const int NUM_ACTORS = 2000;
static const int NUM_COMPONENTS = 4;
static const int NUM_FRAMES = 10000;
static const int NUM_COLD_FIELDS = 16;
static const int EXTRA = 10;

class Component
{
public:
  int data[NUM_FIELDS];

  Component()
  {
    for (int i = 0; i < NUM_FIELDS; i++) data[i] = i;
  }

  int update()
  {
    int sum = 0;
    for (int i = 0; i < NUM_FIELDS; i++) sum += data[i];
    return sum;
  }
};

class ActorBefore
{
public:
  bool isActive = true;

  Component* components[NUM_COMPONENTS];
  int cold[NUM_COLD_FIELDS];
};

void testSlow(float best)
{
  ActorBefore** actors = new ActorBefore*[NUM_ACTORS * EXTRA];

  // Create a bunch of actors with random amounts of space between them.
  for (int i = 0; i < NUM_ACTORS * EXTRA; i++)
  {
    actors[i] = new ActorBefore();
  }

  // Shuffle them in the list.
  shuffle(actors, NUM_ACTORS * EXTRA);

  // Deactivate half of them.
  for (int i = 0; i < NUM_ACTORS * EXTRA / 2; i++)
  {
    actors[i]->isActive = false;
  }

  // Shuffle them in the list.
  shuffle(actors, NUM_ACTORS * EXTRA);

  // Create a bunch of components.
  Component** components = new Component*[NUM_ACTORS * NUM_COMPONENTS * EXTRA];
  for (int i = 0; i < NUM_ACTORS * NUM_COMPONENTS * EXTRA; i++)
  {
    components[i] = new Component();
  }

  // Shuffle them in the list.
  shuffle(components, NUM_ACTORS * NUM_COMPONENTS * EXTRA);

  // Wire them up to actors.
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    for (int j = 0; j < NUM_COMPONENTS; j++)
    {
      actors[i]->components[j] = components[i * NUM_COMPONENTS + j];
    }
  }

  // Run the simulation.
  startProfile();
  int sum = 0;
  for (int f = 0; f < NUM_FRAMES; f++)
  {
    for (int c = 0; c < NUM_COMPONENTS; c++)
    {
      for (int a = 0; a < NUM_ACTORS; a++)
      {
        if (!actors[a]->isActive) continue;
        sum += actors[a]->components[c]->update();
      }
    }
  }
  endProfile("worst", best);
  if (sum != 240000000) printf("%d\n", sum);

  // Free everything.
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    delete actors[i];
  }

  for (int i = 0; i < NUM_ACTORS * NUM_COMPONENTS; i++)
  {
    delete components[i];
  }

  delete [] actors;
  delete [] components;
}

float testFast()
{
  Component* components[NUM_COMPONENTS];

  // Create the component arrays.
  for (int i = 0; i < NUM_COMPONENTS; i++)
  {
    components[i] = new Component[NUM_ACTORS];
  }

  // Run the simulation.
  startProfile();
  int sum = 0;
  for (int f = 0; f < NUM_FRAMES; f++)
  {
    for (int c = 0; c < NUM_COMPONENTS; c++)
    {
      Component* componentArray = components[c];
      for (int a = 0; a < NUM_ACTORS / 2; a++)
      {
        sum += componentArray[a].update();
      }
    }
  }
  float result = endProfile("best");
  if (sum != 240000000) printf("%d\n", sum);

  // Free everything.
  for (int i = 0; i < NUM_COMPONENTS; i++)
  {
    delete [] components[i];
  }

  return result;
}

int main(int argc, const char * argv[])
{
  for (int i = 0; i < 3; i++)
  {
    float best = testFast();
    testSlow(best);
  }
  return 0;
}

