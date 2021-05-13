#ifndef render_square_h
#define render_square_h

#include "base_object.h"

typedef struct Square {
    BASE_OBJECT;
    int left;
    int right;
    int top;
    int bottom;
} Square;

Square *square_create(int left, int right, int top, int bottom);
Square *square_copy(Square *sq);

#endif /* render_square_h */
