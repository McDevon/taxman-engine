#include "debug_draw.h"
#include "transforms.h"
#include "string_builder.h"
#include "platform_adapter.h"
#include "utils.h"

typedef struct DebugDraw {
    GAME_OBJECT;
    ArrayList *lines;
} DebugDraw;

typedef struct Line {
    BASE_OBJECT;
    Vector2DInt start;
    Vector2DInt end;
} Line;

void line_destroy(void *value)
{
}

char *line_describe(void *value)
{
    Line *self = (Line *)value;
    
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "{ ");
    sb_append_int(sb, self->start.x);
    sb_append_string(sb, ", ");
    sb_append_int(sb, self->start.y);
    sb_append_string(sb, " } -> { ");
    sb_append_int(sb, self->end.x);
    sb_append_string(sb, ", ");
    sb_append_int(sb, self->end.y);
    sb_append_string(sb, " }");

    char *description = sb_get_string(sb);
    destroy(sb);
    return description;
}

BaseType LineType = { "Line", line_destroy, line_describe};

Line *line_create(Vector2DInt start, Vector2DInt end)
{
    Line *line = platform_calloc(1, sizeof(Line));
    line->w_type = &LineType;
    
    line->start = start;
    line->end = end;
    
    return line;
}

void debugdraw_line(DebugDraw *self, Vector2D start, Vector2D end)
{
    list_add(self->lines, line_create((Vector2DInt){ (int32_t)start.x, (int32_t)start.y }, (Vector2DInt){ (int32_t)end.x, (int32_t)end.y }));
}

void debugdraw_clear(DebugDraw *self)
{
    list_clear(self->lines);
}

void debugdraw_render(GameObject *obj, RenderContext *ctx)
{
    DebugDraw *self = (DebugDraw *)obj;
    
    const int32_t target_channels = image_data_channel_count(ctx->w_target_buffer);
    const int32_t target_width = ctx->w_target_buffer->size.width;
    const int32_t target_height = ctx->w_target_buffer->size.height;
    ImageBuffer *target = ctx->w_target_buffer->buffer;

    const size_t count = list_count(self->lines);
    for (size_t i = 0; i < count; ++i) {
        Line *line = list_get(self->lines, i);
        
        int32_t x0 = line->start.x;
        int32_t y0 = line->start.y;
        const int32_t x1 = line->end.x;
        const int32_t y1 = line->end.y;
        const int32_t dx = abs(x1 - x0);
        const int32_t sx = x0 < x1 ? 1 : -1;
        const int32_t dy = -abs(y1 - y0);
        const int32_t sy = y0 < y1 ? 1 : -1;
        int32_t err = dx + dy;
        
        while (true) {
            if (x0 >= 0 && x0 < target_width && y0 >= 0 && y0 < target_height) {
                int32_t i_index = (x0 + y0 * target_width) * target_channels;
                target[i_index] = 0;                
            }
            
            if (x0 == x1 && y0 == y1) break;

            const int32_t e2 = 2 * err;
            if (e2 >= dy) {
                err += dy;
                x0 += sx;
            }
            if (e2 <= dx) {
                err += dx;
                y0 += sy;
            }
        }
    }
}

void debugdraw_destroy(void *obj)
{
    DebugDraw *self = (DebugDraw *)obj;
    destroy(self->lines);
    go_destroy(obj);
}

char *debugdraw_describe(void *DebugDraw)
{
    return go_describe(DebugDraw);
}

GameObjectType DebugDrawType = {
    { { "DebugDraw", &debugdraw_destroy, &debugdraw_describe } },
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &debugdraw_render
};

DebugDraw *debugdraw_create()
{
    GameObject *go = go_alloc(sizeof(DebugDraw));
    DebugDraw *debugDraw = (DebugDraw *)go;
    debugDraw->w_type = &DebugDrawType;
    
    debugDraw->lines = list_create();

    return debugDraw;
}

