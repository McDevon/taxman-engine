#ifndef game_main_h
#define game_main_h

#include "types.h"
#include "render_context.h"

struct ScreenRenderOptions;

typedef void (update_buffer_t)(uint8_t *buffer, struct ScreenRenderOptions *render_options);

typedef struct ScreenRenderOptions {
    ImageData *screen_dither;
    update_buffer_t *custom_screen_update;
    Size2DInt source_size;
    Vector2DInt source_offset;
    bool invert;
} ScreenRenderOptions;

void game_init(void *first_scene);
void game_step(Float delta_time_seconds, Float crank, ButtonControls buttons);

void reset_screen_options(void);
void set_screen_dither(ImageData * screen_dither);
void set_screen_invert(bool invert);
void set_screen_source_buffer(ImageData *screen_buffer);
void set_screen_source_offset(Vector2DInt source_offset);
void set_custom_screen_update(update_buffer_t *custom_update_function);

RenderContext *get_main_render_context(void);

#endif /* game_main_h */
