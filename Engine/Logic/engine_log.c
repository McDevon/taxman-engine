#include <stdio.h>
#include <stdarg.h>
#include "engine_log.h"
#include "string_builder.h"
#include "string_builder_private.h"
#include "platform_adapter.h"

void log_print(const char *format, ...)
{
    StringBuilder *sb = sb_create();

    sb_append_string(sb, "[debug] ");
    
    va_list arg;
    int done;
    va_start (arg, format);
    done = sb_vfprintf(sb, format, arg);
    va_end (arg);
    
    sb_append_line_break(sb);
    
    char *string = sb_get_string(sb);
    destroy(sb);
    
    platform_print(string);
    platform_free(string);
}

void log_print_error(const char *format, ...)
{
    StringBuilder *sb = sb_create();

    sb_append_string(sb, "[error] ");
    
    va_list arg;
    int done;
    va_start (arg, format);
    done = sb_vfprintf(sb, format, arg);
    va_end (arg);
    
    sb_append_line_break(sb);
    
    char *string = sb_get_string(sb);
    destroy(sb);
    
    platform_print(string);
    platform_free(string);
}
