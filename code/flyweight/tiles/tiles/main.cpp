//
//  main.cpp
//  tiles
//
//  Created by Bob Nystrom on 9/14/13.
//  Copyright (c) 2013 Bob Nystrom. All rights reserved.
//

#include <iostream>
#include <sys/time.h>

#define ADD_OTHER_STUFF

static const int SIZE = 128;

struct timeval startTime;

void startProfile()
{
  gettimeofday(&startTime, NULL);
}

void endProfile(const char* message)
{
  struct timeval endTime;
  gettimeofday(&endTime, NULL);

  endTime.tv_sec -= startTime.tv_sec;
  endTime.tv_usec -= startTime.tv_usec;

  printf("%s: %lds %dμs\n", message, endTime.tv_sec, endTime.tv_usec);
}

int pickTile(int x, int y) {
  return (x * 7 + y * 13) % 4;
}

namespace tileEnum
{
  enum Tile
  {
    TILE_GRASS = 0,
    TILE_FOREST,
    TILE_MOUNTAIN,
    TILE_WATER
  };

  struct World
  {
    Tile tiles[SIZE * SIZE];
    World()
    {
      for (int y = 0; y < SIZE; y++)
      {
        for (int x = 0; x < SIZE; x++)
        {
          tiles[y * SIZE + x] = (Tile)pickTile(x, y);
        }
      }
    }

    int addTiles()
    {
      int sum = 0;
      for (int y = 0; y < SIZE; y++)
      {
        for (int x = 0; x < SIZE; x++)
        {
          switch (tiles[y * SIZE + x])
          {
            case TILE_GRASS: sum += 1; break;
            case TILE_FOREST: sum += 2; break;
            case TILE_MOUNTAIN: sum += 3; break;
            case TILE_WATER: sum += 4; break;
          }
        }
      }
      return sum;
    }
  };
};

namespace tileByte
{
  enum Tile
  {
    TILE_GRASS = 0,
    TILE_FOREST,
    TILE_MOUNTAIN,
    TILE_WATER
  };

  struct World
  {
    unsigned char tiles[SIZE * SIZE];
    World()
    {
      for (int y = 0; y < SIZE; y++)
      {
        for (int x = 0; x < SIZE; x++)
        {
          tiles[y * SIZE + x] = pickTile(x, y);
        }
      }
    }

    int addTiles()
    {
      int sum = 0;
      for (int y = 0; y < SIZE; y++)
      {
        for (int x = 0; x < SIZE; x++)
        {
          switch (tiles[y * SIZE + x])
          {
            case TILE_GRASS: sum += 1; break;
            case TILE_FOREST: sum += 2; break;
            case TILE_MOUNTAIN: sum += 3; break;
            case TILE_WATER: sum += 4; break;
          }
        }
      }
      return sum;
    }
  };
};

namespace tileField
{
  struct Tile
  {
    int weight;
#ifdef ADD_OTHER_STUFF
    int other[16];
#endif
  };

  struct World
  {
    Tile tileTypes[4];
    
    Tile* tiles[SIZE * SIZE];
    World()
    {
      tileTypes[0].weight = 1;
      tileTypes[1].weight = 2;
      tileTypes[2].weight = 3;
      tileTypes[3].weight = 4;

      for (int y = 0; y < SIZE; y++)
      {
        for (int x = 0; x < SIZE; x++)
        {
          tiles[y * SIZE + x] = &tileTypes[pickTile(x, y)];
        }
      }
    }

    int addTiles()
    {
      int sum = 0;
      for (int y = 0; y < SIZE; y++)
      {
        for (int x = 0; x < SIZE; x++)
        {
          sum += tiles[y * SIZE + x]->weight;
        }
      }
      return sum;
    }
  };
};

namespace tileVirtual
{
  struct Tile
  {
    virtual int weight() = 0;
  };

  struct GrassTile : public Tile
  {
    virtual int weight() { return 1; }
  };

  struct ForestTile : public Tile
  {
    virtual int weight() { return 2; }
  };

  struct MountainTile : public Tile
  {
    virtual int weight() { return 3; }
  };

  struct WaterTile : public Tile
  {
    virtual int weight() { return 4; }
  };

  struct World
  {
    GrassTile grass;
    ForestTile forest;
    MountainTile mountain;
    WaterTile water;
    Tile* tileTypes[4];

    Tile* tiles[SIZE * SIZE];
    World()
    {
      tileTypes[0] = &grass;
      tileTypes[1] = &forest;
      tileTypes[2] = &mountain;
      tileTypes[3] = &water;

      for (int y = 0; y < SIZE; y++)
      {
        for (int x = 0; x < SIZE; x++)
        {
          tiles[y * SIZE + x] = tileTypes[pickTile(x, y)];
        }
      }
    }

    int addTiles()
    {
      int sum = 0;
      for (int y = 0; y < SIZE; y++)
      {
        for (int x = 0; x < SIZE; x++)
        {
          sum += tiles[y * SIZE + x]->weight();
        }
      }
      return sum;
    }
  };
};

int main(int argc, const char * argv[])
{
  for (int i = 0; i < 1; i++)
  {
    {
      tileEnum::World world;
      startProfile();
      int sum = world.addTiles();
      endProfile("tile enum");
      printf("%d\n", sum);
    }

    {
      tileByte::World world;
      startProfile();
      int sum = world.addTiles();
      endProfile("tile byte");
      printf("%d\n", sum);
    }

    {
      tileField::World world;
      startProfile();
      int sum = world.addTiles();
      endProfile("tile field");
      printf("%d\n", sum);
    }

    {
      tileVirtual::World world;
      startProfile();
      int sum = world.addTiles();
      endProfile("tile virtual");
      printf("%d\n", sum);
    }
  }
  
  return 0;
}
