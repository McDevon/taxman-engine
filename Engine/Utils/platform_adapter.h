#ifndef platform_adapter_h
#define platform_adapter_h

#include <stdlib.h>
#include "stdbool.h"
#include "game_main.h"
#include "platform_types.h" // platform_time_t from platform implementation

typedef void (load_image_data_callback_t)(const char *, const uint32_t, const uint32_t, const bool, const uint8_t *, void *);
typedef void (load_text_data_callback_t)(const char *, const char *, void *);

void platform_display_set_image(uint8_t *buffer, ScreenRenderOptions *render_options);
void platform_load_image(const char *file_path, load_image_data_callback_t *callback, void *context);
void platform_read_text_file(const char *file_path, load_text_data_callback_t *callback, void *context);

void *platform_malloc(size_t size);
void *platform_calloc(size_t count, size_t size);
void *platform_realloc(void *ptr, size_t size);
char *platform_strdup(const char *str);
char *platform_strndup(const char *str, size_t size);
void platform_free(void *ptr);

platform_time_t platform_current_time(void);
float platform_time_to_seconds(platform_time_t);

void platform_print(const char *text);

#endif /* platform_adapter_h */
