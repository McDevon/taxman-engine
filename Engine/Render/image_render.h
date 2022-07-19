#ifndef render_dither_h
#define render_dither_h

#include <stdio.h>
#include "image.h"
#include "render_context.h"

void image_render(ImageData *target_buffer, const Image *image, const Vector2DInt position, const uint8_t flip_flags_xy, const bool invert);

void context_render_rect_image(RenderContext *context, const Image *image, const Vector2DInt position, const uint8_t flip_flags_xy, const bool invert);
void context_render_rect_dither(RenderContext *context, const Image *image, const Image *dither_texture, const Vector2DInt position, const Vector2DInt offset, const int flip_flags_xy);

void context_clear_white(RenderContext *context);
void context_clear_black(RenderContext *context);

void context_render(RenderContext *context, const Image *image, const uint8_t flip_flags_xy, const bool invert);

#endif /* render_dither_h */
