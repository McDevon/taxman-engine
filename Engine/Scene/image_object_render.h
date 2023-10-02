#ifndef image_object_render_h
#define image_object_render_h

#include "image.h"
#include "image_render.h"
#include "game_object.h"

void image_object_render(Image *image, GameObject *obj, RenderOptions render_options, DrawMode draw_mode, RenderContext *ctx);

#endif /* image_object_render_h */
