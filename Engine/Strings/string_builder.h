#ifndef string_builder_h
#define string_builder_h

#include "base_object.h"
#include "types.h"

typedef struct StringBuilder {
    BASE_OBJECT;
    char *string;
    size_t length;
    size_t capacity;
} StringBuilder;

StringBuilder *sb_create(void);

char *sb_get_string(StringBuilder *sb);

void sb_clear(StringBuilder *sb);

int sb_append_string(StringBuilder *sb, const char *string);
int sb_append_line_break(StringBuilder *sb);
int sb_append_int(StringBuilder *sb, int value);
int sb_append_hex(StringBuilder *sb, int value);
int sb_append_uint64(StringBuilder *sb, uint64_t value);
int sb_append_float(StringBuilder *sb, Float value, int precision);
int sb_append_number(StringBuilder *sb, Number value);
int sb_append_number_precision(StringBuilder *sb, Number value, int precision);

int sb_append_vector2d(StringBuilder *sb, Vector2D value);
int sb_append_size2d(StringBuilder *sb, Size2D value);
int sb_append_int_point(StringBuilder *sb, Vector2DInt value);
int sb_append_int_size(StringBuilder *sb, Size2DInt value);
int sb_append_int_rect(StringBuilder *sb, Rect2DInt value);

#endif /* string_builder_h */
