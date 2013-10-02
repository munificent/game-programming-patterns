#include <iostream>

namespace Singleton1
{
  //^1
  class FileSystem
  {
  public:
    static FileSystem& instance()
    {
      // Lazy initialize.
      static FileSystem *instance = new FileSystem();

      return *instance;
    }

  private:
    FileSystem() {}
  };
  //^1

  void test()
  {
    if (&FileSystem::instance() == NULL)
    {
      std::cout << "singleton is null!" << std::endl;
    }
    else
    {
      std::cout << "singleton is ok" << std::endl;
    }
  }
}

namespace Singleton2
{
  //^2
  class IFileSystem
  {
    virtual char* readFile(char* path) = 0;
    virtual void  writeFile(char* path, char* contents) = 0;
  };

  class PS3FileSystem : public IFileSystem
  {
    virtual char* readFile(char* path)
    { 
      // Use Sony file IO API...
      //^omit
      return NULL;
      //^omit
    }

    virtual void writeFile(char* path, char* contents)
    {
      // Use sony file IO API...
    }
  };

  class WiiFileSystem : public IFileSystem
  {
    virtual char* readFile(char* path)
    { 
      // Use Nintendo file IO API...
      //^omit
      return NULL;
      //^omit
    }
    
    virtual void  writeFile(char* path, char* contents)
    {
      // Use Nintendo file IO API...
    }
  };
  //^2
}

namespace Singleton3
{
#define PLAYSTATION3 1
#define WII 2
#define PLATFORM PLAYSTATION3

  class PS3FileSystem;
  class WiiFileSystem;

  //^3
  class IFileSystem
  {
  public:
    static IFileSystem& instance();

    virtual char* readFile(char* path) = 0;
    virtual void  writeFile(char* path, char* contents) = 0;

  protected:
    IFileSystem() {}
  };
  //^3

  class PS3FileSystem : public IFileSystem
  {
    virtual char* readFile(char* path)
    { 
      return NULL;
    }
    
    virtual void writeFile(char* path, char* contents) {}
  };

  class WiiFileSystem : public IFileSystem
  {
    virtual char* readFile(char* path)
    { 
      return NULL;
    }
    
    virtual void writeFile(char* path, char* contents) {}
  };

  //^4
  IFileSystem& IFileSystem::instance()
  {
    #if PLATFORM == PLAYSTATION3
      static IFileSystem *instance = new PS3FileSystem();
    #elif PLATFORM == WII
      static IFileSystem *instance = new WiiFileSystem();
    #endif

    return *instance;
  }
  //^4
}

namespace Singleton4
{
  //^5
  class FileSystem
  {
  public:
    static FileSystem& instance() { return instance_; }

  private:
    FileSystem() {}

    static FileSystem instance_;
  };
  //^5

  FileSystem FileSystem::instance_ = FileSystem();
}

namespace Singleton5
{
  //^6
  class FileSystem
  {
  public:
    FileSystem()
    {
      assert(!instantiated_);
      instantiated_ = true;
    }

    ~FileSystem() { instantiated_ = false; }

  private:
    static bool instantiated_;
  };

  bool FileSystem::instantiated_ = false;
  //^6
}

namespace Singleton7
{
  #define SCREEN_WIDTH 100
  #define SCREEN_HEIGHT 100

  //^8
  class Bullet
  {
  public:
    int getX() const { return x_; }
    int getY() const { return y_; }

    void setX(int x) { x_ = x; }
    void setY(int y) { y_ = y; }

  private:
    int x_, y_;
  };

  class BulletManager
  {
  public:
    Bullet* create(int x, int y)
    {
      Bullet* bullet = new Bullet();
      bullet->setX(x);
      bullet->setY(y);

      return bullet;
    }

    bool isOnScreen(Bullet& bullet)
    {
      return bullet.getX() >= 0 &&
             bullet.getX() < SCREEN_WIDTH &&
             bullet.getY() >= 0 &&
             bullet.getY() < SCREEN_HEIGHT;
    }

    void move(Bullet& bullet)
    {
      bullet.setX(bullet.getX() + 5);
    }
  };
  //^8
}

namespace Singleton8
{
  //^9
  class Bullet
  {
  public:
    Bullet(int x, int y) : x_(x), y_(y) {}

    bool isOnScreen()
    {
      return x_ >= 0 && x_ < SCREEN_WIDTH &&
             y_ >= 0 && y_ < SCREEN_HEIGHT;
    }

    void move() { x_ += 5; }

  private:
    int x_, y_;
  };
  //^9
}

namespace Singleton9
{
  class ILog
  {
  public:
    virtual void write(const char* text) = 0;
  };

  //^10
  class GameObject
  {
  protected:
    ILog& getLog() { return log_; }

  private:
    static ILog& log_;
  };

  class Enemy : public GameObject
  {
    void doSomething()
    {
      getLog().write("I can log!");
    }
  };
  //^10
}

namespace Singleton10
{
  class Log {};
  class FileSystem {};
  class AudioPlayer
  {
  public:
    virtual void play(int id) = 0;
  };

  //^11
  class World
  {
  public:
    static World& instance() { return instance_; }

    // Functions to set log_, et. al. ...

    Log&         getLog()         { return *log_; }
    FileSystem&  getFileSystem()  { return *fileSystem_; }
    AudioPlayer& getAudioPlayer() { return *audioPlayer_; }

  private:
    static World instance_;

    Log         *log_;
    FileSystem  *fileSystem_;
    AudioPlayer *audioPlayer_;
  };
  //^11

  World World::instance_ = World();

  void foo()
  {
    int VERY_LOUD_BANG = 0;
    //^12
    World::instance().getAudioPlayer().play(VERY_LOUD_BANG);
    //^12
  }
}
