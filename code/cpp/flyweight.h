//
//  flyweight.h
//  cpp
//
//  Created by Bob Nystrom on 9/21/13.
//  Copyright (c) 2013 Bob Nystrom. All rights reserved.
//

#ifndef cpp_flyweight_h
#define cpp_flyweight_h

namespace Flyweight
{
  class Mesh {};
  class Skeleton {};
  class Texture {};
  class Pose {};
  class Vector {};
  class Color {};

  static const int WIDTH = 1024;
  static const int HEIGHT = 1024;

  static Texture GRASS_TEXTURE;
  static Texture FOREST_TEXTURE;
  static Texture RIVER_TEXTURE;

  int random(int max) { return 0; }
  
  namespace HeavySoldier
  {
    //^heavy-soldier
    class Soldier
    {
    private:
      Mesh mesh_;
      Skeleton skeleton_;
      Texture texture_;
      Pose pose_;
      Vector position_;
      double height_;
      Color skinTone_;
    };
    //^heavy-soldier
  }

  namespace SplitSoldier
  {
    //^soldier-model
    class SoldierModel
    {
    private:
      Mesh mesh_;
      Skeleton skeleton_;
      Texture texture_;
    };
    //^soldier-model

    //^split-soldier
    class Soldier
    {
    private:
      SoldierModel* model_;
      Pose pose_;
      Vector position_;
      double height_;
      Color skinTone_;
    };
    //^split-soldier
  }

  namespace TerrainEnum
  {
    //^terrain-enum
    enum Terrain
    {
      TERRAIN_GRASS,
      TERRAIN_FOREST,
      TERRAIN_HILL,
      TERRAIN_RIVER
      // Other terrains...
    };
    //^terrain-enum

    //^enum-battlefield
    class Battlefield
    {
    private:
      Terrain tiles_[WIDTH * HEIGHT];
      //^omit
      int getMovementCost(int x, int y);
      bool isWater(int x, int y);
      //^omit
    };
    //^enum-battlefield

    //^enum-data
    int Battlefield::getMovementCost(int x, int y)
    {
      switch (tiles_[y * WIDTH + x])
      {
        case TERRAIN_GRASS: return 1;
        case TERRAIN_FOREST: return 3;
        case TERRAIN_HILL: return 4;
        case TERRAIN_RIVER: return 2;
          // Other terrains...
      }
    }

    bool Battlefield::isWater(int x, int y)
    {
      switch (tiles_[y * WIDTH + x])
      {
        case TERRAIN_GRASS: return false;
        case TERRAIN_FOREST: return false;
        case TERRAIN_HILL: return false;
        case TERRAIN_RIVER: return true;
          // Other terrains...
      }
    }
    //^enum-data
  }

  namespace TerrainClass
  {
    //^terrain-class
    class Terrain
    {
    public:
      Terrain(int movementCost,
              int opacity,
              bool isWater,
              Texture texture)
      : movementCost_(movementCost),
        opacity_(opacity),
        isWater_(isWater),
        texture_(texture)
      {}

      int getMovementCost() const { return movementCost_; }
      int getOpacity() const { return opacity_; }
      bool isWater() const { return isWater_; }
      const Texture& getTexture() const { return texture_; }

    private:
      int movementCost_;
      int opacity_;
      bool isWater_;
      Texture texture_;
    };
    //^terrain-class


    class Battlefield
    {
      //^omit
    public:
      const Terrain& getTile(int x, int y) const;
      //^omit
    private:
      Terrain* tiles_[WIDTH * HEIGHT];
      //^omit
      void generateTerrain();
      //^omit
    };

    //^generate
    void Battlefield::generateTerrain()
    {
      Terrain* grass = new Terrain(1, 0, false, GRASS_TEXTURE);
      Terrain* forest = new Terrain(3, 5, false, FOREST_TEXTURE);
      Terrain* river = new Terrain(1, 0, true, RIVER_TEXTURE);
      // TODO(bob): Manage memory!

      // Fill the battlefield with grass.
      for (int y = 0; y < HEIGHT; y++)
      {
        for (int x = 0; x < WIDTH; x++)
        {
          // Sprinkle some woods.
          if (random(10) == 0)
          {
            tiles_[y * WIDTH + x] = forest;
          }
          else
          {
            tiles_[y * WIDTH + x] = grass;
          }
        }
      }

      // Lay a river.
      int x = random(WIDTH);
      for (int y = 0; y < HEIGHT; y++) {
        tiles_[y * WIDTH + x] = river;
      }
    }
    //^generate

    //^get-tile
    const Terrain& Battlefield::getTile(int x, int y) const
    {
      return *tiles_[y * WIDTH + x];
    }
    //^get-tile

    void foo()
    {
      Battlefield battlefield;

      //^use-get-tile
      int cost = battlefield.getTile(2, 3).getMovementCost();
      //^use-get-tile
      use(cost);
    }
  }
}

#endif
