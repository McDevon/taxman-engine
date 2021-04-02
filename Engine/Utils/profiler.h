#ifndef profiler_h
#define profiler_h

#define ENABLE_PROFILER
//#undef ENABLE_PROFILER

void profiler_init(void);
void profiler_finish(void);

void profiler_start_segment(const char *segment_name);
void profiler_end_segment(void);

char *profiler_get_data(void);

void profiler_toggle(void);

#endif /* profiler_h */
