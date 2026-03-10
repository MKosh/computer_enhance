#include "metrics.h"
#include "mem.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

const u64 microseconds = 1000000;

////////////////////////////////////////////////////////////////////////////////
///
void timerInit(Timer* timer)
{
  timer->count = 0;
  timer->capacity = TABLE_INIT_SIZE;
  timer->times = ALLOCATE(Times, TABLE_INIT_SIZE);
  for (size_t i = 1; i < timer->capacity; i++) {
    timer->times[i].key = (String){ .data = "", .count = 0 };
    timer->times[i].start = 0;
    timer->times[i].stop = 0;
  }
  timer->times[0].key = (String){ .data = "Total", .count = strlen("Total") };
  timer->times[0].start = readCpuTimer();
  timer->times[0].stop = 0;
  // printf("Starting timer at %lu", timer->times[0].start);
}

////////////////////////////////////////////////////////////////////////////////
///
void timerEnd(Timer* timer)
{
  timer->times[0].stop = readCpuTimer();
}

////////////////////////////////////////////////////////////////////////////////
///
void timerFree(Timer* timer)
{
  FREE_ARRAY(Times, timer->times, timer->capacity);
  timer->capacity = 0;
  timer->count = 0;
  timer->times = NULL;
}

////////////////////////////////////////////////////////////////////////////////
///
void timerStart(Timer* timer, const char* label)
{
  bool found_timer = false;
  for (size_t i = 0; i < timer->capacity; i++) {
    if (strcmp(timer->times[i].key.data, label) == 0) {
      // Found an already started timer
      found_timer = true;
    }
  }

  if (found_timer == true) {
    fprintf(stderr, "Found an already started timer.\n");
  } else {
    for (size_t i = 0; i < timer->capacity; i++) {
      if (strcmp(timer->times[i].key.data, "") == 0) {
        timer->times[i].key = (String){ .data = (char*)label, .count = strlen(label) };
        timer->times[i].start = readCpuTimer();
        found_timer = true;
        /* DEBUG */
        //printf("starting timer %s.\n", label);
        break;
      }
    }
  }

  if (!found_timer) {
    fprintf(stderr, "Could not find an open timer array slot.\n");
    exit(1);
  }
}

////////////////////////////////////////////////////////////////////////////////
///
void timerStop(Timer* timer, const char* label)
{
  for (size_t i = 0; i < timer->capacity; i++) {
    if (strcmp(timer->times[i].key.data, label) == 0) {
      timer->times[i].stop = readCpuTimer();

      /* DEBUG */
      //printf("starting timer %s.\n", label);
      return;
    }
  }

  fprintf(stderr, "Could not find Timer entry named %s.\n", label);
}

////////////////////////////////////////////////////////////////////////////////
///
void timerPrint(Timer* timer)
{
  timerEnd(timer);
  u64 total_time = timer->times[0].stop - timer->times[0].start;
  u64 freq = estimateCpuFreq();
  printf("Total time: %gms (%gs) (CPU freq: %luHz = %gGHz)\n", (double)total_time/freq*1000, (double)total_time/freq, freq, (double)freq/1000000000.);
  for (size_t i = 1; i < timer->capacity; i++) {
    if (timer->times[i].key.data[0] != '\0') {
      u64 time = timer->times[i].stop - timer->times[i].start;
      printf("  %s: %gms (%g%%)\n", timer->times[i].key.data, (double)time/freq*1000, (double)time/total_time*100);
    }
  }
}



////////////////////////////////////////////////////////////////////////////////
///
u64 readOsTimer(void)
{
  struct timeval value;
  gettimeofday(&value, 0);

  u64 result = microseconds*(u64)value.tv_sec + (u64)value.tv_usec;
  return result;
}

////////////////////////////////////////////////////////////////////////////////
///
u64 readCpuTimer(void)
{
  return __rdtsc();
}

////////////////////////////////////////////////////////////////////////////////
///
u64 estimateCpuFreq(void)
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
