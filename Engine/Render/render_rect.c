#include "render_rect.h"
#include "string_builder.h"
#include "platform_adapter.h"

void square_destroy(void *value)
{
}

char *square_describe(void *value)
{
    RenderRect *self = (RenderRect *)value;
    
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

RenderRect *rrect_create(int left, int right, int top, int bottom)
{
    RenderRect *square = platform_calloc(1, sizeof(RenderRect));
    square->w_type = &SquareType;
    
    square->left = left;
    square->right = right;
    square->top = top;
    square->bottom = bottom;
    
    return square;
}

RenderRect *rrect_copy(RenderRect *sq)
{
    return rrect_create(sq->left, sq->right, sq->top, sq->bottom);
}
