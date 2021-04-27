#include "string_builder.h"
#include "platform_adapter.h"
#include "engine_log.h"
#include <string.h>
#include <stdlib.h>

void string_builder_destroy(void *value)
{
    StringBuilder *builder = (StringBuilder *)value;
    platform_free(builder->string);
}

char *string_builder_describe(void *value)
{
    StringBuilder *builder = (StringBuilder *)value;
    return platform_strdup(builder->string);
}

BaseType StringBuilderType = { "StringBuilder", &string_builder_destroy, &string_builder_describe };

StringBuilder *sb_create()
{
    StringBuilder *builder = platform_calloc(1, sizeof(StringBuilder));
    builder->w_type = &StringBuilderType;
    builder->string = platform_calloc(10, sizeof(char));
    builder->capacity = 10;
    builder->length = 0;
    
    return builder;
}

void sb_clear(StringBuilder *sb)
{
    sb->length = 0;
    sb->string[0] = '\0';
}

int sb_append_string(StringBuilder *sb, const char *string)
{
    size_t len = strlen(string);
    while (sb->length + len >= sb->capacity) {
        char *buffer = sb->string;
        size_t new_capacity = sb->capacity * 2;
        char *new_buffer = platform_realloc(buffer, sizeof(char) * new_capacity);
        if (!new_buffer) { return -1; }
        
        sb->capacity = new_capacity;
        sb->string = new_buffer;
    }
    
    strncpy(sb->string + sb->length, string, len + 1);
    sb->length += len;
    
    return 0;
}

int sb_append_line_break(StringBuilder *sb)
{
    return sb_append_string(sb, "\n");
}

int sb_append_int(StringBuilder *sb, int value)
{
    char string[12];
    snprintf(string, 12, "%d", value);
    return sb_append_string(sb, string);
}

int sb_append_hex(StringBuilder *sb, int value)
{
    char string[12];
    snprintf(string, 12, "%x", value);
    return sb_append_string(sb, string);
}

int sb_append_uint64(StringBuilder *sb, uint64_t value)
{
    char string[22];
    snprintf(string, 22, "%llu", value);
    return sb_append_string(sb, string);
}

int sb_append_float(StringBuilder *sb, Float value, int precision)
{
    char string[22];
    char format[8];
    snprintf(format, 8, "%%.%df", precision);
    snprintf(string, 20, format, value);
    return sb_append_string(sb, string);
}

int sb_append_number_precision(StringBuilder *sb, Number value, int precision)
{
    char *string = nb_to_str(value, precision);
    int ret = sb_append_string(sb, string);
    platform_free(string);
    return ret;
}

int sb_append_number(StringBuilder *sb, Number value)
{
    return sb_append_number_precision(sb, value, 2);
}

int sb_append_vector2d(StringBuilder *sb, Vector2D value)
{
    int ret = sb_append_string(sb, "[ ");
    ret += sb_append_number_precision(sb, value.x, 2);
    ret += sb_append_string(sb, ", ");
    ret += sb_append_number_precision(sb, value.y, 2);
    ret += sb_append_string(sb, " ]");
    return ret;
}

int sb_append_size2d(StringBuilder *sb, Size2D value)
{
    int ret = sb_append_string(sb, "{ ");
    ret += sb_append_number_precision(sb, value.width, 2);
    ret += sb_append_string(sb, ", ");
    ret += sb_append_number_precision(sb, value.height, 2);
    ret += sb_append_string(sb, " }");
    return ret;
}

int sb_append_int_point(StringBuilder *sb, Vector2DInt value)
{
    int ret = sb_append_string(sb, "[ ");
    ret += sb_append_int(sb, value.x);
    ret += sb_append_string(sb, ", ");
    ret += sb_append_int(sb, value.y);
    ret += sb_append_string(sb, " ]");
    return ret;
}

int sb_append_int_size(StringBuilder *sb, Size2DInt value)
{
    int ret = sb_append_string(sb, "{ ");
    ret += sb_append_int(sb, value.width);
    ret += sb_append_string(sb, ", ");
    ret += sb_append_int(sb, value.height);
    ret += sb_append_string(sb, " }");
    return ret;
}

int sb_append_int_rect(StringBuilder *sb, Rect2DInt value)
{
    int ret = sb_append_string(sb, "( [ ");
    ret += sb_append_int(sb, value.origin.x);
    ret += sb_append_string(sb, ", ");
    ret += sb_append_int(sb, value.origin.y);
    ret += sb_append_string(sb, " ] {");
    ret += sb_append_int(sb, value.size.width);
    ret += sb_append_string(sb, ", ");
    ret += sb_append_int(sb, value.size.height);
    ret += sb_append_string(sb, " } )");
    return ret;
}

char *sb_get_string(StringBuilder *builder)
{
    return platform_strdup(builder->string);
}

void sb_debug_log_to_console(StringBuilder *sb)
{
    LOG("%s", sb->string);
}

void sb_log_to_console(StringBuilder *sb)
{
    platform_print(sb->string);
}
