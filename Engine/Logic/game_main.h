#ifndef game_main_h
#define game_main_h

#include "types.h"

void game_init(void *first_scene);
void game_step(Number delta_time_millis, Controls controls);

#endif /* game_main_h */
