#ifndef string_utils_h
#define string_utils_h

#include <stdbool.h>
#include <string.h>

char * copy_str_without_path_extension(const char *input);

#define str_equals(a, b) (strcmp(a, b) == 0)

#define str_starts_with(str, start) \
({ __typeof__ (start) __start = (start); \
  (strncmp(str, __start, strlen(__start)) == 0); })

bool str_ends_with(const char *str, const char *end);


#endif /* string_utils_h */
