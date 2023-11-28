#ifndef audio_player_h
#define audio_player_h

#include "types.h"

void audio_load_file(const char *file_name, resource_callback_t resource_callback, void *context);
void audio_play_file(const char *file_name);
void audio_stop_file(const char *file_name);
void audio_free_file(const char *file_name);

#endif /* audio_player_h */
