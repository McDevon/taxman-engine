#include <stdlib.h>
#include "string_utils.h"
#include "platform_adapter.h"

char * copy_str_without_path_extension(const char *input) {
    char *separator = strchr(input, '.');
    if (separator == NULL) {
        return platform_strdup(input);
    } else {
        const long len = (long)separator - (long)input;
        char *ret = platform_calloc(len + 1, sizeof(char));
        for (long i = 0; i < len; ++i) {
            ret[i] = input[i];
        }
        ret[len] = '\0';
        return ret;
    }
}

bool str_ends_with(const char *str, const char *end)
{
    if (!str || !end) {
        return false;
    }
    size_t len_str = strlen(str);
    size_t len_end = strlen(end);
    if (len_end > len_str) {
        return false;
    }
    return strncmp(str + len_str - len_end, end, len_end) == 0;
}
