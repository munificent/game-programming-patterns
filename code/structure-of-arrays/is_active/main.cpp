#include "utils.h"

static const int NUM_ACTORS = 10000000;
static const int NUM_FIELDS = 4;

// When creating the contiguous array of actors, we'll multiply the number of
// actors by this to add some extra padding and spread them out in memory more.
static const int DEAD_ACTORS = 5;

class Actor
{
public:
  Actor()
  {
    for (int i = 0; i < NUM_FIELDS; i++) data[i] = i;
  }

  bool isActive = true;

  int data[NUM_FIELDS];

  int update()
  {
    int sum = 0;
    for (int i = 0; i < NUM_FIELDS; i++) sum += data[i];
    return sum;
  }
};

// A flat array of contiguous actors with the active ones first in the list.
// We know how many are active, so don't have to check at all.
float testInOrderInlineKnown()
{
  Actor* actors = new Actor[NUM_ACTORS];

  // De-activate half of them.
  for (int i = NUM_ACTORS / 2; i < NUM_ACTORS; i++)
  {
    actors[i].isActive = false;
  }

  long sum = 0;
  startProfile();
  for (int i = 0; i < NUM_ACTORS / 2; i++)
  {
    sum += actors[i].update();
  }
  float elapsed = endProfile("       best case");
  use(sum);

  delete [] actors;
  return elapsed;
}

// A flat array of contiguous actors with the active ones first in the list.
// We stop as soon as we hit an inactive one since we know they're sorted.
void testInOrderBailInline(float best)
{
  Actor* actors = new Actor[NUM_ACTORS];

  // De-activate half of them.
  for (int i = NUM_ACTORS / 2; i < NUM_ACTORS; i++)
  {
    actors[i].isActive = false;
  }

  long sum = 0;
  startProfile();
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    if (!actors[i].isActive) break;
    sum += actors[i].update();
  }
  endProfile("   in order bail", best);
  use(sum);

  delete [] actors;
}

// A flat array of contiguous actors with the active ones first in the list.
void testInOrderInline(float best)
{
  Actor* actors = new Actor[NUM_ACTORS];

  // De-activate half of them.
  for (int i = 0; i < NUM_ACTORS / 2; i++)
  {
    actors[i].isActive = false;
  }

  long sum = 0;
  startProfile();
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    if (actors[i].isActive)
    {
      sum += actors[i].update();
    }
  }
  endProfile(" in order inline", best);
  use(sum);

  delete [] actors;
}

// A flat array of contiguous actors with the active and inactive ones randomly
// distributed.
void testMixedInline(float best)
{
  Actor* actors = new Actor[NUM_ACTORS];

  // De-activate half of them.
  int* indexes = shuffledArray(NUM_ACTORS);
  for (int i = 0; i < NUM_ACTORS / 2; i++)
  {
    actors[indexes[i]].isActive = false;
  }
  delete [] indexes;

  long sum = 0;
  startProfile();
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    if (actors[i].isActive)
    {
      sum += actors[i].update();
    }
  }
  endProfile("    mixed inline", best);
  use(sum);

  delete [] actors;
}

// A flat array of contiguous actors with the active and inactive ones randomly
// distributed, then sorted during the profile. We stop as soon as we hit an
// inactive one.
void testSortedInline(float best)
{
  Actor* actors = new Actor[NUM_ACTORS];

  // De-activate half of them.
  int* indexes = shuffledArray(NUM_ACTORS);
  for (int i = 0; i < NUM_ACTORS / 2; i++)
  {
    actors[indexes[i]].isActive = false;
  }
  delete [] indexes;

  long sum = 0;
  startProfile();

  // Sort the active ones to the front.
  int search = 1;
  int numActive = 0;
  for (; numActive < NUM_ACTORS; numActive++)
  {
    if (actors[numActive].isActive) continue;

    // Find an active actor to swap with this one.
    for (; search < NUM_ACTORS; search++)
    {
      if (actors[search].isActive)
      {
        Actor temp = actors[numActive];
        actors[numActive] = actors[search];
        actors[search] = temp;
        search++;
        break;
      }
    }

    // If we searched past the end, we've moved all of the active ones
    // forward.
    if (search >= NUM_ACTORS) break;
  }

  for (int i = 0; i < numActive; i++)
  {
    sum += actors[i].update();
  }
  endProfile("   sorted inline", best);
  use(sum);

  delete [] actors;
}

// An array of pointers to randomly ordered actors, with the active first in
// the list. Bail out as soon as we get to an inactive one.
void testInOrderPointer(float best)
{
  Actor* actors = new Actor[NUM_ACTORS];

  Actor** actorList = new Actor*[NUM_ACTORS];
  int* indexes = shuffledArray(NUM_ACTORS);
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    actorList[i] = &actors[indexes[i]];
    actorList[i]->isActive = i < NUM_ACTORS / 2;
  }
  indexes = shuffledArray(NUM_ACTORS);
  delete [] indexes;

  long sum = 0;
  startProfile();
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    if (!actorList[i]->isActive) break;
    sum += actorList[i]->update();
  }
  endProfile("in order pointer", best);
  use(sum);

  delete [] actors;
}

// An array of pointers to randomly ordered actors, with the active and
// inactive ones randomly distributed.
void testMixedPointer(float best)
{
  Actor* actors = new Actor[NUM_ACTORS * DEAD_ACTORS];

  // De-activate half of them.
  int* indexes = shuffledArray(NUM_ACTORS * DEAD_ACTORS);
  for (int i = 0; i < NUM_ACTORS * DEAD_ACTORS / 2; i++)
  {
    actors[indexes[i]].isActive = false;
  }
  delete [] indexes;

  Actor** actorList = new Actor*[NUM_ACTORS];
  indexes = shuffledArray(NUM_ACTORS * DEAD_ACTORS);
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    actorList[i] = &actors[indexes[i]];
  }
  delete [] indexes;

  long sum = 0;
  startProfile();
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    if (!actorList[i]->isActive) continue;
    sum += actorList[i]->update();
  }
  endProfile("   mixed pointer", best);
  use(sum);

  delete [] actors;
}

// An array of pointers to randomly ordered actors, with the active and
// inactive ones randomly distributed. They are sorted during the profile.
// Since we determine the number of active actors during the sort, we skip the
// active check.
void testSortedPointer(float best)
{
  Actor* actors = new Actor[NUM_ACTORS];

  // De-activate half of them.
  int* indexes = shuffledArray(NUM_ACTORS);
  for (int i = 0; i < NUM_ACTORS / 2; i++)
  {
    actors[indexes[i]].isActive = false;
  }
  delete [] indexes;

  Actor** actorList = new Actor*[NUM_ACTORS];
  for (int i = 0; i < NUM_ACTORS; i++)
  {
    actorList[i] = &actors[indexes[i]];
  }
  indexes = shuffledArray(NUM_ACTORS);
  delete [] indexes;

  long sum = 0;
  startProfile();

  // Sort the active ones to the front.
  int search = 1;
  int numActive = 0;
  for (; numActive < NUM_ACTORS; numActive++)
  {
    if (actorList[numActive]->isActive) continue;

    // Find an active actor to swap with this one.
    for (; search < NUM_ACTORS; search++)
    {
      if (actorList[search]->isActive)
      {
        Actor* temp = actorList[numActive];
        actorList[numActive] = actorList[search];
        actorList[search] = temp;
        search++;
        break;
      }
    }

    // If we searched past the end, we've moved all of the active ones
    // forward.
    if (search >= NUM_ACTORS) break;
  }
  
  for (int i = 0; i < numActive; i++)
  {
    sum += actorList[i]->update();
  }
  endProfile("  sorted pointer", best);
  use(sum);

  delete [] actors;
}

// TODO(bob): Do similar test but with virtual method calls?

int main(int argc, const char * argv[])
{
  srand((unsigned int)time(0));

  for (int i = 0; i < 4; i++)
  {
    float elapsed = testInOrderInlineKnown();
    testInOrderBailInline(elapsed);
    testInOrderInline(elapsed);
    testMixedInline(elapsed);
    testSortedInline(elapsed);
    testInOrderPointer(elapsed);
    testMixedPointer(elapsed);
    testSortedPointer(elapsed);
  }

  return 0;
}

