#include "render_square.h"
#include "string_builder.h"
#include "platform_adapter.h"

void square_destroy(void *value)
{
}

char *square_describe(void *value)
{
    Square *self = (Square *)value;
    
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "[");
    sb_append_int(sb, self->left);
    sb_append_string(sb, ", ");
    sb_append_int(sb, self->right);
    sb_append_string(sb, ", ");
    sb_append_int(sb, self->top);
    sb_append_string(sb, ", ");
    sb_append_int(sb, self->bottom);
    sb_append_string(sb, " ]");

    char *description = sb_get_string(sb);
    destroy(sb);
    return description;
}

BaseType SquareType = { "Square", &square_destroy, &square_describe };

Square *square_create(int left, int right, int top, int bottom)
{
    Square *square = platform_calloc(1, sizeof(Square));
    square->w_type = &SquareType;
    
    square->left = left;
    square->right = right;
    square->top = top;
    square->bottom = bottom;
    
    return square;
}
