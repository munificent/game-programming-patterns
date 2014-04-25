#ifndef cpp_prototype_h
#define cpp_prototype_h

namespace PrototypePattern
{
  namespace Classes
  {
    //^monster-classes
    class Monster
    {
      // Stuff...
    };

    class Ghost : public Monster {};
    class Demon : public Monster {};
    class Sorcerer : public Monster {};
    //^monster-classes

    //^spawner-classes
    class Spawner
    {
    public:
      virtual ~Spawner() {}
      virtual Monster* spawnMonster() = 0;
    };

    class GhostSpawner : public Spawner
    {
    public:
      virtual Monster* spawnMonster()
      {
        return new Ghost();
      }
    };

    class DemonSpawner : public Spawner
    {
    public:
      virtual Monster* spawnMonster()
      {
        return new Demon();
      }
    };

    // You get the idea...
    //^spawner-classes
  }

  namespace Clone
  {
    //^virtual-clone
    class Monster
    {
    public:
      virtual ~Monster() {}
      virtual Monster* clone() = 0;

      // Other stuff...
    };
    //^virtual-clone

    //^clone-ghost
    class Ghost : public Monster {
    public:
      Ghost(int health, int speed)
      : health_(health),
        speed_(speed)
      {}

      virtual Monster* clone()
      {
        return new Ghost(health_, speed_);
      }

    private:
      int health_;
      int speed_;
    };
    //^clone-ghost


    //^spawner-clone
    class Spawner
    {
    public:
      Spawner(Monster* prototype)
      : prototype_(prototype)
      {}

      Monster* spawnMonster()
      {
        return prototype_->clone();
      }

    private:
      Monster* prototype_;
    };
    //^spawner-clone

    void test()
    {
      //^spawn-ghost-clone
      Monster* ghostPrototype = new Ghost(15, 3);
      Spawner* ghostSpawner = new Spawner(ghostPrototype);
      //^spawn-ghost-clone
      use(ghostSpawner);
    }
  }

  namespace Callbacks
  {
    class Monster
    {
      // Stuff...
    };

    class Ghost : public Monster {};

    //^callback
    Monster* spawnGhost()
    {
      return new Ghost();
    }
    //^callback

    //^spawner-callback
    typedef Monster* (*SpawnCallback)();

    class Spawner
    {
    public:
      Spawner(SpawnCallback spawn)
      : spawn_(spawn)
      {}

      Monster* spawnMonster()
      {
        return spawn_();
      }

    private:
      SpawnCallback spawn_;
    };
    //^spawner-callback

    void test()
    {
      //^spawn-ghost-callback
      Spawner* ghostSpawner = new Spawner(spawnGhost);
      //^spawn-ghost-callback
      use(ghostSpawner);
    }
  }

  namespace Templates
  {
    class Monster
    {
      // Stuff...
    };

    class Ghost : public Monster {};

    //^templates
    class Spawner
    {
    public:
      virtual ~Spawner() {}
      virtual Monster* spawnMonster() = 0;
    };

    template <class T>
    class SpawnerFor : public Spawner
    {
    public:
      virtual Monster* spawnMonster() { return new T(); }
    };
    //^templates

    void test()
    {
      //^use-templates
      Spawner* ghostSpawner = new SpawnerFor<Ghost>();
      //^use-templates
      use(ghostSpawner);
    }
  }
}

#endif
