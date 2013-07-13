import 'dart:math';

const MAX_POS = 100;

const NUM_UNITS = 200;
const NUM_RUNS = 100;

final rand = new Random();

class Unit {
  final int pos;
  Unit(this.pos);
}

List<Unit> makeUnits(int seed) {
  var rand = new Random(seed);

  var units = [];
  for (var i = 0; i < NUM_UNITS; i++) {
    units.add(new Unit(rand.nextInt(MAX_POS)));
  }

  return units;
}

slow() {
  var compares = 0;
  var hits = 0;

  for (var i = 0; i < NUM_RUNS; i++) {
    var units = makeUnits(i);

    for (var a = 0; a < units.length - 1; a++) {
      for (var b = a + 1; b < units.length; b++) {
        compares++;
        if (units[a].pos == units[b].pos) {
          hits++;
        }
      }
    }
  }

  compares /= NUM_RUNS;
  hits /= NUM_RUNS;
  print("slow: $compares compares, $hits hits");
}

fast() {
  var compares = 0;
  var hits = 0;

  for (var i = 0; i < NUM_RUNS; i++) {
    var units = makeUnits(i);

    var slots = new List.generate(MAX_POS, (_) => []);
    for (var unit in units) slots[unit.pos].add(unit);

    for (var slot in slots) {
      for (var a = 0; a < slot.length - 1; a++) {
        for (var b = a + 1; b < slot.length; b++) {
          compares++;
          hits++;
        }
      }
    }
  }

  compares /= NUM_RUNS;
  hits /= NUM_RUNS;
  print("fast: $compares compares, $hits hits");
}

main() {
  slow();
  fast();
}