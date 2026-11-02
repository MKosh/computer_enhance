#pragma once

#define ASSERT(cond) \
  do { if (!(cond)) { fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #cond); abort(); } } while (0)
