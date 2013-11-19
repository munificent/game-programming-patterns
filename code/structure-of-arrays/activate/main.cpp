#include "utils.h"

static const int NUM_FRAMES = 1000;
static const int NUM_ACTORS = 10000;
static const int NUM_FIELDS = 4;
static const int TOGGLE_RATE = 10;

class Actor
{
public:
  Actor()
  {
    for (int i = 0; i < NUM_FIELDS; i++) data[i] = i;
  }

  bool isActive = false;

  int data[NUM_FIELDS];

  int update()
  {
    int sum = 0;
    for (int i = 0; i < NUM_FIELDS; i++) sum += data[i];
    return sum;
  }
};

class SmallActor
{
public:
  SmallActor()
  {
    for (int i = 0; i < NUM_FIELDS; i++) data[i] = i;
  }

  int data[NUM_FIELDS];

  int update()
  {
    int sum = 0;
    for (int i = 0; i < NUM_FIELDS; i++) sum += data[i];
    return sum;
  }
};

// A flat array of contiguous actors with randomly chosen ones active.
void testMixed()
{
  Actor* actors = new Actor[NUM_ACTORS];

  // Enable half of them randomly.
  int* indexes = shuffledArray(NUM_ACTORS);
  for (int i = 0; i < NUM_ACTORS / 2; i++)
  {
    actors[indexes[i]].isActive = true;
  }
  delete [] indexes;

  long sum = 0;
  startProfile();
  for (int i = 0; i < NUM_FRAMES; i++)
  {
    for (int j = 0; j < NUM_ACTORS; j++)
    {
      if (actors[j].isActive)
      {
        sum += actors[j].update();
      }

      // Randomly change active state.
      if (randRange(1, TOGGLE_RATE) == 0) actors[j].isActive = !actors[j].isActive;
    }
  }
  endProfile(" mixed");
  use(sum);

  delete [] actors;
}

// A flat array of contiguous actors with the active ones first.
void testSorted()
{
  SmallActor* actors = new SmallActor[NUM_ACTORS];

  // Enable half of them.
  int numActive = NUM_ACTORS / 2;

  long sum = 0;
  startProfile();
  for (int i = 0; i < NUM_FRAMES; i++)
  {
    for (int j = 0; j < numActive; j++)
    {
      sum += actors[j].update();
    }

    for (int j = 0; j < NUM_ACTORS; j++)
    {
      // Randomly change active state.
      if (randRange(1, TOGGLE_RATE) == 0)
      {
        if (j < numActive)
        {
          // Inactive, so move it after all of the active ones.
          numActive--;
        }
        else
        {
          // Active, so move it to the end of the active section.
          numActive++;
        }
        SmallActor temp = actors[j];
        actors[j] = actors[numActive];
        actors[numActive] = temp;
      }
    }
  }
  endProfile("sorted");
  use(sum);

  delete [] actors;
}

int main(int argc, const char * argv[])
{
  srand((unsigned int)time(0));

  for (int i = 0; i < 4; i++)
  {
    testMixed();
    testSorted();
  }

  return 0;
}

