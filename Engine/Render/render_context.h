#ifndef render_context_h
#define render_context_h

#include "image.h"
#include "base_object.h"
#include "types.h"
#include "array_list.h"
#include "render_rect.h"

extern BaseType RenderContextType;

typedef struct RenderContext {
    BASE_OBJECT;
    const ImageData *target_buffer;
    ArrayList *rendered_rects;
    ArrayList *rect_pool;
    ArrayList *active_rects;
    ArrayList *end_rects;
    ArrayList *merge_rects;
    AffineTransform camera_matrix;
    bool background_enabled;
} RenderContext;

void context_rect_rendered(RenderContext *ctx, int left, int right, int top, int bottom);
void context_background_rendered(RenderContext *ctx);
void context_release_render_rect(RenderContext *ctx, RenderRect *rect);
void clean_union_of_rendered_rects(ArrayList *rendered_rects, ArrayList *result);
void context_clean_union_of_rendered_rects(RenderContext *ctx, ArrayList *rendered_rects, ArrayList *result);

#endif /* render_context_h */
