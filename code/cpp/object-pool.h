#include <iostream>

namespace Version1
{
  class ParticlePool;

  //^1
  class Particle
  {
  public:
    Particle()
    : framesLeft_(0)
    {}

    void init(double x, double y,
              double xVel, double yVel, int lifetime)
    {
      x_ = x; y_ = y;
      xVel_ = xVel; yVel_ = yVel;
      framesLeft_ = lifetime;
    }

    void animate()
    {
      if (!inUse()) return;

      framesLeft_--;
      x_ += xVel_;
      y_ += yVel_;
    }

    bool inUse() const { return framesLeft_ > 0; }

  private:
    int framesLeft_;
    double x_, y_;
    double xVel_, yVel_;
  };
  //^1

  //^2
  class ParticlePool
  {
  public:
    void create(double x, double y,
                double xVel, double yVel, int lifetime);

    void animate()
    {
      for (int i = 0; i < POOL_SIZE; i++)
      {
        particles_[i].animate();
      }
    }

  private:
    static const int POOL_SIZE = 100;
    Particle particles_[POOL_SIZE];
  };
  //^2

  //^3
  void ParticlePool::create(double x, double y,
                            double xVel, double yVel, int lifetime)
  {
    // Find an available particle.
    for (int i = 0; i < POOL_SIZE; i++)
    {
      if (!particles_[i].inUse())
      {
        particles_[i].init(x, y, xVel, yVel, lifetime);
        //^omit

        std::cout << "created " << i << std::endl;
        //^omit
        return;
      }
    }
  }
  //^3
};

namespace Temp1
{
  //^4
  class Particle
  {
  public:
    //^omit
    void init(double x, double y,
              double xVel, double yVel, int lifetime) {}
    bool animate();
    bool inUse() { return false; }
    double x_;
    double y_;
    double xVel_;
    double yVel_;
    //^omit
    // ...

    Particle* getNext() const { return state_.next; }
    void setNext(Particle* next) { state_.next = next; }

  private:
    int framesLeft_;

    union
    {
      // State when it's in use.
      struct
      {
        double x, y;
        double xVel, yVel;
      } live;

      // State when it's available.
      Particle* next;
    } state_;
  };
  //^4

  //^particle-animate
  bool Particle::animate()
  {
    if (!inUse()) return false;

    framesLeft_--;
    x_ += xVel_;
    y_ += yVel_;

    return framesLeft_ == 0;
  }
  //^particle-animate

  //^5
  class ParticlePool
  {
    //^omit
    ParticlePool();
    void create(double x, double y,
                double xVel, double yVel, int lifetime);
    void animate();
    //^omit
    // ...
  private:
    //^omit
    static const int POOL_SIZE = 100;
    Particle particles_[POOL_SIZE];
    //^omit
    Particle* firstAvailable_;
  };
  //^5

  //^6
  ParticlePool::ParticlePool()
  {
    // The first one is available.
    firstAvailable_ = &particles_[0];

    // Each particle points to the next.
    for (int i = 0; i < POOL_SIZE - 1; i++)
    {
      particles_[i].setNext(&particles_[i + 1]);
    }

    // The last one terminates the list.
    particles_[POOL_SIZE - 1].setNext(NULL);
  }
  //^6

  //^7
  void ParticlePool::create(double x, double y,
                            double xVel, double yVel, int lifetime)
  {
    // Make sure the pool isn't full.
    assert(firstAvailable_ != NULL);

    // Remove it from the available list.
    Particle* newParticle = firstAvailable_;
    firstAvailable_ = newParticle->getNext();

    newParticle->init(x, y, xVel, yVel, lifetime);
  }
  //^7

  //^8
  void ParticlePool::animate()
  {
    for (int i = 0; i < POOL_SIZE; i++)
    {
      if (particles_[i].animate())
      {
        // Add this particle to the front of the list.
        particles_[i].setNext(firstAvailable_);
        firstAvailable_ = &particles_[i];
      }
    }
  }
  //^8
};

namespace Temp2
{
  class ParticlePool;

  //^10
  class Particle
  {
    friend class ParticlePool;

  private:
    Particle()
    : inUse_(false)
    {}

    bool inUse_;
  };

  class ParticlePool
  {
    Particle pool_[100];
  };
  //^10
}

namespace Temp3
{
  //^11
  template <class TObject>
  class GenericPool
  {
  private:
    static const int POOL_SIZE = 100;

    TObject pool_[POOL_SIZE];
    bool    inUse_[POOL_SIZE];
  };
  //^11
}

namespace Temp4
{
  //^12
  class Particle
  {
    // Multiple ways to initialize.
    void init(double x, double y);
    void init(double x, double y, double angle);
    void init(double x, double y, double xVel, double yVel);
  };

  class ParticlePool
  {
  public:
    void create(double x, double y)
    {
      // Forward to Particle...
    }

    void create(double x, double y, double angle)
    {
      // Forward to Particle...
    }

    void create(double x, double y, double xVel, double yVel)
    {
      // Forward to Particle...
    }
  };
  //^12
}

namespace Temp5
{
  //^13
  class Particle
  {
  public:
    // Multiple ways to initialize.
    void init(double x, double y);
    void init(double x, double y, double angle);
    void init(double x, double y, double xVel, double yVel);
  };

  class ParticlePool
  {
  public:
    Particle* create()
    {
      // Return reference to available particle...
      //^omit
      return &pool_[0];
      //^omit
    }
  private:
    Particle pool_[100];
  };
  //^13

  class Test
  {
    static void run()
    {
      //^14
      ParticlePool pool;

      pool.create()->init(1, 2);
      pool.create()->init(1, 2, 0.3);
      pool.create()->init(1, 2, 3.3, 4.4);
      //^14

      //^15
      Particle* particle = pool.create();
      if (particle != NULL) particle->init(1, 2);
      //^15
    }
  };
};

// 64 characters --------------------------------------------------------|
void TestParticlePool()
{
  std::cout << "Object pool" << std::endl;

  Version1::ParticlePool pool = Version1::ParticlePool();

  pool.create(0, 0, 1, 1, 10);
  pool.create(1, 0, 1, 1, 10);
  pool.create(2, 0, 1, 1, 10);
   // pool.update();
}