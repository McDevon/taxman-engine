#ifndef render_context_h
#define render_context_h

#include "image.h"
#include "base_object.h"
#include "types.h"

extern BaseType RenderContextType;

typedef struct RenderContext {
    BASE_OBJECT;
    const ImageData *target_buffer;
    ImageData *dither_buffer;
    AffineTransform camera_matrix;
} RenderContext;

#endif /* render_context_h */
