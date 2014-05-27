#include "common.h"

namespace Subclasses
{
  //^1
  class Monster
  {
  public:
    virtual ~Monster() {}
    virtual const char* getAttack() = 0;

  protected:
    Monster(int startingHealth)
    : health_(startingHealth)
    {}

  private:
    int health_; // Current health.
  };
  //^1

  //^2
  class Dragon : public Monster
  {
  public:
    Dragon() : Monster(230) {}

    virtual const char* getAttack()
    {
      return "The dragon breathes fire!";
    }
  };

  class Troll : public Monster
  {
  public:
    Troll() : Monster(48) {}

    virtual const char* getAttack()
    {
      return "The troll clubs you!";
    }
  };
  //^2
}

namespace NoInheritance
{
  //^3
  class Breed
  {
  public:
    Breed(int health, const char* attack)
    : health_(health),
      attack_(attack)
    {}

    int getHealth() { return health_; }
    const char* getAttack() { return attack_; }

  private:
    int health_; // Starting health.
    const char* attack_;
  };
  //^3

  //^4
  class Monster
  {
  public:
    Monster(Breed& breed)
    : health_(breed.getHealth()),
      breed_(breed)
    {}

    const char* getAttack()
    {
      return breed_.getAttack();
    }

  private:
    int    health_; // Current health.
    Breed& breed_;
  };
  //^4

  void testUsage()
  {
    Breed& someBreed = *(new Breed(123, "attack"));

    //^7
    Monster* monster = new Monster(someBreed);
    //^7
    use(monster);
  }
}

namespace BreedCtor
{
  class Breed;

  class Monster
  {
  public:
    Monster(Breed& breed) {}
  };

  //^5
  class Breed
  {
  public:
    Monster* newMonster() { return new Monster(*this); }

    // Previous Breed code...
    //^omit
    Breed(int health, const char* attack)
    : health_(health),
      attack_(attack)
    {}

    int getHealth() { return health_; }
    const char* getAttack() { return attack_; }

  private:
    int health_; // Starting health.
    const char* attack_;
    //^omit
  };
  //^5

  void testUsage()
  {
    Breed& someBreed = *(new Breed(1, "foo"));

    //^8
    Monster* monster = someBreed.newMonster();
    //^8
    use(monster);
  }
}

namespace BreedCtorMonster
{
  class Breed
  {
  public:
    int getHealth() { return 0; }
    const char* getAttack() { return "s"; }
  };

  //^6
  class Monster
  {
    friend class Breed;

  public:
    const char* getAttack() { return breed_.getAttack(); }

  private:
    Monster(Breed& breed)
    : health_(breed.getHealth()),
      breed_(breed)
    {}

    int health_; // Current health.
    Breed& breed_;
  };
  //^6
}

namespace Inheritance
{
  //^9
  class Breed
  {
  public:
    Breed(Breed* parent, int health, const char* attack)
    : parent_(parent),
      health_(health),
      attack_(attack)
    {}

    int         getHealth();
    const char* getAttack();

  private:
    Breed*      parent_;
    int         health_; // Starting health.
    const char* attack_;
  };
  //^9

  //^10
  int Breed::getHealth()
  {
    // Override.
    if (health_ != 0 || parent_ == NULL) return health_;

    // Inherit.
    return parent_->getHealth();
  }

  const char* Breed::getAttack()
  {
    // Override.
    if (attack_ != NULL || parent_ == NULL) return attack_;

    // Inherit.
    return parent_->getAttack();
  }
  //^10
}

namespace CopyDown
{
  class Breed
  {
  public:
    //^copy-down
    Breed(Breed* parent, int health, const char* attack)
    : health_(health),
      attack_(attack)
    {
      // Inherit non-overridden attributes.
      if (parent != NULL)
      {
        if (health == 0) health_ = parent->getHealth();
        if (attack == NULL) attack_ = parent->getAttack();
      }
    }
    //^copy-down

    //^copy-down-access
    int         getHealth() { return health_; }
    const char* getAttack() { return attack_; }
    //^copy-down-access

  private:
    int         health_; // Starting health.
    const char* attack_;
  };
}

namespace ExposeBreed
{
  class Breed;

  //^11
  class Monster
  {
  public:
    Breed& getBreed() { return breed_; }

    // Existing code...
    //^omit
    Breed& breed_;
    //^omit
  };
  //^11
}

namespace OverrideAttack
{
  class Breed
  {
  public:
    Breed(const char* attack)
    : attack_(attack)
    {}

    const char* getAttack() { return attack_; }

  private:
    const char* attack_;
  };

  class Monster
  {
  public:
    Monster(Breed& breed)
    : breed_(breed)
    {}

    const char* getAttack();

  private:
    int health_;
    Breed& breed_;
  };

  #define LOW_HEALTH 1

  //^12
  const char* Monster::getAttack()
  {
    if (health_ < LOW_HEALTH)
    {
      return "The monster flails weakly.";
    }

    return breed_.getAttack();
  }
  //^12
}