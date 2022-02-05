#ifndef profiler_h
#define profiler_h

#define ENABLE_PROFILER
#undef ENABLE_PROFILER

void profiler_start_segment(const char *segment_name);
void profiler_end_segment(void);

void profiler_schedule_start(void);
void profiler_schedule_end(void);
void profiler_schedule_toggle(void);

#endif /* profiler_h */
