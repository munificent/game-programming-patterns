#pragma once

// Makes the argument appear to be used so that we don't get an unused
// variable warning for it. Lets us leave that warning on to catch unintended
// unused variables.
template <class T>
void use(const T& obj) {
  // Do nothing.
}

void assert(bool condition) {
  if (!condition) {
    printf("WTF\n");
    exit(1);
  }
}