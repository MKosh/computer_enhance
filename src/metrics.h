#include "utils.h"

const u64 microseconds = 1000000;

#ifdef _WIN32


#else

#include <x86intrin.h>
#include <sys/time.h>


inline u64 readOsTimer(void)
{
  struct timeval value;
  gettimeofday(&value, 0);

  u64 result = microseconds*(u64)value.tv_sec + (u64)value.tv_usec;
  return result;
}

inline u64 readCpuTimer(void)
{
  return __rdtsc();
}

inline u64 estimateCpuFreq(void)
{

  u64 milliseconds_to_wait = 100;
  u64 os_freq = microseconds;

  u64 cpu_start = readCpuTimer();
  u64 os_start = readOsTimer();
  u64 os_end = 0;
  u64 os_elapsed = 0;
  u64 os_wait_time = os_freq * milliseconds_to_wait / 1000;

  while (os_elapsed < os_wait_time) {
    os_end = readOsTimer();
    os_elapsed = os_end - os_start;
  }

  u64 cpu_end = readCpuTimer();

  u64 cpu_elapsed = cpu_end - cpu_start;
  u64 cpu_freq = 0;
  if (os_elapsed) {
    cpu_freq = os_freq * cpu_elapsed / os_elapsed;
  }

  return cpu_freq;
}

#endif
