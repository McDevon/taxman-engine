#ifndef platform_adapter_h
#define platform_adapter_h

#include <stdlib.h>
#include "stdbool.h"
#include "game_main.h"
#include "platform_types.h" // platform_time_t from platform implementation

typedef void (load_image_data_callback_t)(const char *, const uint32_t, const uint32_t, const bool, const uint8_t *, void *);
typedef void (load_text_data_callback_t)(const char *, const char *, const size_t, void *);
typedef void (load_raw_data_callback_t)(const char *, const uint8_t *, const size_t, void *);
typedef void (file_exists_callback_t)(const char *, bool, void *);
typedef void (write_success_callback_t)(const char *, bool, void *);

void platform_display_set_image(uint8_t *buffer, ScreenRenderOptions *render_options);
void platform_load_image(const char *file_path, load_image_data_callback_t *callback, void *context);
void platform_read_text_file(const char *file_path, const bool user_file, load_text_data_callback_t *callback, void *context);
void platform_write_text_file(const char *file_path, const char *text, size_t length, write_success_callback_t *callback, void *context);
void platform_read_data_file(const char *file_path, const bool user_file, load_raw_data_callback_t *callback, void *context);
void platform_write_data_file(const char *file_path, const uint8_t *data, size_t length, write_success_callback_t *callback, void *context);
void platform_file_exists(const char *file_path, const bool user_file, file_exists_callback_t *callback, void *context);

void *platform_malloc(size_t size);
void *platform_calloc(size_t count, size_t size);
void *platform_realloc(void *ptr, size_t size);
char *platform_strdup(const char *str);
char *platform_strndup(const char *str, size_t size);
void platform_free(void *ptr);

platform_time_t platform_current_time(void);
float platform_time_to_seconds(platform_time_t);

typedef void (audio_object_callback_t)(const char *, void *, void *);

/**
 Returns a pointer to an audio object that can be used to play the loaded audio file.
 Use for short audio effect, not for music.
 */
void platform_load_audio_file(const char *file_name, audio_object_callback_t *callback, void *context);
void platform_play_audio_object(void *audio_object);
void platform_stop_audio_object(void *audio_object);
void platform_free_audio_object(void *audio_object);

void platform_print(const char *text);

#endif /* platform_adapter_h */
