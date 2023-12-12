#ifndef types_h
#define types_h

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef float Float;

typedef void (resource_callback_t)(const char *, bool, void *context);
typedef void (context_callback_t)(void *context);
typedef void (object_callback_t)(void *, void *context);

typedef struct ResourceCallbackContainer {
    resource_callback_t *resource_callback;
    void *context;
} ResourceCallbackContainer;

typedef struct Vector2D {
    Float x;
    Float y;
} Vector2D;

typedef struct Pose2D {
    Vector2D position;
    Float rotation;
} Pose2D;

typedef struct Size2D {
    Float width;
    Float height;
} Size2D;

typedef struct AffineTransform {
    Float i11;
    Float i12;
    Float i13;
    
    Float i21;
    Float i22;
    Float i23;
} AffineTransform;

typedef struct Rect2D {
    Vector2D origin;
    Size2D size;
} Rect2D;

typedef struct Edges {
    Float left;
    Float right;
    Float top;
    Float bottom;
} Edges;

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

typedef struct Controls {
    Float crank;
    uint8_t button_left : 1;
    uint8_t button_right : 1;
    uint8_t button_up : 1;
    uint8_t button_down : 1;
    uint8_t button_a : 1;
    uint8_t button_b : 1;
    uint8_t button_menu : 1;
} Controls;

typedef enum {
    dir_left,
    dir_right,
    dir_up,
    dir_down
} Direction;

typedef struct DirectionTable {
    uint8_t left : 1;
    uint8_t right : 1;
    uint8_t up : 1;
    uint8_t down : 1;
} DirectionTable;

typedef enum {
    drawmode_default,
    drawmode_scale,
    drawmode_rotate,
    drawmode_rotate_and_scale,
} DrawMode;

#define directions_none (DirectionTable){ 0, 0, 0, 0 }
#define directions_all (DirectionTable){ 1, 1, 1, 1 }

#define int_rect_make(x, y, w, h) (Rect2DInt){ (Vector2DInt){ x, y }, (Size2DInt){ w, h } }
#define rect_make(x, y, w, h) (Rect2D){ (Vector2D){ x, y }, (Size2D){ w, h } }

#define empty_controls (Controls){ (Float)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)0 }

#endif /* types_h */
