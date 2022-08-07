#ifndef game_main_h
#define game_main_h

#include "types.h"
#include "render_context.h"

typedef struct ScreenRenderOptions {
    ImageData *screen_dither;
    bool invert;
} ScreenRenderOptions;

void game_init(void *first_scene);
void game_step(Number delta_time_millis, Controls controls);

void set_screen_dither(ImageData * screen_dither);
void set_screen_invert(bool invert);

RenderContext *get_main_render_context(void);

#endif /* game_main_h */
