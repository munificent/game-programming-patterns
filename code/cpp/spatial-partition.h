#include <vector>
#include "expect.h"

namespace SpatialPartition
{
  class Unit
  {
  public:
    Unit(const char* name, int position)
    : name(name),
      position_(position),
      hit(NULL)
    {}

    const char* name;
    Unit* hit;

    int position() const { return position_; }
  private:
    int position_;
  };

  class Vector
  {
  public:
    Vector(int position)
    : position(position)
    {}

    int position;
  };

  bool operator==(int left, Vector right)
  {
    return left == right.position;
  }
  
  namespace NaiveCollision
  {
    std::vector<std::pair<Unit*, Unit*> > hits;
    
    void handleAttack(Unit* a, Unit* b)
    {
      hits.push_back(std::make_pair(a, b));
    }
    
    //^pairwise
    void handleMelee(Unit* units[], int numUnits)
    {
      for (int a = 0; a < numUnits - 1; a++)
      {
        for (int b = a + 1; b < numUnits; b++)
        {
          if (units[a]->position() == units[b]->position())
          {
            handleAttack(units[a], units[b]);
          }
        }
      }
    }
    //^pairwise

    void test()
    {
      Unit* units[5];
      units[0] = new Unit("a", 1);
      units[1] = new Unit("b", 2);
      units[2] = new Unit("c", 3);
      units[3] = new Unit("d", 2);
      units[4] = new Unit("e", 1);
      handleMelee(units, 5);
      EXPECT(hits[0].first == units[0]);
      EXPECT(hits[0].second == units[4]);
      EXPECT(hits[1].first == units[1]);
      EXPECT(hits[1].second == units[3]);
    }
  }

  namespace SimpleUnit
  {
    class Grid;
    
    //^unit-simple
    class Unit
    {
      friend class Grid;

    public:
      Unit(Grid* grid, double x, double y)
      : grid_(grid),
        x_(x),
        y_(y)
      {}

      void move(double x, double y);

    private:
      double x_, y_;
      Grid* grid_;
    };
    //^unit-simple
  }

  namespace SimpleGrid
  {
    class Grid;

    //^grid-simple
    class Grid
    {
    public:
      Grid()
      {
        // Clear the grid.
        for (int x = 0; x < NUM_CELLS; x++)
        {
          for (int y = 0; y < NUM_CELLS; y++)
          {
            cells_[x][y] = NULL;
          }
        }
      }

      static const int NUM_CELLS = 10;
      static const int CELL_SIZE = 20;
    private:
      Unit* cells_[NUM_CELLS][NUM_CELLS];
    };
    //^grid-simple
  }

  namespace LinkedUnit
  {
    //^unit-linked
    class Unit
    {
      // Previous code...
    private:
      Unit* prev_;
      Unit* next_;
    };
    //^unit-linked
  }

  namespace AddToGrid
  {
    class Unit;

    //^add-decl
    class Grid
    {
    public:
      void add(Unit* unit);

      // Previous code...
      //^omit
      static const int NUM_CELLS = 10;
      static const int CELL_SIZE = 20;
      Unit* cells_[NUM_CELLS][NUM_CELLS];
      //^omit
    };
    //^add-decl

    class Unit
    {
    public:
      Unit(Grid* grid, double x, double y);

      // Previous code...
      //^omit
      friend class Grid;

    private:
      double x_, y_;

      Grid* grid_;

      Unit* prev_;
      Unit* next_;
      //^omit
    };

    //^unit-ctor
    Unit::Unit(Grid* grid, double x, double y)
    : grid_(grid),
      x_(x),
      y_(y),
      prev_(NULL),
      next_(NULL)
    {
      grid_->add(this);
    }
    //^unit-ctor

    //^add
    void Grid::add(Unit* unit)
    {
      // Determine which grid cell it's in.
      int cellX = (int)(unit->x_ / Grid::CELL_SIZE);
      int cellY = (int)(unit->y_ / Grid::CELL_SIZE);

      // Add to the front of list for the cell it's in.
      unit->prev_ = NULL;
      unit->next_ = cells_[cellX][cellY];
      cells_[cellX][cellY] = unit;

      if (unit->next_ != NULL)
      {
        unit->next_->prev_ = unit;
      }
    }
    //^add
  }

  namespace FixedGrid
  {
    class Unit;

    void handleAttack(Unit* unit, Unit* other)
    {

    }
    
    class Grid
    {
    public:
      Grid()
      {
        // Clear the grid.
        for (int x = 0; x < NUM_CELLS; x++)
        {
          for (int y = 0; y < NUM_CELLS; y++)
          {
            cells_[x][y] = NULL;
          }
        }
      }

      static const int CELL_SIZE = 20;

      void move(Unit* unit, double x, double y);
      void add(Unit* unit);

      Unit* findAt(double x, double y);

      void handleMelee();
      void handleCell(Unit* unit);

      //^omit
      void dump();
      //^omit
    private:
      static const int NUM_CELLS = 10;

      Unit* cells_[NUM_CELLS][NUM_CELLS];
    };

    //^unit
    class Unit
    {
      friend class Grid;
      
    public:
      //^omit
      const char* name;
      //^omit
      Unit(Grid* grid, double x, double y)
      : grid_(grid),
        //^omit
        name(NULL),
        //^omit
        x_(x),
        y_(y),
        prev_(NULL),
        next_(NULL)
      {
        grid_->add(this);
      }

      void move(double x, double y);

    private:
      double x_, y_;

      Grid* grid_;
      
      Unit* prev_;
      Unit* next_;
    };
    //^unit

    //^unit-move
    void Unit::move(double x, double y)
    {
      grid_->move(this, x, y);
    }
    //^unit-move

    //^grid-move
    void Grid::move(Unit* unit, double x, double y)
    {
      // See which cell it was in.
      int oldCellX = (int)(unit->x_ / Grid::CELL_SIZE);
      int oldCellY = (int)(unit->y_ / Grid::CELL_SIZE);

      // See which cell it's moving to.
      int cellX = (int)(x / Grid::CELL_SIZE);
      int cellY = (int)(y / Grid::CELL_SIZE);

      unit->x_ = x;
      unit->y_ = y;
      
      // If it didn't change cells, we're done.
      if (oldCellX == cellX && oldCellY == cellY) return;

      // Unlink it from the list of its old cell.
      if (unit->prev_ != NULL)
      {
        unit->prev_->next_ = unit->next_;
      }

      if (unit->next_ != NULL)
      {
        unit->next_->prev_ = unit->prev_;
      }

      // If it's the head of a list, remove it.
      if (cells_[oldCellX][oldCellY] == unit)
      {
        cells_[oldCellX][oldCellY] = unit->next_;
      }

      // Add it back to the grid at its new cell.
      add(unit);
    }
    //^grid-move

    void Grid::add(Unit* unit)
    {
      // Determine which grid cell it's in.
      int cellX = (int)(unit->x_ / Grid::CELL_SIZE);
      int cellY = (int)(unit->y_ / Grid::CELL_SIZE);
      
      // Add to the front of list for the cell its in.
      unit->prev_ = NULL;
      unit->next_ = cells_[cellX][cellY];
      cells_[cellX][cellY] = unit;

      if (unit->next_ != NULL)
      {
        unit->next_->prev_ = unit;
      }
    }

    Unit* Grid::findAt(double x, double y)
    {
      int cellX = (int)(x / Grid::CELL_SIZE);
      int cellY = (int)(y / Grid::CELL_SIZE);

      Unit* unit = cells_[cellX][cellY];
      while (unit != NULL)
      {
        if (unit->x_ == x && unit->y_ == y) return unit;
        unit = unit->next_;
      }

      return NULL;
    }

    void Grid::dump()
    {
      for (int y = 0; y < NUM_CELLS; y++)
      {
        for (int x = 0; x < NUM_CELLS; x++)
        {
          Unit* unit = cells_[x][y];
          if (unit == NULL) continue;
          printf("%d, %d : ", x, y);
          while (unit != NULL)
          {
            printf("%s ", unit->name);
            unit = unit->next_;
          }
          printf("\n");
        }
      }
      printf("---\n");
    }

    // TODO(bob): Need tests for this.
    //^grid-melee
    void Grid::handleMelee()
    {
      for (int x = 0; x < NUM_CELLS; x++)
      {
        for (int y = 0; y < NUM_CELLS; y++)
        {
          handleCell(cells_[x][y]);
        }
      }
    }
    //^grid-melee

    //^handle-cell
    void Grid::handleCell(Unit* unit)
    {
      while (unit != NULL)
      {
        Unit* other = unit->next_;
        while (other != NULL)
        {
          if (unit->x_ == other->x_ &&
              unit->y_ == other->y_)
          {
            handleAttack(unit, other);
          }
          other = other->next_;
        }

        unit = unit->next_;
      }
    }
    //^handle-cell

    void test()
    {
      Grid grid;

      Unit a(&grid, 0, 0); a.name = "a";
      Unit b(&grid, 0, 0); b.name = "b";
      Unit c(&grid, 0, 0); c.name = "c";

      b.move(50, 65);
      c.move(55, 65);
      a.move(20, 100);
      c.move(22, 100);

      EXPECT(grid.findAt(20, 100) == &a);
      EXPECT(grid.findAt(50, 65) == &b);
      EXPECT(grid.findAt(22, 100) == &c);
    }
  }

  namespace AttackDistance
  {
    const int ATTACK_DISTANCE = 2;
    
    class Unit;

    void handleAttack(Unit* unit, Unit* other)
    {

    }

    int distance(Unit* a, Unit* b) { return 3; }

    class Grid
    {
    public:
      Grid()
      {
        // Clear the grid.
        for (int x = 0; x < NUM_CELLS; x++)
        {
          for (int y = 0; y < NUM_CELLS; y++)
          {
            cells_[x][y] = NULL;
          }
        }
      }

      static const int CELL_SIZE = 20;

      void move(Unit* unit, double x, double y);
      void add(Unit* unit);

      Unit* findAt(double x, double y);

      void handleMelee();
      void handleCell(int x, int y);
      void handleUnit(Unit* unit, Unit* other);
    private:
      static const int NUM_CELLS = 10;

      Unit* cells_[NUM_CELLS][NUM_CELLS];
    };

    class Unit
    {
      friend class Grid;

    public:
      Unit(Grid* grid, double x, double y)
      : grid_(grid),
        x_(x),
        y_(y),
        prev_(NULL),
        next_(NULL)
      {
        grid_->add(this);
      }

      void move(double x, double y);

    private:
      double x_, y_;

      Grid* grid_;

      Unit* prev_;
      Unit* next_;
    };

    // TODO(bob): Need tests for this.
    void Grid::handleMelee()
    {
      for (int x = 0; x < NUM_CELLS; x++)
      {
        for (int y = 0; y < NUM_CELLS; y++)
        {
          handleCell(x, y);
        }
      }
    }

    //^handle-neighbor
    //^handle-cell-unit
    void Grid::handleCell(int x, int y)
    {
      Unit* unit = cells_[x][y];
      while (unit != NULL)
      {
        // Handle other units in this cell.
        handleUnit(unit, unit->next_);
        //^omit handle-cell-unit
        
        // Also try the neighboring cells.
        if (x > 0 && y > 0) handleUnit(unit, cells_[x - 1][y - 1]);
        if (x > 0) handleUnit(unit, cells_[x - 1][y]);
        if (y > 0) handleUnit(unit, cells_[x][y - 1]);
        if (x > 0 && y < NUM_CELLS - 1)
        {
          handleUnit(unit, cells_[x - 1][y + 1]);
        }
        //^omit handle-cell-unit

        unit = unit->next_;
      }
    }
    //^handle-cell-unit
    //^handle-neighbor

    //^handle-unit
    void Grid::handleUnit(Unit* unit, Unit* other)
    {
      while (other != NULL)
      {
        //^handle-distance
        if (distance(unit, other) < ATTACK_DISTANCE)
        {
          handleAttack(unit, other);
        }
        //^handle-distance

        other = other->next_;
      }
    }
    //^handle-unit
  }

  void test()
  {
    printf("Testing Spatial Partition\n");
    NaiveCollision::test();
    FixedGrid::test();
  }
}
