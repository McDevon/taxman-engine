#ifndef render_dither_h
#define render_dither_h

#include <stdio.h>
#include "image.h"
#include "render_context.h"

typedef struct RenderOptions {
    uint8_t flip_x : 1;
    uint8_t flip_y : 1;
    uint8_t invert : 1;
} RenderOptions;

#define render_options_make(flip_x_input, flip_y_input, invert_input) \
({ RenderOptions o; \
    o.flip_x = (flip_x_input); \
    o.flip_y = (flip_y_input); \
    o.invert = (invert_input); \
    o; \
})

void context_render_rect_image(RenderContext *context, const Image *image, const Vector2DInt position, const RenderOptions render_options);
void context_render_scale_image(RenderContext *context, const Image *image, const Vector2DInt position, const Vector2D scale, const RenderOptions render_options);
void context_render_rotate_image(RenderContext *context, const Image *image, const Vector2DInt position, const Number angle, const Vector2D anchor_in_image_coordinates, const RenderOptions render_options);
void context_render_rect_dither(RenderContext *context, const Image *image, const Image *dither_texture, const Vector2DInt position, const Vector2DInt offset, const int flip_flags_xy_image, const int flip_flags_xy_dither);
void context_render_rect_dither_threshold(RenderContext *context, const uint8_t threshold, const Image *dither_image, const Vector2DInt position, const int flip_flags_xy);

void context_fill(RenderContext *context, uint8_t color);
void context_clear_white(RenderContext *context);
void context_clear_black(RenderContext *context);
void context_clear_transparent_white(RenderContext *context);
void context_fill_rect(RenderContext *context, RenderRect *rect, uint8_t color);

void context_render(RenderContext *context, const Image *image, const RenderOptions render_options);

#endif /* render_dither_h */
