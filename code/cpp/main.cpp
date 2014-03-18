#include <iostream>

#include "hello-world.h"
#include "component.h"
#include "object-pool.h"
#include "service-locator.h"
#include "singleton.h"
#include "double-buffer.h"
#include "subclass-sandbox.h"
#include "type-object.h"
#include "game-loop.h"
#include "spatial-partition.h"
#include "state.h"
#include "update-method.h"
#include "dirty-flag.h"
#include "flyweight.h"
#include "command.h"
#include "observer.h"
#include "prototype.h"
#include "data-locality.h"
#include "event-queue.h"
#include "bytecode.h"

int main (int argc, char * const argv[])
{
  UnbufferedSlapstick::testComedy();
  SpatialPartition::test();
  ObserverPattern::test();

  return 0;
}
