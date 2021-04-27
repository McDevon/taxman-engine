#include "profiler.h"
#include "profiler_internal.h"
#include "hash_table.h"
#include "hash_table_private.h"
#include "array_list.h"
#include "base_object.h"
#include "string_builder.h"
#include "engine_log.h"
#include "platform_adapter.h"

struct ProfilerEntry;

#define PE_CONTENTS \
    BASE_OBJECT; \
    HashTable *subentries; \
    struct ProfilerEntry *w_parent; \
    char *key; \
    platform_time_t start_time; \
    platform_time_t total_time

typedef struct ProfilerEntry {
    PE_CONTENTS;
} ProfilerEntry;

ProfilerEntry *profiler_root_entry;
ProfilerEntry *w_profiler_top_entry;
ProfilerScheduleState _profiler_schedule_state = prof_none;

char *profiler_entry_describe(void *obj)
{
    //ProfilerEntry *entry = (ProfilerEntry*)obj;
    return platform_strdup("Profiler entry");
}

void profiler_entry_destroy(void *obj)
{
    ProfilerEntry *entry = (ProfilerEntry*)obj;
    destroy(entry->subentries);
}

static BaseType ProfilerEntryType = { "ProfilerEntry", &profiler_entry_destroy, &profiler_entry_describe };

ProfilerEntry *profiler_entry_create()
{
    ProfilerEntry *entry = platform_calloc(1, sizeof(ProfilerEntry));

    entry->start_time = 0;
    entry->total_time = 0;
    
    entry->w_type = &ProfilerEntryType;
    entry->w_parent = NULL;
    
    entry->subentries = hashtable_create();
    
    return entry;
}

void profiler_init()
{
    profiler_root_entry = profiler_entry_create();
    w_profiler_top_entry = profiler_root_entry;
    profiler_root_entry->key = "Total";
    
    profiler_root_entry->start_time = platform_current_time();
    
    _profiler_schedule_state = prof_none;
}

void profiler_finish()
{
    destroy(profiler_root_entry);
    profiler_root_entry = NULL;

    _profiler_schedule_state = prof_none;
}

void profiler_start_segment(const char *segment_name)
{
    if (!profiler_root_entry) {
        return;
    }
    ProfilerEntry *entry = hashtable_get(w_profiler_top_entry->subentries, segment_name);
    if (!entry) {
        entry = profiler_entry_create();
        entry->key = (char *)segment_name;
        hashtable_put(w_profiler_top_entry->subentries, segment_name, entry);
        entry->w_parent = w_profiler_top_entry;
    }
    entry->start_time = platform_current_time();
    w_profiler_top_entry = entry;
}

void profiler_end_segment()
{
    if (!profiler_root_entry) {
        return;
    }
    w_profiler_top_entry->total_time += platform_current_time() - w_profiler_top_entry->start_time;
    w_profiler_top_entry = w_profiler_top_entry->w_parent;
}

void profiler_indent(StringBuilder *sb, int depth)
{
    for (int i = 0; i < depth; ++i) {
        sb_append_string(sb, "  ");
    }
}

int profiler_compare_entry_time(const void *a, const void *b)
{
    ProfilerEntry *entry_a = *(ProfilerEntry **)a;
    ProfilerEntry *entry_b = *(ProfilerEntry **)b;
    
    if (entry_a->total_time > entry_b->total_time) {
        return list_sorted_ascending;
    } else if (entry_a->total_time < entry_b->total_time) {
        return list_sorted_descending;
    } else {
        return list_sorted_same;
    }
}

void profiler_add_entry_data(ProfilerEntry *entry, StringBuilder *sb, int depth)
{
    if (!hashtable_count(entry->subentries)) {
        return;
    }

    platform_time_t measured_time = 0;
    ArrayList *profiler_stack = list_create_with_weak_references();
    
    for (int i = 0; i < HASHSIZE; ++i) {
        if (entry->subentries->entries[i] == NULL) {
            continue;
        }
        HashTableEntry *table_entry = entry->subentries->entries[i];
        while (table_entry) {
            ProfilerEntry *profiler_entry = (ProfilerEntry *)table_entry->value;
            measured_time += profiler_entry->total_time;
            list_add(profiler_stack, profiler_entry);
            table_entry = table_entry->next;
        }
    }
    
    list_sort(profiler_stack, &profiler_compare_entry_time);
    
    float total_seconds = platform_time_to_seconds(entry->total_time);
    float measured_seconds = platform_time_to_seconds(measured_time);
    
    profiler_indent(sb, depth);
    sb_append_string(sb, "Measured time makes up ");
    sb_append_float(sb, measured_seconds / total_seconds * 100, 2);
    sb_append_string(sb, "% of running time ( ");
    sb_append_float(sb, measured_seconds, 4);
    sb_append_string(sb, "s / ");
    sb_append_float(sb, total_seconds, 4);
    sb_append_string(sb, "s )");
    sb_append_line_break(sb);
    
    size_t count = list_count(profiler_stack);
    for (size_t i = 0; i < count; ++i) {
        ProfilerEntry *subentry = (ProfilerEntry *)list_get(profiler_stack, i);
        
        float entry_seconds = platform_time_to_seconds(subentry->total_time);
        
        profiler_indent(sb, depth);
        sb_append_string(sb, subentry->key);
        sb_append_string(sb, ": ");
        sb_append_float(sb, entry_seconds, 4);
        sb_append_string(sb, "s ");
        sb_append_float(sb, entry_seconds / measured_seconds * 100, 2);
        sb_append_string(sb, "%");
        sb_append_line_break(sb);
        
        profiler_add_entry_data(subentry, sb, depth + 1);
    }
    
    destroy(profiler_stack);
}

char *profiler_get_data()
{
    if (w_profiler_top_entry->w_parent) {
        LOG("#PROFILER error: stack not empty");
        return NULL;
    }
    
    w_profiler_top_entry->total_time += platform_current_time() - w_profiler_top_entry->start_time;

    StringBuilder *sb = sb_create();
    sb_append_string(sb, "PROFILING RESULTS");
    sb_append_line_break(sb);
    
    profiler_add_entry_data(profiler_root_entry, sb, 0);
    
    char *output = sb_get_string(sb);
    destroy(sb);
    
    return output;
}

void profiler_toggle()
{
    if (profiler_root_entry) {
        char *data = profiler_get_data();
        platform_print(data);
        free(data);
        
        profiler_finish();
        LOG("#PROFILER Finished");
    } else {
        LOG("#PROFILER Started");
        profiler_init();
    }

    _profiler_schedule_state = prof_none;
}

ProfilerScheduleState profiler_schedule(void)
{
    return _profiler_schedule_state;
}

void profiler_schedule_start(void)
{
    _profiler_schedule_state = prof_start;
}

void profiler_schedule_end(void)
{
    _profiler_schedule_state = prof_end;
}

void profiler_schedule_toggle(void)
{
    _profiler_schedule_state = prof_toggle;
}
