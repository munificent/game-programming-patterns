#ifndef cpp_bytecode_h
#define cpp_bytecode_h

namespace Bytecode
{
  //^magic-api
  void setHealth(int wizard, int amount);
  void setWisdom(int wizard, int amount);
  void setAgility(int wizard, int amount);
  //^magic-api

  //^magic-api-fx
  void playSound(int soundId);
  void spawnParticles(int particleType);
  //^magic-api-fx

  void setHealth(int wizard, int amount) {}
  void setWisdom(int wizard, int amount) {}
  void setAgility(int wizard, int amount) {}
  void playSound(int soundId) {}
  void spawnParticles(int particleType) {}
  int getHealth(int wizard) { return 0; }


  //^instruction-enum
  enum Instruction
  {
    INST_SET_HEALTH,
    INST_SET_WISDOM,
    INST_SET_AGILITY,
    INST_PLAY_SOUND,
    INST_SPAWN_PARTICLES
    //^omit
    ,INST_LITERAL,
    INST_GET_HEALTH,
    INST_GET_WISDOM,
    INST_GET_AGILITY,
    INST_ADD,
    //^omit
  };
  //^instruction-enum

  static const int SOUND_BANG = 1;
  static const int PARTICLE_FLAME = 1;

  void interpretInstruction()
  {
    Instruction instruction = INST_SET_AGILITY;

    //^interpret-instruction
    switch (instruction) {
      case INST_SET_HEALTH:
        setHealth(0, 100);
        break;

      case INST_SET_WISDOM:
        setWisdom(0, 100);
        break;

      case INST_SET_AGILITY:
        setAgility(0, 100);
        break;

      case INST_PLAY_SOUND:
        playSound(SOUND_BANG);
        break;

      case INST_SPAWN_PARTICLES:
        spawnParticles(PARTICLE_FLAME);
        break;
        //^omit
      case INST_LITERAL:
      case INST_GET_HEALTH:
      case INST_GET_WISDOM:
      case INST_GET_AGILITY:
      case INST_ADD:
        break;
        //^omit
    }
    //^interpret-instruction
  }

  namespace NoParams
  {
  //^vm
  class VM
  {
  public:
    void interpret(char bytecode[], int size)
    {
      for (int i = 0; i < size; i++)
      {
        char instruction = bytecode[i];
        switch (instruction)
        {
          // Cases for each instruction...
            //^omit
          case INST_SPAWN_PARTICLES:
            break;
          default:
            break;
            //^omit
        }
      }
    }
  };
  //^vm
  }

  namespace Stack
  {
    //^stack
    class VM
    {
    public:
      VM()
      : stackSize_(0)
      {}

      // Other stuff...

    private:
      static const int MAX_STACK = 128;
      int stackSize_;
      int stack_[MAX_STACK];
    };
    //^stack
  }

  namespace PushPop
  {
    //^push-pop
    class VM
    {
    private:
      void push(int value)
      {
        // Check for stack overflow.
        assert(stackSize_ < MAX_STACK);
        stack_[stackSize_++] = value;
      }

      int pop()
      {
        // Make sure the stack isn't empty.
        assert(stackSize_ > 0);
        return stack_[--stackSize_];
      }

      // Other stuff...
      //^omit
      void interpret();
      static const int MAX_STACK = 128;
      int stackSize_;
      int stack_[MAX_STACK];
      //^omit
    };
    //^push-pop

    void VM::interpret()
    {
      int instruction = INST_SET_AGILITY;
      //^pop-instructions
      switch (instruction)
      {
        case INST_SET_HEALTH:
        {
          int amount = pop();
          int wizard = pop();
          setHealth(wizard, amount);
          break;
        }

        case INST_SET_WISDOM:
        case INST_SET_AGILITY:
          // Same as above...

        case INST_PLAY_SOUND:
          playSound(pop());
          break;

        case INST_SPAWN_PARTICLES:
          spawnParticles(pop());
          break;
      }
      //^pop-instructions

      char bytecode[123];
      int i = 0;
      switch (INST_LITERAL)
      {
        //^interpret-literal
        case INST_LITERAL:
        {
          // Read the next byte from the bytecode.
          int value = bytecode[++i];
          push(value);
          break;
        }
        //^interpret-literal

        //^read-stats
        case INST_GET_HEALTH:
        {
          int wizard = pop();
          push(getHealth(wizard));
          break;
        }

        case INST_GET_WISDOM:
        case INST_GET_AGILITY:
          // You get the idea...
        //^read-stats

        //^add
        case INST_ADD:
        {
          int b = pop();
          int a = pop();
          push(a + b);
          break;
        }
      }
    }
  }
}

#endif
