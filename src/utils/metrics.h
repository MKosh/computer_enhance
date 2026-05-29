#pragma once

#include "types.h"
#include "string8.h"
#include <stdbool.h>

// const u64 microseconds = 1000000;

#ifdef _WIN32


#else

#include <x86intrin.h>
#include <sys/time.h>


////////////////////////////////////////////////////////////////////////////////
///
u64 readOsTimer(void);

////////////////////////////////////////////////////////////////////////////////
///
u64 readCpuTimer(void);

////////////////////////////////////////////////////////////////////////////////
///
u64 estimateCpuFreq(void);

#endif

/*
 * haversine.c
 * int main() {
 *   Profiler prof;
 *   profilerBegin(prof);
 *   profileBlockBegin("Read");
 *   readFile
 *   profileBlockEnd("Read");
 *   profileBlockBegin("Parse");
 *   parseJSON();
 *   profileBlockEnd("Parse");
 *   etc.
 * }
 *
 * parse.c
 * auto parseJSON() {
 *   Profiler prof;
 *   profilerBegin(prof);
 *   profileBlockBegin("Parse element");
 *   parseJsonElement();
 *   profileBlockEnd("Parse element");
 *   profilerEnd(prof);
 * }
 *
 * auto parseJsonElement() {
 *   
 * }
 *
 */

typedef struct TimeStamp TimeStamp;
typedef struct Profiler Profiler;
typedef struct ProfBlock ProfBlock;

struct TimeStamp {
  u64 time_elapsed_exclusive;
  u64 time_elapsed_inclusive;
  const char* label;
};

struct ProfBlock {
  const char* label;
  u64 OldTSCElapsedInclusive;
  u64 StartTSC;
  u64 ParentIndex;
  u64 StampIndex;
};

struct Profiler {
  u64 start;
  u64 stop;
  TimeStamp times[4096];
};


#define ProfileBlock(name, label) ProfBlock name = profilerBlockBegin(label, TimeStampIndex++)
#define ProfileBlockEnd(name) profilerBlockEnd(&(name));

void profilerInit(Profiler* prof);
void profilerBegin(Profiler* prof);
void profilerEnd(Profiler* prof);
ProfBlock profilerBlockBegin(const char* label, u64 index);
void profilerBlockEnd(ProfBlock* block);
void profilerEndAndPrint(Profiler* prof);
void profilerFree(Profiler* prof);
void printTimeElapsed(u64 total_time, TimeStamp* anchor, u64 freq);

typedef struct Times Times;
typedef struct Timer Timer;

struct Times {
  String key;
  u64 start;
  u64 stop;
};


struct Timer {
  u32 count;
  u32 capacity;
  Times* times;
};

#define TABLE_INIT_SIZE 67 
void timerInit(Timer* timer);
void timerFree(Timer* timer);
void timerStart(Timer* timer, const char* label);
void timerStop(Timer* timer, const char* label);
void timerPrint(Timer* timer);
void timerEnd(Timer* timer);
