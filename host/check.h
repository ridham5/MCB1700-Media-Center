#ifndef CHECK_H
#define CHECK_H

#include <stdio.h>

static int checks_run, checks_failed;

#define CHECK(cond) do { \
  checks_run++; \
  if (!(cond)) { checks_failed++; printf("  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } \
} while (0)

#define CHECK_EQ(a, b) do { \
  long _a = (long)(a), _b = (long)(b); \
  checks_run++; \
  if (_a != _b) { checks_failed++; printf("  FAIL %s:%d: %s == %s (%ld != %ld)\n", __FILE__, __LINE__, #a, #b, _a, _b); } \
} while (0)

#define TEST(name) static void name(void)
#define RUN(name) do { printf("%s\n", #name); name(); } while (0)

#define DONE() do { \
  printf("\n%d checks, %d failed\n", checks_run, checks_failed); \
  return checks_failed ? 1 : 0; \
} while (0)

#endif
