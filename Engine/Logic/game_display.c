#include "game_display.h"
#include "platform_adapter.h"

void update_buffer(uint8_t *buffer, ImageData *screen_dither)
{
    platform_display_set_image(buffer, screen_dither);
}
