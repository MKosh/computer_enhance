#pragma once

#include "types.h"
#include "string8.h"
#include "mem.h"
#include "allocator_interface.h"

#include <x86intrin.h>
#include <sys/time.h>
#include <sys/resource.h>

constexpr u32 Anchor_Count = KiB(4);

typedef enum ProfilingMetric {
  Metric_TimeElapsedExclusive = 0,
  Metric_TimeElapsedInclusive,
  Metric_ProcessedByteCount,
  Metric_PageFaults,
  Metric_HitCount,
  Metric_MetricCount,
} ProfilingMetric;

typedef struct Anchor {
  bool is_valid;
  String label;               // 
  u64 metrics[Metric_MetricCount]; // List of metrics
  // u64 time_elapsed_exclusive; // Does NOT contain children
  // u64 time_elapsed_inclusive; // Does contain children
  // u64 processed_byte_count;   //
  // u64 page_faults;            //
} Anchor;

typedef struct ProfilerBlock {
  String label;
  u64 old_TSC_elapsed_inclusive;
  u64 start_tsc;
  u64 page_faults;
  u64 parent_index;
  u64 anchor_index;
} ProfilerBlock;

typedef struct Profiler {
  u64 start;
  u64 stop;
  u64 global_prof_parent;
  Anchor anchors[Anchor_Count];
} Profiler;

Profiler profiler;

ProfilerBlock profileBlockBegin_(Allocator *allocator, const char *label, const char * file, u64 bytes);
void profileBlockEnd_(ProfilerBlock* block);
u64 readOsPageFaultCount(void);
u64 readOsTimer(void);
u64 readCpuTimer(void);
u64 estimateCpuFreq(void);
void profilerInit(Profiler *profiler);
void profilerBegin(Profiler *profiler);
void profilerEndAndPrint(Profiler *profiler);
void printAnchorMetrics(u64 total_time, Anchor *anchor, u64 freq);
void profilerFree(Profiler *profiler);
void printAnchorMetric(u64 total_time, Anchor *anchor, u32 metric, u64 freq);

#define CONCAT2(A, B) A##,##B
#define CONCAT(A, B) CONCAT2(A, B)
#define profileBlockBegin(name, label, allocator) ProfilerBlock (name) = profileBlockBegin_(allocator, label, __FILE__, 0)
#define profileBlockEnd(name) profileBlockEnd_(&(name))
#define profileBandwidthBegin(name, label, bytes, allocator) ProfilerBlock (name) = profileBlockBegin_(allocator, label, __FILE__, bytes)

