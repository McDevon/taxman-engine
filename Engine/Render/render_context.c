#include "render_context.h"
#include "render_square.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "engine_log.h"
#include <string.h>

void render_context_destroy(void *value)
{
    RenderContext *self = (RenderContext *)value;
    if (self->rendered_squares) {
        destroy(self->rendered_squares);
    }
    if (self->square_pool) {
        destroy(self->square_pool);
    }
}

char *render_context_describe(void *value)
{
    return platform_strdup("[]");
}

void context_square_rendered(RenderContext *self, int left, int right, int top, int bottom)
{
    if (!self->background_enabled) {
        return;
    }
    
    Square *sq = NULL;
    size_t pool_count = list_count(self->square_pool);
    if (pool_count) {
        sq = list_drop_index(self->square_pool, pool_count - 1);
        sq->left = left;
        sq->right = right;
        sq->top = top;
        sq->bottom = bottom;
    } else {
        sq = square_create(left, right, top, bottom);
    }
    list_add(self->rendered_squares, sq);
}

void context_clean_union_of_rendered_squares(ArrayList *rendered_squares, ArrayList *result)
{
    if (!rendered_squares || !result) {
        LOG_ERROR("clean_union_of_rendered_squares: received null parameter");
        return;
    }
    
    if (list_count(result) != 0) {
        LOG_ERROR("clean_union_of_rendered_squares: result array not empty");
        return;
    }
    
    size_t count = list_count(rendered_squares);
    
    for (size_t i = 0; i < count; ++i) {
        Square *sq = list_get(rendered_squares, i);
        list_add(result, square_create(sq->left + 1, sq->right - 1, sq->top + 1, sq->bottom - 1));
    }
}

BaseType RenderContextType = { "RenderContext", &render_context_destroy, &render_context_describe };
