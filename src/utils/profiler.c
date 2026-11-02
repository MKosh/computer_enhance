#include "profiler.h"

#include <stdio.h>

static const u64 microseconds = 1000000;
Profiler profiler = { 0 };

void profilerInit(Profiler *profiler)
{
  profiler->start = 0;
  profiler->stop = 0;
  Anchor anchor;
  for (u32 i = 0; i < Anchor_Count; i++) {
    anchor = profiler->anchors[i];
    anchor.is_valid = false;
    anchor.label = (String){ .len = 0, .str = nullptr };
    for (u32 m = 0; m < Metric_MetricCount; m++) {
      anchor.metrics[m] = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
///
void profilerBegin(Profiler* profiler)
{
  profiler->start = readCpuTimer();
}

ProfilerBlock profileBlockBegin_(Allocator *allocator, const char *label, const char *file, u64 bytes)
{
  ProfilerBlock block = { 0 };
  // Global hashmap in the global profiler so that any code file that has access
  // to the hashmap can insert anchors into it.
  // The idea is to keep the indices safe across translation units by including
  // the file in the hash. Then, at the end the profiler can loop over all
  // slots in the hash map and print out the metrics. Will still need a top
  // level call to init and endAndPrint. Maybe I can liter the parser code with
  // profiling blocks as well, including the init line, but has a preprocessor
  // directive turn that init line on and off as needed. Like if I define something
  // in the main file then the init line will not be used in the parser file.
  // printf("%s: %s\n", file, label);
  // u64 label_length = strlen(label);
  // u64 file_length = strlen(file);
  // char *string = malloc(label_length + file_length + 1);
  // strncpy(string, label, label_length);
  // strcat(string, file);
  // string[label_length + file_length] = '\0';
  // printf("%s\n", string);

  // String string = string_from_cstr(label, alloc);
  // String str2 = string_from_cstr(file, alloc);
  String string = string_cstr_concat(label, file, allocator);

  // String str = string_from_cstr(string);
  u64 hash = string_hash(string) % Anchor_Count;

  //
  block.parent_index = profiler.global_prof_parent;
  block.anchor_index = hash;
  block.label = string;

  Anchor *anchor = profiler.anchors + block.anchor_index;
  block.old_TSC_elapsed_inclusive = anchor->metrics[Metric_TimeElapsedInclusive];
  anchor->is_valid = true;
  anchor->metrics[Metric_ProcessedByteCount] += bytes;
  anchor->metrics[Metric_PageFaults] += readOsPageFaultCount();

  profiler.global_prof_parent = hash;
  block.start_tsc = readCpuTimer();
  //
  return block;
}

////////////////////////////////////////////////////////////////////////////////
///
u64 readCpuTimer(void)
{
  return __rdtsc();
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
u64 readOsPageFaultCount(void)
{
  struct rusage usage = {};
  if (getrusage(RUSAGE_SELF, &usage) == -1) {
    // failure
    fprintf(stderr, "Error: getrusage failed.\n");
    return 0;
  }

  // ru_minflt the number of page faults serviced without any I/O activity
  // ru_majflt the number of page faults serviced that required I/O activity
  u64 result = usage.ru_minflt + usage.ru_majflt;
  return result;
}

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

void profilerEndAndPrint(Profiler *profiler)
{
  profiler->stop = readCpuTimer();
  u64 cpu_freq = estimateCpuFreq();

  u64 total_time_elapsed = profiler->stop - profiler->start;

  if (cpu_freq) {
    f64 seconds = ((f64)total_time_elapsed / (f64)cpu_freq);
    printf("\nTotal time: %0.4fms = %0.4fs (CPU freq: %gGHz)\n", 1000.0 * seconds, seconds, (f64)cpu_freq/1000000000.);
  }

  for (u32 index = 0; index < Anchor_Count; ++index) {
    Anchor *anchor = profiler->anchors + index;
    if (!anchor->is_valid) { continue; }
    if (anchor->metrics[Metric_TimeElapsedExclusive]) {
      f64 percent = 100.0 * ((f64)anchor->metrics[Metric_TimeElapsedExclusive] / (f64) total_time_elapsed);
      printf("  %s: %gms (%.2f%%", anchor->label.str, 1000.0 * ((f64)anchor->metrics[Metric_TimeElapsedExclusive] / (f64)cpu_freq), percent);
      if (anchor->metrics[Metric_TimeElapsedInclusive] != anchor->metrics[Metric_TimeElapsedExclusive]) {
        f64 percent_with_children = 100.0 * ((f64)anchor->metrics[Metric_TimeElapsedInclusive] / (f64)total_time_elapsed);
        printf(", %.2f%% w/ children", percent_with_children);
      }     
      printf(")");
    }

    if (anchor->metrics[Metric_ProcessedByteCount]) {
      f64 megabyte = 1024.0f * 1024.0f;
      f64 gigabyte = megabyte * 1024.0f;

      f64 seconds = (f64)anchor->metrics[Metric_TimeElapsedInclusive] / (f64)cpu_freq;
      f64 bytes_per_second = (f64)anchor->metrics[Metric_ProcessedByteCount] / seconds;
      f64 megabytes = (f64)anchor->metrics[Metric_ProcessedByteCount] / (f64)megabyte;
      f64 gigabytes_per_second = bytes_per_second / gigabyte;

      printf("  %.3fmb at %.2fgb/s", megabytes, gigabytes_per_second);
    }

    if (anchor->metrics[Metric_PageFaults]) {
      printf("  %ld page faults", anchor->metrics[Metric_PageFaults]);
    }

    printf("\n");
  }
}


int main(void)
{
  Allocator *allocator = arena_list_allocator_create(KiB(10));
  profileBlockBegin(start, "Start", allocator);

  profileBlockEnd(start);
}
