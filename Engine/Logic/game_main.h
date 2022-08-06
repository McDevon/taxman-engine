#ifndef game_main_h
#define game_main_h

#include "types.h"
#include "render_context.h"

void game_init(void *first_scene);
void game_step(Number delta_time_millis, Controls controls);

void set_screen_dither(ImageData * screen_dither);

RenderContext *get_main_render_context(void);

#endif /* game_main_h */
