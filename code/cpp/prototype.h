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
    class Sorceror : public Monster {};
    //^monster-classes

    //^generator-classes
    class Generator
    {
    public:
      virtual ~Generator() {}
      virtual Monster* generateMonster() = 0;
    };

    class GhostGenerator : public Generator
    {
    public:
      Monster* generateMonster()
      {
        return new Ghost();
      }
    };

    class DemonGenerator : public Generator
    {
    public:
      Monster* generateMonster()
      {
        return new Demon();
      }
    };

    // You get the idea...
    //^generator-classes
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

      Monster* clone()
      {
        return new Ghost(health_, speed_);
      }

    private:
      int health_;
      int speed_;
    };
    //^clone-ghost


    //^generator-clone
    class Generator
    {
    public:
      Generator(Monster* prototype)
      : prototype_(prototype)
      {}

      Monster* generateMonster()
      {
        return prototype_->clone();
      }

    private:
      Monster* prototype_;
    };
    //^generator-clone
  }

  namespace Callbacks
  {
    class Monster
    {
      // Stuff...
    };

    class Ghost : public Monster {};

    //^callback
    Monster* generateGhost()
    {
      return new Ghost();
    }
    //^callback

    //^generator-callback
    typedef Monster* (*GenerateCallback)();

    class Generator
    {
    public:
      Generator(GenerateCallback generate)
      : generate_(generate)
      {}

      Monster* generateMonster()
      {
        return generate_();
      }

    private:
      GenerateCallback generate_;
    };
    //^generator-callback

    void test()
    {
      //^generate-ghost-callback
      Generator* ghostGenerator = new Generator(generateGhost);
      //^generate-ghost-callback
      use(ghostGenerator);
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
    class Generator
    {
    public:
      virtual ~Generator() {}
      virtual Monster* generateMonster() = 0;
    };

    template <class T>
    class GeneratorFor : public Generator
    {
    public:
      Monster* generateMonster() { return new T(); }
    };
    //^templates

    void test()
    {
      //^use-templates
      Generator* ghostGenerator = new GeneratorFor<Ghost>();
      //^use-templates
      use(ghostGenerator);
    }
  }
}

#endif
