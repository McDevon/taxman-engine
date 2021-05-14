#ifndef render_rect_h
#define render_rect_h

#include "base_object.h"

typedef struct RenderRect {
    BASE_OBJECT;
    int left;
    int right;
    int top;
    int bottom;
} RenderRect;

RenderRect *rrect_create(int left, int right, int top, int bottom);
RenderRect *rrect_copy(RenderRect *sq);

#endif /* render_rect_h */
