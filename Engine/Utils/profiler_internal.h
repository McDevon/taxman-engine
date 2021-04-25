#ifndef profiler_internal_h
#define profiler_internal_h

typedef enum {
    prof_none,
    prof_start,
    prof_end,
    prof_toggle
} ProfilerScheduleState;

void profiler_init(void);
void profiler_finish(void);

char *profiler_get_data(void);

void profiler_toggle(void);

ProfilerScheduleState profiler_schedule(void);

#endif /* profiler_internal_h */
