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
  static Texture HILL_TEXTURE;
  static Texture RIVER_TEXTURE;

  int random(int max) { return 0; }
  
  namespace HeavyTree
  {
    //^heavy-tree
    class Tree
    {
    private:
      Mesh mesh_;
      Texture bark_;
      Texture leaves_;
      Vector position_;
      double height_;
      double thickness_;
      Color barkTint_;
      Color leafTint_;
    };
    //^heavy-tree
  }

  namespace SplitTree
  {
    //^tree-model
    class TreeModel
    {
    private:
      Mesh mesh_;
      Texture bark_;
      Texture leaves_;
    };
    //^tree-model

    //^split-tree
    class Tree
    {
    private:
      TreeModel* model_;

      Vector position_;
      double height_;
      double thickness_;
      Color barkTint_;
      Color leafTint_;
    };
    //^split-tree
  }

  namespace TerrainEnum
  {
    //^terrain-enum
    enum Terrain
    {
      TERRAIN_GRASS,
      TERRAIN_HILL,
      TERRAIN_RIVER
      // Other terrains...
    };
    //^terrain-enum

    //^enum-world
    class World
    {
    private:
      Terrain tiles_[WIDTH][HEIGHT];
      //^omit
      int getMovementCost(int x, int y);
      bool isWater(int x, int y);
      //^omit
    };
    //^enum-world

    //^enum-data
    int World::getMovementCost(int x, int y)
    {
      switch (tiles_[x][y])
      {
        case TERRAIN_GRASS: return 1;
        case TERRAIN_HILL:  return 3;
        case TERRAIN_RIVER: return 2;
          // Other terrains...
      }
    }

    bool World::isWater(int x, int y)
    {
      switch (tiles_[x][y])
      {
        case TERRAIN_GRASS: return false;
        case TERRAIN_HILL:  return false;
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
              bool isWater,
              Texture texture)
      : movementCost_(movementCost),
        isWater_(isWater),
        texture_(texture)
      {}

      int getMovementCost() const { return movementCost_; }
      bool isWater() const { return isWater_; }
      const Texture& getTexture() const { return texture_; }

    private:
      int movementCost_;
      bool isWater_;
      Texture texture_;
    };
    //^terrain-class

    //^world-terrain-pointers
    class World
    {
      //^omit
    public:
      World()
      : grassTerrain_(1, false, GRASS_TEXTURE),
        hillTerrain_(3, false, HILL_TEXTURE),
        riverTerrain_(1, true, RIVER_TEXTURE)
      {}
      const Terrain& getTile(int x, int y) const;
      //^omit
    private:
      Terrain* tiles_[WIDTH][HEIGHT];

      // Other stuff...
      //^omit
      Terrain grassTerrain_;
      Terrain hillTerrain_;
      Terrain riverTerrain_;
      void generateTerrain();
      //^omit
    };
    //^world-terrain-pointers

    //^generate
    void World::generateTerrain()
    {
      // Fill the ground with grass.
      for (int x = 0; x < WIDTH; x++)
      {
        for (int y = 0; y < HEIGHT; y++)
        {
          // Sprinkle some hills.
          if (random(10) == 0)
          {
            tiles_[x][y] = &hillTerrain_;
          }
          else
          {
            tiles_[x][y] = &grassTerrain_;
          }
        }
      }

      // Lay a river.
      int x = random(WIDTH);
      for (int y = 0; y < HEIGHT; y++) {
        tiles_[x][y] = &riverTerrain_;
      }
    }
    //^generate

    //^get-tile
    const Terrain& World::getTile(int x, int y) const
    {
      return *tiles_[x][y];
    }
    //^get-tile

    void foo()
    {
      World world;

      //^use-get-tile
      int cost = world.getTile(2, 3).getMovementCost();
      //^use-get-tile
      use(cost);
    }
  }

  namespace WorldTerrain
  {
    class Terrain
    {
    public:
      Terrain(int movementCost,
              bool isWater,
              Texture texture)
      : movementCost_(movementCost),
        isWater_(isWater),
        texture_(texture)
      {}

      int getMovementCost() const { return movementCost_; }
      bool isWater() const { return isWater_; }
      const Texture& getTexture() const { return texture_; }

    private:
      int movementCost_;
      bool isWater_;
      Texture texture_;
    };
    
    //^world-terrain
    class World
    {
    public:
      World()
      : grassTerrain_(1, false, GRASS_TEXTURE),
        hillTerrain_(3, false, HILL_TEXTURE),
        riverTerrain_(1, true, RIVER_TEXTURE)
      {}

    private:
      Terrain grassTerrain_;
      Terrain hillTerrain_;
      Terrain riverTerrain_;

      // Other stuff...
    };
    //^world-terrain
  }
}

#endif
