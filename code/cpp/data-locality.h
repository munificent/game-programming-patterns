#include <time.h>

#ifndef cpp_data_locality_h
#define cpp_data_locality_h

// TODO(bob):
//
// cache effects are magnified by:
// - turning up number of actors
// - adding padding to the actor class
//   (both because it spaces the actors out more in memory)
//   in examples below, creating much bigger shuffled array just to spread
//   them out more.
// - adding padding to component magnifies it too, but also punishes best case

namespace DataLocality
{
  void doAbsolutelyNothingFor500Cycles() {}

  struct Thing
  {
    void doStuff() {}
  };

  static const int NUM_THINGS = 3;

  void callDoNothing()
  {
    Thing things[NUM_THINGS];

    //^do-nothing
    for (int i = 0; i < NUM_THINGS; i++)
    {
      doAbsolutelyNothingFor500Cycles();
      things[i].doStuff();
    }
    //^do-nothing
  }

  //^components
  class AIComponent
  {
  public:
    void update() { /* Work with and modify state... */ }

  private:
    // Goals, mood, etc. ...
  };

  class PhysicsComponent
  {
  public:
    void update() { /* Work with and modify state... */ }

  private:
    // Rigid body, velocity, mass, etc. ...
  };

  class RenderComponent
  {
  public:
    void render() { /* Work with and modify state... */ }

  private:
    // Mesh, textures, shaders, etc. ...
  };
  //^components

  //^game-entity
  class GameEntity
  {
  public:
    GameEntity(AIComponent* ai,
               PhysicsComponent* physics,
               RenderComponent* render)
    : ai_(ai), physics_(physics), render_(render)
    {}

    AIComponent*      ai()      { return ai_; }
    PhysicsComponent* physics() { return physics_; }
    RenderComponent*  render()  { return render_; }

  private:
    AIComponent*      ai_;
    PhysicsComponent* physics_;
    RenderComponent*  render_;
  };
  //^game-entity

  void gameLoop()
  {
    int numEntities = 123;
    GameEntity* entities[123];
    bool gameOver = false;

    //^game-loop
    while (!gameOver)
    {
      // Process AI.
      for (int i = 0; i < numEntities; i++)
      {
        entities[i]->ai()->update();
      }

      // Update physics.
      for (int i = 0; i < numEntities; i++)
      {
        entities[i]->physics()->update();
      }

      // Draw to screen.
      for (int i = 0; i < numEntities; i++)
      {
        entities[i]->render()->render();
      }

      // Other game loop machinery for timing...
    }
    //^game-loop
  }

  static const int MAX_ENTITIES = 100;

  void componentArrays()
  {
    //^component-arrays
    AIComponent* aiComponents =
        new AIComponent[MAX_ENTITIES];
    PhysicsComponent* physicsComponents =
        new PhysicsComponent[MAX_ENTITIES];
    RenderComponent* renderComponents =
        new RenderComponent[MAX_ENTITIES];
    //^component-arrays

    int numEntities = 123;
    bool gameOver = false;

    //^game-loop-arrays
    while (!gameOver)
    {
      // Process AI.
      for (int i = 0; i < numEntities; i++)
      {
        aiComponents[i].update();
      }

      // Update physics.
      for (int i = 0; i < numEntities; i++)
      {
        physicsComponents[i].update();
      }

      // Draw to screen.
      for (int i = 0; i < numEntities; i++)
      {
        renderComponents[i].render();
      }

      // Other game loop machinery for timing...
    }
    //^game-loop-arrays

    delete [] aiComponents;
    delete [] physicsComponents;
    delete [] renderComponents;
  }


  //^particle-system
  class Particle
  {
  public:
    //^omit particle-system
    bool isActive() { return false; }
    //^omit particle-system
    void update() { /* Gravity, etc. ... */ }
  };

  class ParticleSystem
  {
  public:
    void update();
    //^omit particle-system
    void activateParticle(int index);
    void deactivateParticle(int index);
    //^omit particle-system
  private:
    static const int MAX_PARTICLES = 100000;

    int numParticles_ = 0;
    Particle particles_[MAX_PARTICLES];
  };
  //^particle-system

  //^update-particle-system
  void ParticleSystem::update()
  {
    for (int i = 0; i < numParticles_; i++)
    {
      particles_[i].update();
    }
  }
  //^update-particle-system

  void updateParticlesSlow()
  {
    Particle particles_[100];
    int numParticles_ = 0;
    //^particles-is-active
    for (int i = 0; i < numParticles_; i++)
    {
      if (particles_[i].isActive())
      {
        particles_[i].update();
      }
    }
    //^particles-is-active
  }

  Particle particles[100];
  int numActiveParticles_ = 0;
  void updateParticles()
  {
    //^update-particles
    for (int i = 0; i < numActiveParticles_; i++)
    {
      particles[i].update();
    }
    //^update-particles
  }

  //^activate-particle
  void ParticleSystem::activateParticle(int index)
  {
    // Shouldn't already be active!
    assert(index >= numActiveParticles_);

    // Swap it with the first inactive particle
    // right after the active ones.
    Particle temp = particles_[numActiveParticles_];
    particles_[numActiveParticles_] = particles_[index];
    particles_[index] = temp;

    // Now there's one more.
    numActiveParticles_++;
  }
  //^activate-particle

  //^deactivate-particle
  void ParticleSystem::deactivateParticle(int index)
  {
    // Shouldn't already be inactive!
    assert(index < numActiveParticles_);

    // There's one fewer.
    numActiveParticles_--;

    // Swap it with the last active particle
    // right before the inactive ones.
    Particle temp = particles_[numActiveParticles_];
    particles_[numActiveParticles_] = particles_[index];
    particles_[index] = temp;
  }
  //^deactivate-particle

  /*

  clock_t startTime;

  void startProfile()
  {
    startTime = clock();
  }

  float endProfile(const char* message)
  {
    float elapsed = (float)(clock() - startTime) * 1000.0f / CLOCKS_PER_SEC;
    printf("%s%10.4fms\n", message, elapsed);
    return elapsed;
  }

  float endProfile(const char* message, float comparison)
  {
    float elapsed = (float)(clock() - startTime) * 1000.0f / CLOCKS_PER_SEC;
    printf("%s%10.4fms  %6.2fx\n", message, elapsed, elapsed / comparison);
    return elapsed;
  }

  //^component
  static const int NUM_FIELDS = 4;

  class Component
  {
  public:
    Component()
    {
      for (int i = 0; i < NUM_FIELDS; i++) data[i] = i;
    }

    double data[NUM_FIELDS];

    int update()
    {
      int sum = 0;
      for (int i = 0; i < NUM_FIELDS; i++) sum += data[i]++;
      return sum;
    }
  };
  //^component

  //^actor
  static const int NUM_COMPONENTS = 4;

  class Actor
  {
  public:
    Component* components[NUM_COMPONENTS];
  };
  //^actor

  static const int NUM_ACTORS = 10000;

  void temp()
  {
    //^actor-list
    static const int NUM_ACTORS = 10000;
    Actor* actors[NUM_ACTORS];
    // TODO(bob): Heap alloc.
    //^actor-list
    use(actors);
  }

  //^update-world
  int updateWorld(Actor* actors[])
  {
    int sum = 0;
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
      for (int j = 0; j < NUM_ACTORS; j++)
      {
        sum += actors[j]->components[i]->update();
      }
    }
    return sum;
  }
  //^update-world

  void runGame(Actor* actors[])
  {
    //^game-loop
    static const int NUM_FRAMES = 10000;
    int sum = 0;
    for (int i = 0; i < NUM_FRAMES; i++)
    {
      sum += updateWorld(actors);
    }

    if (sum == 0) printf("%d\n", sum);
    //^game-loop
  }

  //^rand-range
  int randRange(int min, int max)
  {
    return (int)((double)rand() * (max - min) / RAND_MAX + min);
  }
  //^rand-range

  //^shuffle-array
  int* shuffledArray(int length)
  {
    // Fill array with consecutive indexes.
    int* array = new int[length];
    for (int i = 0; i < length; i++) array[i] = i;

    // Shuffle them.
    for (int i = 0; i < length; i++)
    {
      int j = randRange(i, length);
      int t = array[i];
      array[i] = array[j];
      array[j] = t;
    }
    
    return array;
  }
  //^shuffle-array

  void testShuffled(float best)
  {
    Actor** actors = new Actor*[NUM_ACTORS];

    static const int PADDING = 2;

    //^arrays
    Actor* actorArray = new Actor[NUM_ACTORS * PADDING];
    Component* componentArray = new Component[NUM_ACTORS * NUM_COMPONENTS * PADDING];
    //^arrays
    
    // Fill the actor list with pointers to random actors.
    int* actorIndexes = shuffledArray(NUM_ACTORS * PADDING);
    int* componentIndexes = shuffledArray(NUM_ACTORS * NUM_COMPONENTS * PADDING);
    int component = 0;
    for (int i = 0; i < NUM_ACTORS; i++)
    {
      actors[i] = &actorArray[actorIndexes[i]];

      for (int j = 0; j < NUM_COMPONENTS; j++)
      {
        actors[i]->components[j] = &componentArray[componentIndexes[component]];
        component++;
      }
    }
    delete [] actorIndexes;
    delete [] componentIndexes;

    startProfile();
    runGame(actors);
    endProfile("shuffled", best);

    delete [] actors;
    delete [] actorArray;
    delete [] componentArray;
  }

  void testOrdered(float best)
  {
    Actor** actors = new Actor*[NUM_ACTORS];
    Actor* actorArray = new Actor[NUM_ACTORS];
    Component* componentArray = new Component[NUM_ACTORS * NUM_COMPONENTS];

    int component = 0;
    for (int i = 0; i < NUM_ACTORS; i++)
    {
      actors[i] = &actorArray[i];

      for (int j = 0; j < NUM_COMPONENTS; j++)
      {
        actors[i]->components[j] = &componentArray[component];
        component++;
      }
    }

    startProfile();
    runGame(actors);
    endProfile("ordered", best);

    delete [] actors;
    delete [] actorArray;
    delete [] componentArray;
  }

  float testComponents()
  {
    Component* componentArray = new Component[NUM_ACTORS * NUM_COMPONENTS];

    startProfile();
    static const int NUM_FRAMES = 10000;
    int sum = 0;
    for (int f = 0; f < NUM_FRAMES; f++)
    {
      for (int i = 0; i < NUM_ACTORS * NUM_COMPONENTS; i++)
      {
        sum += componentArray[i].update();
      }
    }

    if (sum == 0) printf("%d\n", sum);
    float best = endProfile("components");
    delete [] componentArray;
    return best;
  }

  void test()
  {
    float best = testComponents();
    testShuffled(best);
    testOrdered(best);
    testComponents();
    testShuffled(best);
    testOrdered(best);
  }
   */
}

#endif
