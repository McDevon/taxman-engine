#ifndef render_context_h
#define render_context_h

#include "image.h"
#include "base_object.h"
#include "types.h"
#include "array_list.h"

extern BaseType RenderContextType;

typedef struct RenderContext {
    BASE_OBJECT;
    const ImageData *target_buffer;
    ArrayList *rendered_squares;
    ArrayList *square_pool;
    AffineTransform camera_matrix;
    bool background_enabled;
} RenderContext;

void context_square_rendered(RenderContext *ctx, int left, int right, int top, int bottom);
void context_clean_union_of_rendered_squares(ArrayList *rendered_squares, ArrayList *result);

#endif /* render_context_h */
