typedef int SoundId;
typedef int ParticleType;

const SoundId SOUND_SPROING = 1;
const SoundId SOUND_SWOOP = 1;
const SoundId SOUND_DIVE = 1;
const ParticleType PARTICLE_DUST = 1;
const ParticleType PARTICLE_SPARKLES = 1;

namespace SimpleExample
{
  //^1
  class Superpower
  {
  public:
    virtual ~Superpower() {}

  protected:
    virtual void activate() = 0;

    void move(double x, double y, double z)
    {
      // Code here...
    }

    void playSound(SoundId sound, double volume)
    {
      // Code here...
    }

    void spawnParticles(ParticleType type, int count)
    {
      // Code here...
    }
  };
  //^1

  //^2
  class SkyLaunch : public Superpower
  {
  protected:
    virtual void activate()
    {
      // Spring into the air.
      playSound(SOUND_SPROING, 1.0f);
      spawnParticles(PARTICLE_DUST, 10);
      move(0, 0, 20);
    }
  };
  //^2
}

namespace Elaborated
{
  //^3
  class Superpower
  {
  protected:
    //^omit
    virtual void activate() = 0;
    void move(double x, double y, double z) {}
    void playSound(SoundId sound, double volume) {}
    void spawnParticles(ParticleType type, int count) {}
    //^omit
    double getHeroX()
    {
      // Code here...
      //^omit
      return 0;
      //^omit
    }

    double getHeroY()
    {
      // Code here...
      //^omit
      return 0;
      //^omit
    }

    double getHeroZ()
    {
      // Code here...
      //^omit
      return 0;
      //^omit
    }

    // Existing stuff...
  };
  //^3

  //^4
  class SkyLaunch : public Superpower
  {
  protected:
    virtual void activate()
    {
      if (getHeroZ() == 0)
      {
        // On the ground, so spring into the air.
        playSound(SOUND_SPROING, 1.0f);
        spawnParticles(PARTICLE_DUST, 10);
        move(0, 0, 20);
      }
      else if (getHeroZ() < 10.0f)
      {
        // Near the ground, so do a double jump.
        playSound(SOUND_SWOOP, 1.0f);
        move(0, 0, getHeroZ() + 20);
      }
      else
      {
        // Way up in the air, so do a dive attack.
        playSound(SOUND_DIVE, 0.7f);
        spawnParticles(PARTICLE_SPARKLES, 1);
        move(0, 0, -getHeroZ());
      }
    }
  };
  //^4
}

namespace Forwarding
{
  struct SoundEngine
  {
    void play(SoundId sound, double volume) {}
  };

  SoundEngine soundEngine_;

  //^5
  void playSound(SoundId sound, double volume)
  {
    soundEngine_.play(sound, volume);
  }
  //^5
}

namespace HelperClassBefore
{
  //^6
  class Superpower
  {
  protected:
    void playSound(SoundId sound, double volume)
    {
      // Code here...
    }

    void stopSound(SoundId sound)
    {
      // Code here...
    }

    void setVolume(SoundId sound)
    {
      // Code here...
    }

    // Sandbox method and other operations...
  };
  //^6
};

namespace HelperClassAfter
{
  //^7
  class SoundPlayer
  {
    void playSound(SoundId sound, double volume)
    {
      // Code here...
    }

    void stopSound(SoundId sound)
    {
      // Code here...
    }

    void setVolume(SoundId sound)
    {
      // Code here...
    }
  };
  //^7

  //^8
  class Superpower
  {
  protected:
    SoundPlayer& getSoundPlayer()
    {
      return soundPlayer_;
    }

    // Sandbox method and other operations...

  private:
    SoundPlayer soundPlayer_;
  };
  //^8
}

namespace PassToConstructor
{
  class ParticleSystem {};

  //^pass-to-ctor-base
  class Superpower
  {
  public:
    Superpower(ParticleSystem* particles)
    : particles_(particles)
    {}

    // Sandbox method and other operations...

  private:
    ParticleSystem* particles_;
  };
  //^pass-to-ctor-base

  //^pass-to-ctor-sub
  class SkyLaunch : public Superpower
  {
  public:
    SkyLaunch(ParticleSystem* particles)
    : Superpower(particles)
    {}
  };
  //^pass-to-ctor-sub
}

namespace TwoStageInit
{
  class ParticleSystem {};

  class Superpower
  {
  public:
    void init(ParticleSystem* particles) {}
  };

  ParticleSystem* particles;

  class SkyLaunch : public Superpower {};

  void foo()
  {
    //^9
    Superpower* power = new SkyLaunch();
    power->init(particles);
    //^9
  }
}

namespace TwoStageInitEncapsulated
{
  class ParticleSystem {};

  class Superpower
  {
  public:
    void init(ParticleSystem* audio) {}
  };

  class SkyLaunch : public Superpower {};

  //^10
  Superpower* createSkyLaunch(ParticleSystem* particles)
  {
    Superpower* power = new SkyLaunch();
    power->init(particles);
    return power;
  }
  //^10
}

namespace StaticState
{
  class ParticleSystem {};

  //^11
  class Superpower
  {
  public:
    static void init(ParticleSystem* particles)
    {
      particles_ = particles;
    }

    // Sandbox method and other operations...

  private:
    static ParticleSystem* particles_;
  };
  //^11
}

namespace UseServiceLocator
{
  struct ParticleSystem
  {
    void spawn(ParticleType type, int count);
  };

  ParticleSystem particles;

  class Locator
  {
  public:
    static ParticleSystem& getParticles() { return particles; }
  };

  //^12
  class Superpower
  {
  protected:
    void spawnParticles(ParticleType type, int count)
    {
      ParticleSystem& particles = Locator::getParticles();
      particles.spawn(type, count);
    }

    // Sandbox method and other operations...
  };
  //^12
}
