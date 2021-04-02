#include "profiler.h"
#include "hash_table.h"
#include "hash_table_private.h"
#include "array_list.h"
#include "base_object.h"
#include "string_builder.h"
#include "engine_log.h"
#include "platform_adapter.h"

HashTable *profiler_hash_table;
ArrayList *profiler_stack;
platform_time_t profiler_start_time;

#define PE_CONTENTS \
    BASE_OBJECT; \
    char *key; \
    platform_time_t start_time; \
    platform_time_t total_time

typedef struct ProfilerEntry {
    PE_CONTENTS;
} ProfilerEntry;

char *profiler_entry_describe(void *obj)
{
    //ProfilerEntry *entry = (ProfilerEntry*)obj;
    return platform_strdup("Profiler entry");
}

void profiler_entry_destroy(void *obj)
{
    //ProfilerEntry *entry = (ProfilerEntry*)obj;
}

static BaseType ProfilerEntryType = { "ProfilerEntry", &profiler_entry_destroy, &profiler_entry_describe };

ProfilerEntry *profiler_entry_create()
{
    ProfilerEntry *entry = platform_calloc(1, sizeof(ProfilerEntry));

    entry->start_time = 0;
    entry->total_time = 0;
    
    entry->w_type = &ProfilerEntryType;
    
    return entry;
}

void profiler_init()
{
    profiler_hash_table = hashtable_create();
    profiler_stack = list_create_with_weak_references();
    profiler_start_time = platform_current_time();
}

void profiler_finish()
{
    destroy(profiler_hash_table);
    destroy(profiler_stack);
    profiler_start_time = 0;
}

void profiler_start_segment(const char *segment_name)
{
    if (!profiler_start_time) {
        return;
    }
    ProfilerEntry *entry = hashtable_get(profiler_hash_table, segment_name);
    if (!entry) {
        entry = profiler_entry_create();
        entry->key = (char *)segment_name;
        hashtable_put(profiler_hash_table, segment_name, entry);
    }
    entry->start_time = platform_current_time();
    list_add(profiler_stack, entry);
}

void profiler_end_segment()
{
    if (!profiler_start_time) {
        return;
    }
    ProfilerEntry *entry = list_drop_index(profiler_stack, list_count(profiler_stack) - 1);
    entry->total_time += platform_current_time() - entry->start_time;
}

char *profiler_get_data()
{
    if (list_count(profiler_stack)) {
        LOG("#PROFILER error: stack not empty");
        return NULL;
    }
    
    platform_time_t total_time = 0;
    
    for (int i = 0; i < HASHSIZE; ++i) {
        if (profiler_hash_table->entries[i] == NULL) {
            continue;
        }
        HashTableEntry *table_entry = profiler_hash_table->entries[i];
        while (table_entry) {
            ProfilerEntry *profiler_entry = (ProfilerEntry *)table_entry->value;
            total_time += profiler_entry->total_time;
            list_add(profiler_stack, profiler_entry);
            table_entry = table_entry->next;
        }
    }
    
    platform_time_t profiler_end_time = platform_current_time();
    platform_time_t profiling_time = profiler_end_time - profiler_start_time;

    float total_seconds = platform_time_to_seconds(profiling_time);
    float measured_seconds = platform_time_to_seconds(total_time);

    StringBuilder *sb = sb_create();
    sb_append_string(sb, "PROFILING RESULTS");
    sb_append_line_break(sb);
    sb_append_string(sb, "Measured time makes up ");
    sb_append_float(sb, measured_seconds / total_seconds * 100, 2);
    sb_append_string(sb, "% of running time ( ");
    sb_append_float(sb, total_seconds, 2);
    sb_append_string(sb, "s / ");
    sb_append_float(sb, measured_seconds, 2);
    sb_append_string(sb, "s )");
    sb_append_line_break(sb);
    
    size_t count = list_count(profiler_stack);
    for (size_t i = 0; i < count; ++i) {
        ProfilerEntry *profiler_entry = (ProfilerEntry *)list_get(profiler_stack, i);
        
        float entry_seconds = platform_time_to_seconds(profiler_entry->total_time);
        
        sb_append_string(sb, profiler_entry->key);
        sb_append_string(sb, ": ");
        sb_append_float(sb, entry_seconds, 2);
        sb_append_string(sb, "s ");
        sb_append_float(sb, entry_seconds / measured_seconds * 100, 2);
        sb_append_string(sb, "%");
        sb_append_line_break(sb);
    }
    
    char *output = sb_get_string(sb);
    destroy(sb);
    
    return output;
}

void profiler_toggle()
{
    if (profiler_start_time) {
        char *data = profiler_get_data();
        printf("%s", data);
        free(data);
        
        profiler_finish();
        LOG("#PROFILER Finished");
    } else {
        LOG("#PROFILER Started");
        profiler_init();
    }
}
