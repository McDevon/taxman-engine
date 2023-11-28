#include "audio_player.h"
#include "platform_adapter.h"
#include "hash_table.h"
#include "hash_table_private.h"
#include "engine_log.h"

static HashTableEntry *audio_object_table_entry[HASHSIZE];
static HashTable audio_object_table = { { { &HashTableType } }, audio_object_table_entry, NULL };

void audio_file_loaded(const char *file_name, void *audio_object, void *context)
{
    bool success = audio_object != NULL;
    if (success) {
        hashtable_put(&audio_object_table, file_name, audio_object);
    } else {
        LOG_ERROR("Failed to load audio file %s", file_name);
    }
    ResourceCallbackContainer *container = (ResourceCallbackContainer *)context;
    container->resource_callback(file_name, success, container->context);
    platform_free(container);
}

void audio_load_file(const char *file_name, resource_callback_t resource_callback, void *context)
{
    ResourceCallbackContainer *container = platform_calloc(1, sizeof(ResourceCallbackContainer));
    container->context = context;
    container->resource_callback = resource_callback;
    platform_load_audio_file(file_name, &audio_file_loaded, container);
}

void audio_play_file(const char *file_name)
{
    void *audio_object = hashtable_get(&audio_object_table, file_name);
    if (!audio_object) {
        LOG_ERROR("Cannot play audio file, file not loaded: %s", file_name);
        return;
    }
    platform_play_audio_object(audio_object);
}

void audio_free_file(const char *file_name)
{
    void *audio_object = hashtable_get(&audio_object_table, file_name);
    if (!audio_object) {
        LOG_ERROR("Cannot free audio file, file not loaded: %s", file_name);
        return;
    }
    platform_free_audio_object(audio_object);
}

