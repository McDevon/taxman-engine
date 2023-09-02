#ifndef game_main_h
#define game_main_h

#include "types.h"
#include "render_context.h"

typedef struct ScreenRenderOptions {
    ImageData *screen_dither;
    Size2DInt source_size;
    Vector2DInt source_offset;
    bool invert;
} ScreenRenderOptions;

void game_init(void *first_scene);
void game_step(Number delta_time_millis, Controls controls);

void reset_screen_options(void);
void set_screen_dither(ImageData * screen_dither);
void set_screen_invert(bool invert);
void set_screen_source_buffer(ImageData *screen_buffer);
void set_screen_source_offset(Vector2DInt source_offset);

RenderContext *get_main_render_context(void);

#endif /* game_main_h */
