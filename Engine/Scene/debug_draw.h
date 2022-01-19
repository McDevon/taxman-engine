#ifndef debug_draw_h
#define debug_draw_h

#include <stdio.h>

#include "game_object.h"

typedef struct DebugDraw DebugDraw;

DebugDraw *debugdraw_create(void);

void debugdraw_line(DebugDraw *debugDraw, Vector2D start, Vector2D end);
void debugdraw_clear(DebugDraw *debugDraw);

#endif /* debug_draw_h */
