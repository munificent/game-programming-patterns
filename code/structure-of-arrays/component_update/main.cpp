#include "utils.h"

// The tests a simulated game loop with a bunch of different memory
// configurations. You have a bunch of actors. Each actor has a few components.
// Each frame every component for every actor is updated. Except for "wrong
// order", all components of the same "type" are update for each actor.
// In other words, it updates component #1 for every actor, then component #2,
// etc.
//
// Each component has a few fields of state. "Updating" a component just does a
// minimal amount of work to read each state.
//
// The tests cover different ways the actors and components can be organized
// in memory to show cache effects. On my machine, output is something like:
//
//     just components     9.4320ms
//         wrong order    15.5460ms    1.65x
//       inline actors    15.7990ms    1.68x
//     order by actors    22.4630ms    2.38x
// order by components    13.0540ms    1.38x
//   random components    89.5480ms    9.49x
//       random actors   149.9980ms   15.90x
//
// This is with 4 components and 4 words of state for each component. Varying
// those numbers affects the timing. With 8 components and 1 word of state for
// each (a pathologically bad setup), I get numbers like:
//
//         wrong order     7.8380ms    1.91x
//       inline actors    17.7670ms    4.33x
//     order by actors    44.5870ms   10.87x
// order by components    37.3180ms    9.10x
//   random components   112.7880ms   27.51x
//       random actors   221.1880ms   53.95x  <-- !!!

static const int NUM_ACTORS = 1000000;
static const int NUM_COMPONENTS = 4;
static const int NUM_FIELDS = 4;

class Component
{
public:
  Component()
  {
    for (int i = 0; i < NUM_FIELDS; i++) data[i] = i;
  }

  int data[NUM_FIELDS];

  int update()
  {
    int sum = 0;
    for (int i = 0; i < NUM_FIELDS; i++) sum += data[i]++;
    return sum;
  }
};

class InlineActor
{
public:
  Component components[NUM_COMPONENTS];
};

class Actor
{
public:
  Component* components[NUM_COMPONENTS];
};

// Iterate through the components in memory directly instead of going through
// actors. This is the best case scenario.
float testComponents()
{
  Component* components = new Component[NUM_ACTORS * NUM_COMPONENTS];

  startProfile();
  long sum = 0;
  for (int i = 0; i < NUM_ACTORS * NUM_COMPONENTS; i++)
  {
    sum += components[i].update();
  }
  float elapsed = endProfile("    just components ");

  use(sum);

  delete [] components;
  return elapsed;
}

// For each actor, update all of its components. Not representative of real
// game since games update all components of one type. But gives us a point of
// comparison for testInline().
void testWrongOrder(float best)
{
  InlineActor* actors = new InlineActor[NUM_ACTORS];

  startProfile();
  long sum = 0;
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    for (int j = 0; j < NUM_COMPONENTS; j++)
    {
      sum += actors[i].components[j].update();
    }
  }
  endProfile("        wrong order ", best);
  use(sum);

  delete [] actors;
}

// For each component, update each actor, with components stored inline in the
// actor.
void testInline(float best)
{
  InlineActor* actors = new InlineActor[NUM_ACTORS];

  startProfile();
  long sum = 0;
  for (int i = 0; i < NUM_COMPONENTS; i++)
  {
    for (int j = 0; j < NUM_ACTORS; j++)
    {
      sum += actors[j].components[i].update();
    }
  }
  endProfile("      inline actors ", best);
  use(sum);

  delete [] actors;
}

// For each component, update each actor. The actor stores pointers to the
// components, and the components are ordered in memory by actor then component.
void testOrderedByActor(float best)
{
  Actor* actors = new Actor[NUM_ACTORS];
  Component* components = new Component[NUM_ACTORS * NUM_COMPONENTS];

  // Wire up the components to the actors.
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    for (int j = 0; j < NUM_COMPONENTS; j++)
    {
      actors[i].components[j] = &components[i * NUM_COMPONENTS + j];
    }
  }

  startProfile();
  long sum = 0;
  for (int i = 0; i < NUM_COMPONENTS; i++)
  {
    for (int j = 0; j < NUM_ACTORS; j++)
    {
      sum += actors[j].components[i]->update();
    }
  }
  endProfile("    order by actors ", best);
  use(sum);

  delete [] actors;
  delete [] components;
}

// For each component, update each actor. The actor stores pointers to the
// components, and the components are ordered in memory by component then actor.
void testOrderedByComponent(float best)
{
  Actor* actors = new Actor[NUM_ACTORS];
  Component* components = new Component[NUM_ACTORS * NUM_COMPONENTS];

  // Wire up the components to the actors.
  for (int i = 0; i < NUM_COMPONENTS; i++)
  {
    for (int j = 0; j < NUM_ACTORS; j++)
    {
      actors[j].components[i] = &components[i * NUM_ACTORS + j];
    }
  }

  startProfile();
  long sum = 0;
  for (int i = 0; i < NUM_COMPONENTS; i++)
  {
    for (int j = 0; j < NUM_ACTORS; j++)
    {
      sum += actors[j].components[i]->update();
    }
  }
  endProfile("order by components ", best);
  use(sum);

  delete [] actors;
  delete [] components;
}

// For each component, update each actor. The actor stores pointers to the
// components, and the components are in random order.
void testRandomComponents(float best)
{
  Actor* actors = new Actor[NUM_ACTORS];
  Component* components = new Component[NUM_ACTORS * NUM_COMPONENTS];

  int* indexes = shuffledArray(NUM_ACTORS * NUM_COMPONENTS);

  // Wire up the components to the actors.
  for (int i = 0; i < NUM_COMPONENTS; i++)
  {
    for (int j = 0; j < NUM_ACTORS; j++)
    {
      actors[j].components[i] = &components[indexes[i * NUM_ACTORS + j]];
    }
  }

  delete [] indexes;

  startProfile();
  long sum = 0;
  for (int i = 0; i < NUM_COMPONENTS; i++)
  {
    for (int j = 0; j < NUM_ACTORS; j++)
    {
      sum += actors[j].components[i]->update();
    }
  }
  endProfile("  random components ", best);
  use(sum);

  delete [] actors;
  delete [] components;
}

// For each component, update each actor. The list of actors is pointers to
// actors that are in random order in memory. Each actor points to components
// that are in random order in memory.
//
// This is a realistic and also worst case.
void testRandomActors(float best)
{
  Actor* actors = new Actor[NUM_ACTORS];
  Component* components = new Component[NUM_ACTORS * NUM_COMPONENTS];

  int* componentIndexes = shuffledArray(NUM_ACTORS * NUM_COMPONENTS);

  // Wire up the components to the actors.
  for (int i = 0; i < NUM_COMPONENTS; i++)
  {
    for (int j = 0; j < NUM_ACTORS; j++)
    {
      actors[j].components[i] = &components[componentIndexes[i * NUM_ACTORS + j]];
    }
  }

  delete [] componentIndexes;

  // Fill the list of actors.
  Actor** actorList = new Actor*[NUM_ACTORS];

  int* actorIndexes = shuffledArray(NUM_ACTORS);

  for (int i = 0; i < NUM_ACTORS; i++)
  {
    actorList[i] = &actors[actorIndexes[i]];
  }

  delete [] actorIndexes;

  startProfile();
  long sum = 0;
  for (int i = 0; i < NUM_COMPONENTS; i++)
  {
    for (int j = 0; j < NUM_ACTORS; j++)
    {
      sum += actorList[j]->components[i]->update();
    }
  }
  endProfile("      random actors ", best);

  use(sum);

  delete [] actors;
  delete [] components;
  delete [] actorList;
}

int main(int argc, const char * argv[])
{
  for (int i = 0; i < 4; i++)
  {
    float components = testComponents();
    testWrongOrder(components);
    testInline(components);
    testOrderedByActor(components);
    testOrderedByComponent(components);
    testRandomComponents(components);
    testRandomActors(components);
  }

  return 0;
}
