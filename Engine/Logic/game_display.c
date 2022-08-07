#include "game_display.h"
#include "platform_adapter.h"

void update_buffer(uint8_t *buffer, ScreenRenderOptions *render_options)
{
    platform_display_set_image(buffer, render_options);
}
