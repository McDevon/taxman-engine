#ifndef platform_adapter_h
#define platform_adapter_h

typedef uint32_t platform_time_t;

void platform_display_set_image(uint8_t *buffer);
void platform_load_image(const char *file_path, uint32_t *width, uint32_t *height, uint8_t *alpha, uint8_t **buffer);
void platform_destroy_image(void *ptr);
char *platform_read_text_file(const char *file_path);
void platform_close_text_file(char *file_data);

void *platform_malloc(size_t size);
void *platform_calloc(size_t count, size_t size);
void *platform_realloc(void *ptr, size_t size);
char *platform_strdup(const char *str);
char *platform_strndup(const char *str, size_t size);
void platform_free(void *ptr);

platform_time_t platform_current_time(void);
float platform_time_to_seconds(platform_time_t);

#endif /* platform_adapter_h */
