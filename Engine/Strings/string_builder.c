#include "string_builder.h"
#include "platform_adapter.h"
#include "engine_log.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

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

int sb_ensure_increase(StringBuilder *sb, size_t increase)
{
    while (sb->length + increase >= sb->capacity) {
        char *buffer = sb->string;
        size_t new_capacity = sb->capacity * 2;
        char *new_buffer = platform_realloc(buffer, sizeof(char) * new_capacity);
        if (!new_buffer) { return -1; }
        
        sb->capacity = new_capacity;
        sb->string = new_buffer;
    }
    return 0;
}

int sb_append_string(StringBuilder *sb, const char *string)
{
    size_t len = strlen(string);
    int ret = sb_ensure_increase(sb, len);
    if (ret) {
        return ret;
    }
    
    strncpy(sb->string + sb->length, string, len + 1);
    sb->length += len;
    
    return 0;
}

int sb_append_substring(StringBuilder *sb, const char *string, const size_t len)
{
    const size_t used_len = min(len, strlen(string));
    int ret = sb_ensure_increase(sb, used_len);
    if (ret) {
        return ret;
    }
    
    strncpy(sb->string + sb->length, string, used_len + 1);
    sb->length += used_len;
    
    return 0;
}

int sb_append_string_until_char(StringBuilder *sb, const char *string, const char end)
{
    size_t len = strlen(string);
    int ret = sb_ensure_increase(sb, len);
    if (ret) {
        return ret;
    }
    
    char *dst = sb->string + sb->length;
    const char *src = string;
    size_t max = len + 1;
    size_t i = 0;
    while(i++ != max && *src != end && (*dst++ = *src++));
    
    sb->length += i - 1;
    
    return 0;
}

int sb_append_char(StringBuilder *sb, const char ch)
{
    int ret = sb_ensure_increase(sb, 1);
    if (ret) {
        return ret;
    }
    
    sb->string[sb->length] = ch;
    sb->string[sb->length + 1] = '\0';
    sb->length += 1;
    
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

int sb_append_number_precision(StringBuilder *sb, FixNumber value, int precision)
{
    char *string = fn_to_str(value, precision);
    int ret = sb_append_string(sb, string);
    platform_free(string);
    return ret;
}

int sb_append_number(StringBuilder *sb, FixNumber value)
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

int sb_vfprintf(StringBuilder *sb, const char *fmt, va_list arg)
{
    int int_value;
    char char_value;
    char *string_value;
    double double_value;
    char ch;
    
    size_t length = sb->length;
    
    for (size_t offset = 0; (ch = fmt[offset]) != '\0'; ++offset) {
        if (ch == '%' && fmt[offset + 1] != '\0') {
            ++offset;
            switch (fmt[offset]) {
                case '%':
                    sb_append_char(sb, '%');
                    break;
                    
                case 'c':
                    char_value = (char)va_arg(arg, int);
                    sb_append_char(sb, char_value);
                    break;
                    
                case 's':
                    string_value = va_arg(arg, char *);
                    sb_append_string(sb, string_value);
                    break;
                    
                case 'd':
                    int_value = va_arg(arg, int);
                    sb_append_int(sb, int_value);
                    break;
                    
                case 'x':
                    int_value = va_arg(arg, int);
                    sb_append_hex(sb, int_value);
                    break;
                    
                case 'f':
                    double_value = va_arg(arg, double);
                    sb_append_float(sb, (float)double_value, 4);
                    break;
                case '.':
                    double_value = va_arg(arg, double);
                    if (sscanf(fmt + offset, ".%df", &int_value) != 1) {
                        break;
                    }
                    sb_append_float(sb, (float)double_value, int_value);
                    for (;fmt[offset] != 'f'; ++offset);
                    break;
            }
        }
        else {
            sb_append_char(sb, ch);
        }
    }
    return (int)(sb->length - length);
}

int sb_append_format_args(StringBuilder *sb, const char *format, va_list arg)
{
    return sb_vfprintf(sb, format, arg);
}

int sb_append_format(StringBuilder *sb, const char *format, ...)
{
    va_list arg;
    int length;
    
    va_start(arg, format);
    length = sb_append_format_args(sb, format, arg);
    va_end(arg);
    return length;
}

char *sb_string_with_format(const char *format, ...)
{
    va_list arg;
    
    va_start(arg, format);
    StringBuilder *sb = sb_create();
    sb_append_format_args(sb, format, arg);
    va_end(arg);
    
    char *output = sb_get_string(sb);
    destroy(sb);
    
    return output;
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
