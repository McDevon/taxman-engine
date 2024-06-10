#include "game_display.h"
#include "platform_adapter.h"

void update_buffer(uint8_t *buffer, ScreenRenderOptions *render_options)
{
    if (render_options->custom_screen_update) {
        render_options->custom_screen_update(buffer, render_options);
    } else {
        platform_display_set_image(buffer, render_options);
    }
}
