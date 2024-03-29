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
void render_texture_resize(RenderTexture *self, Size2DInt size);

RenderTexture *render_texture_create_with_rotated(Image *original_image, Float angle);
RenderTexture *render_texture_create_with_rotated_anchored(Image *original_image, Float angle, Vector2D *in_out_anchor);

#endif /* render_texture_h */
