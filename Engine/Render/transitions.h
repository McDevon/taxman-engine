#ifndef transitions_h
#define transitions_h

#include <stdio.h>
#include "scene_manager.h"

void transition_swipe_ltr_step(SceneManager *scene_manager, RenderContext *ctx);
void transition_fade_black_step(SceneManager *scene_manager, RenderContext *ctx);

#endif /* transitions_h */
