#include <stdlib.h>
#include <string.h>
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
