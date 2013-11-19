#pragma once

#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <vector>

clock_t startTime;

void startProfile()
{
  startTime = clock();
}

float endProfile(const char* message)
{
  float elapsed = (float)(clock() - startTime) * 1000.0f / CLOCKS_PER_SEC;
  printf("%s%10.4fms\n", message, elapsed);
  return elapsed;
}

float endProfile(const char* message, float comparison)
{
  float elapsed = (float)(clock() - startTime) * 1000.0f / CLOCKS_PER_SEC;
  printf("%s%10.4fms  %6.2fx\n", message, elapsed, elapsed / comparison);
  return elapsed;
}

// Get random value in the given range (half-inclusive).
int randRange(int m, int n)
{
  return rand() % (n - m) + m;
}

// Make sure the code leading to the argument to this call isn't compiled out.
void use(long sum)
{
  if (sum == 123) printf("!");
}

template <typename T>
T median(std::vector<T> items)
{
  std::sort(items.begin(), items.end());
  if (items.size() % 2 == 0)
  {
    return (items[items.size() / 2] + items[items.size() / 2 + 1]) / 2;
  }
  else
  {
    return items[items.size() / 2];
  }
}

int* shuffledArray(int length)
{
  int* array = new int[length];
  for (int i = 0; i < length; i++) array[i] = i;

  for (int i = 0; i < length; i++)
  {
    int j = randRange(i, length);
    int t = array[i];
    array[i] = array[j];
    array[j] = t;
  }

  return array;
}

template <class T>
void shuffle(T* array, int length)
{
  for (int i = 0; i < length; i++)
  {
    int j = randRange(i, length);
    T temp = array[i];
    array[i] = array[j];
    array[j] = temp;
  }
}
