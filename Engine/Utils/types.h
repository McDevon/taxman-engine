#ifndef types_h
#define types_h

#include <stdlib.h>
#include "number.h"

typedef uint8_t Bool;
typedef float Float;

#define True 1
#define False 0

typedef struct Vector2D {
    Number x;
    Number y;
} Vector2D;

typedef struct Size2D {
    Number width;
    Number height;
} Size2D;

typedef struct AffineTransform {
    Number i11;
    Number i12;
    Number i13;
    
    Number i21;
    Number i22;
    Number i23;
} AffineTransform;

typedef struct Rect2D {
    Vector2D origin;
    Size2D size;
} Rect2D;

typedef struct Vector2DInt {
    int32_t x;
    int32_t y;
} Vector2DInt;

typedef struct Size2DInt {
    int32_t width;
    int32_t height;
} Size2DInt;

typedef struct Rect2DInt {
    Vector2DInt origin;
    Size2DInt size;
} Rect2DInt;

typedef struct Vector2DFloat {
    Float x;
    Float y;
} Vector2DFloat;

typedef struct AffineTransformFloat {
    Float i11;
    Float i12;
    Float i13;
    
    Float i21;
    Float i22;
    Float i23;
} AffineTransformFloat;

typedef struct Controls {
    Number crank;
    uint8_t button_left;
    uint8_t button_right;
    uint8_t button_up;
    uint8_t button_down;
    uint8_t button_a;
    uint8_t button_b;
    uint8_t button_menu;
} Controls;

typedef enum {
    dir_left,
    dir_right,
    dir_up,
    dir_down
} Direction;

#define int_rect_make(x, y, w, h) (Rect2DInt){ (Vector2DInt){ x, y }, (Size2DInt){ w, h } }
#define rect_make(x, y, w, h) (Rect2D){ (Vector2D){ x, y }, (Size2D){ w, h } }

#define EMPTY_CONTROLS() { (uint32_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0 }

#endif /* types_h */
