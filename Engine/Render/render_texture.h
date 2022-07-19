#ifndef render_texture_h
#define render_texture_h

#include "base_object.h"
#include "render_context.h"
#include "game_object.h"
#include "image.h"
#include "types.h"

typedef struct RenderTexture {
    BASE_OBJECT;
    ImageData *image_data;
    Image *image;
    RenderContext *render_context;
} RenderTexture;

RenderTexture *render_texture_create(Size2DInt size, int32_t channels);

void render_texture_render_go(RenderTexture *render_texture, GameObject *object);

#endif /* render_texture_h */
