#include <iostream>

class AudioSystem
{
public:
  static void playSound(int id) {}
  static AudioSystem* instance() { return NULL; }
};

void example()
{
  int VERY_LOUD_BANG = 0;

  //^15
  // Use a static class?
  AudioSystem::playSound(VERY_LOUD_BANG);

  // Or maybe a singleton?
  AudioSystem::instance()->playSound(VERY_LOUD_BANG);
  //^15
}

//^9
class IAudio
{
public:
  virtual void playSound(int soundID) = 0;
  virtual void stopSound(int soundID) = 0;
  virtual void stopAllSounds() = 0;
};
//^9

//^10
class ConsoleAudio : public IAudio
{
public:
  virtual void playSound(int soundID)
  {
    // Play sound using console audio api...
  }

  virtual void stopSound(int soundID)
  {
    // Stop sound using console audio api...
  }

  virtual void stopAllSounds()
  {
    // Stop all sounds using console audio api...
  }
};
//^10

//^12
class LoggedAudio : public IAudio
{
public:
  LoggedAudio(IAudio &wrapped) : wrapped_(wrapped) {}

  virtual void playSound(int soundID)
  {
    log("play sound");
    wrapped_.playSound(soundID);
  }

  virtual void stopSound(int soundID)
  {
    log("stop sound");
    wrapped_.stopSound(soundID);
  }

  virtual void stopAllSounds()
  {
    log("stop all sounds");
    wrapped_.stopAllSounds();
  }


private:
  void log(const char* message) { /* Code to log message... */ }

  IAudio &wrapped_;
};
//^12

// design decisions / di
namespace Version1
{
  //^1
  class Locator
  {
  public:
    static IAudio* getAudio() { return service_; }

    static void provide(IAudio* service)
    {
      service_ = service;
    }

  private:
    static IAudio* service_;
  };
  //^1

  IAudio *Locator::service_;

  //^11
  void initGame()
  {
    ConsoleAudio *audio = new ConsoleAudio();
    Locator::provide(audio);
  }
  //^11

  //^5
  void someGameCode()
  {
    //^omit
    int VERY_LOUD_BANG = 0;
    //^omit
    IAudio *audio = Locator::getAudio();
    audio->playSound(VERY_LOUD_BANG);
  }
  //^5
}

// design decisions / compile time
namespace Version2
{
  class DebugAudio: public IAudio
  {
  public:
    virtual void playSound(int soundID) { /* Do nothing. */ }
    virtual void stopSound(int soundID) { /* Do nothing. */ }
    virtual void stopAllSounds()        { /* Do nothing. */ }
  };
  class ReleaseAudio: public DebugAudio {};

  //^2
  class Locator
  {
  public:
    static IAudio& getAudio() { return service_; }

  private:
    #if DEBUG
      static DebugAudio service_;
    #else
      static ReleaseAudio service_;
    #endif
  };
  //^2
}

// design decisions / scope
namespace Version3
{
  //^3
  class Base
  {
    // Code to locate service and set service_...

  protected:
    // Derived classes can use service
    static IAudio& getAudio() { return *service_; }

  private:
    static IAudio* service_;
  };
  //^3
}

namespace Version4
{
  //^4
  class Locator
  {
  public:
    static IAudio& getAudio()
    {
      IAudio* service = NULL;

      // Code here to locate service...

      assert(service != NULL);
      return *service;
    }
  };
  //^4
}

namespace Version5
{
  //^7
  class NullAudio: public IAudio
  {
  public:
    virtual void playSound(int soundID) { /* Do nothing. */ }
    virtual void stopSound(int soundID) { /* Do nothing. */ }
    virtual void stopAllSounds()        { /* Do nothing. */ }
  };
  //^7

  //^8
  class Locator
  {
  public:
    static void initialize() { service_ = &nullService_; }

    static IAudio& getAudio() { return *service_; }

    static void provide(IAudio* service)
    {
      if (service == NULL)
      {
        // Revert to null service.
        service_ = &nullService_;
      }
      else
      {
        service_ = service;
      }
    }

  private:
    static IAudio* service_;
    static NullAudio nullService_;
  };
  //^8

  IAudio *Locator::service_ = NULL;
  NullAudio Locator::nullService_;

  //^13
  void enableAudioLogging()
  {
    // Decorate the existing service.
    IAudio *service = new LoggedAudio(Locator::getAudio());

    // Swap it in.
    Locator::provide(service);
  }
  //^13
}