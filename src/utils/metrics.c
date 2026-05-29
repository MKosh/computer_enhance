#include "metrics.h"
#include "mem.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
/// Static globals
static const u64 microseconds = 1000000;
#define TABLE_MAX_LOAD 0.75
/// Static globals
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// Extern globals
Profiler prof = {0};
extern i32 DEBUG_;
u64 TimeStampIndex = 1;
u64 GlobalProfParent;
/// Extern globals
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
///
void profilerInit(Profiler* prof) {
  prof->start = 0;
  prof->stop = 0;
  // prof->index = 1;
  // prof->capacity = 4096;
  // prof->parentIndex = 1;
  // prof->times = (TimeStamp*)malloc(sizeof(TimeStamp)*prof->capacity);
  if (DEBUG_) printf("Profiler initialized.\n");
}

////////////////////////////////////////////////////////////////////////////////
///
void profilerBegin(Profiler* prof)
{
  prof->start = readCpuTimer();
}

////////////////////////////////////////////////////////////////////////////////
///
void profilerEnd(Profiler* prof)
{
  prof->stop = readCpuTimer();
}

////////////////////////////////////////////////////////////////////////////////
///
ProfBlock profilerBlockBegin([[maybe_unused]]const char* label, u64 index)
{
  // if (prof->index + 1 > prof->capacity) {
  //   prof->capacity *= 2;
  //   prof->times = realloc(prof->times, sizeof(TimeStamp) * prof->capacity);
  // }
 
  // block->ParentIndex = GlobalProfParent;
  // block->StampIndex = index;
  // block->label = label;
  //
  // TimeStamp* anchor = prof.times + block->StampIndex;
  // block->OldTSCElapsedInclusive = anchor->time_elapsed_inclusive;
  //
  // GlobalProfParent = index;
  // block->StartTSC = readCpuTimer();
  assert(index < 4096 && "Too many profiler time stamps");
  ProfBlock block;

  block.ParentIndex = GlobalProfParent;
  block.StampIndex = index;
  block.label = label;

  TimeStamp* anchor = prof.times + block.StampIndex;
  block.OldTSCElapsedInclusive = anchor->time_elapsed_inclusive;

  GlobalProfParent = index;
  block.StartTSC = readCpuTimer();

  return block;
  
}

////////////////////////////////////////////////////////////////////////////////
///
void profilerBlockEnd(ProfBlock* block)
{
  u64 elapsed = readCpuTimer() - block->StartTSC;
  GlobalProfParent = block->ParentIndex;

  TimeStamp* parent = prof.times + block->ParentIndex;
  TimeStamp* anchor = prof.times + block->StampIndex;

  parent->time_elapsed_exclusive -= elapsed;
  anchor->time_elapsed_exclusive += elapsed;
  anchor->time_elapsed_inclusive = block->OldTSCElapsedInclusive + elapsed;

  anchor->label = block->label;
}

void printTimeElapsed(u64 total_time, TimeStamp* anchor, u64 freq)
{
  f64 percent = 100.0 * ((f64)anchor->time_elapsed_exclusive / (f64)total_time);
  printf("  %s: %gms (%.2f%%", anchor->label, 1000.0 * ((f64)anchor->time_elapsed_exclusive / (f64)freq), percent);
  if (anchor->time_elapsed_inclusive != anchor->time_elapsed_exclusive) {
    f64 percent_with_children = 100.0 * ((f64)anchor->time_elapsed_inclusive / (f64)total_time);
    printf(", %.2f%% w/ children", percent_with_children);
  }

  printf(")\n");
}
////////////////////////////////////////////////////////////////////////////////
///
void profilerEndAndPrint(Profiler* prof)
{
  prof->stop = readCpuTimer();
  u64 cpu_freq = estimateCpuFreq();

  u64 total_elapsed = prof->stop - prof->start;

  if (cpu_freq) {
    printf("\nTotal time: %0.4fms (CPU freq: %gGHz)\n", 1000.0 * ((f64)total_elapsed / (f64)cpu_freq), (f64)cpu_freq/1000000000.);
  }

  for (u32 index = 0; index < 4096; ++index) {
    TimeStamp* anchor = prof->times + index;
    if (anchor->time_elapsed_inclusive) {
      printTimeElapsed(total_elapsed, anchor, cpu_freq);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
void timerInit(Timer* timer)
{
  timer->count = 0;
  timer->capacity = TABLE_INIT_SIZE;
  timer->times = ALLOCATE(Times, TABLE_INIT_SIZE);
  for (size_t i = 1; i < timer->capacity; i++) {
    timer->times[i].key = (String){ .str = "", .len = 0 };
    timer->times[i].start = 0;
    timer->times[i].stop = 0;
  }
  timer->times[0].key = (String){ .str = "Total", .len = strlen("Total") };
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
    if (strcmp(timer->times[i].key.str, label) == 0) {
      // Found an already started timer
      found_timer = true;
    }
  }

  if (found_timer == true) {
    fprintf(stderr, "Found an already started timer.\n");
  } else {
    for (size_t i = 0; i < timer->capacity; i++) {
      if (strcmp(timer->times[i].key.str, "") == 0) {
        timer->times[i].key = (String){ .str = (char*)label, .len = strlen(label) };
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
    if (strcmp(timer->times[i].key.str, label) == 0) {
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
    if (timer->times[i].key.str[0] != '\0') {
      u64 time = timer->times[i].stop - timer->times[i].start;
      printf("  %s: %gms (%g%%)\n", timer->times[i].key.str, (double)time/freq*1000, (double)time/total_time*100);
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
