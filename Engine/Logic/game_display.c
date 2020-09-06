#include "game_display.h"
#include "platform_adapter.h"

void update_buffer(uint8_t *buffer)
{
    platform_display_set_image(buffer);
}
