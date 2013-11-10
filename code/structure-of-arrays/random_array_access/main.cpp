#include <stdio.h>

#include "utils.h"

// Based on http://stackoverflow.com/questions/1126529/what-is-the-cost-of-an-l1-cache-miss
//
// Tries to show effects of cache misses by modifying elements of an array
// twice, once in order and once in random order.
//
// On my machine, I get results like:
// ordered:    43.9250ms
//  random:   308.4460ms
//              7.02x
// ordered:    14.6060ms
//  random:   314.2920ms
//             21.52x
// ordered:    14.3640ms
//  random:   304.5980ms
//             21.21x
// ordered:    14.3160ms
//  random:   316.0700ms
//             22.08x
// ordered:    15.6090ms
//  random:   317.1620ms
//             20.32x
// ordered:    15.7320ms
//  random:   281.0440ms
//             17.86x

void accessRandomArray(int iterations, int length)
{
  srand((unsigned int)time(0));

  // Allocate the data as one single chunk of memory.
  int* nodes = new int[length];

  // Create array that give the access order for the data. Going through the
  // indirect for both the ordered and random traversals ensures the randomness
  // itself doesn't add any overhead.
  int* orderedIndexes = new int[length];
  for (int i = 0; i < length; i++)
  {
    orderedIndexes[i] = i;
  }

  int* randomIndexes = shuffledArray(length);

  long sum = 0;

  // Run the whole thing a few times to make sure the numbers are stable.
  for (int a = 0; a < iterations; a++)
  {
    // Write the data in order.
    startProfile();
    for (int i = 0; i < length; i++) nodes[orderedIndexes[i]] = i;
    float ordered = endProfile("ordered: ");

    // Actually use the data so the compiler doesn't optimize it to nothing.
    for (int i = 0; i < iterations; i++) sum += nodes[i];

    // Now access the array positions in a "random" order.
    startProfile();
    for (int i = 0; i < length; i++) nodes[randomIndexes[i]] = i;
    endProfile(" random: ", ordered);

    // Actually use the data so the compiler doesn't optimize it to nothing.
    for (int i = 0; i < iterations; i++) sum -= nodes[i];
  }

  // Actually use the data so the compiler doesn't optimize it to nothing.
  if (sum != 0) printf("Got wrong sum %ld (should be 0)\n", sum);

  delete [] nodes;
  delete [] orderedIndexes;
  delete [] randomIndexes;
}

int main( int argc, char* argv[] )
{
  accessRandomArray(6, 20000000);
}