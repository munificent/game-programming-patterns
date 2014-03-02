#ifndef cpp_bytecode_h
#define cpp_bytecode_h

namespace Bytecode
{
  //^magic-api
  void changeHealth(int wizard, int offset);
  void changeWisdom(int wizard, int offset);
  void changeAgility(int wizard, int offset);
  //^magic-api

  //^magic-api-fx
  void playSound(int soundId);
  void spawnParticles(int particleType);
  //^magic-api-fx

  void changeHealth(int wizard, int offset) {}
  void changeWisdom(int wizard, int offset) {}
  void changeAgility(int wizard, int offset) {}
  void playSound(int soundId) {}
  void spawnParticles(int particleType) {}

  //^instruction-enum
  enum Instruction
  {
    INST_CHANGE_HEALTH,
    INST_CHANGE_WISDOM,
    INST_CHANGE_AGILITY,
    INST_PLAY_SOUND,
    INST_SPAWN_PARTICLES
  };
  //^instruction-enum

  static const int SOUND_BANG = 1;
  static const int PARTICLE_FLAME = 1;

  void interpretInstruction()
  {
    Instruction instruction = INST_CHANGE_AGILITY;

    //^interpret-instruction
    switch (instruction) {
      case INST_CHANGE_HEALTH:
        changeHealth(1);
        break;

      case INST_CHANGE_WISDOM:
        changeWisdom(1);
        break;

      case INST_CHANGE_AGILITY:
        changeAgility(1);
        break;

      case INST_PLAY_SOUND:
        playSound(SOUND_BANG);
        break;

      case INST_SPAWN_PARTICLES:
        spawnParticles(PARTICLE_FLAME);
        break;
    }
    //^interpret-instruction
  }

  //^vm
  class VM
  {
  public:
    void interpret(unsigned char bytecode[], int size)
    {
      for (int ip = 0; ip < size; ip++)
      {
        unsigned char instruction = bytecode[ip];
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

#endif
