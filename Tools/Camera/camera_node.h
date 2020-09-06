#ifndef camera_node_h
#define camera_node_h

#include "engine.h"

typedef struct CameraNode CameraNode;

CameraNode *camera_create(Size2D viewport_size);

void camera_move(CameraNode *camera_node, Vector2D position);
void camera_rotate(CameraNode *camera_node, Number rotation);
void camera_scale(CameraNode *camera_node, Number scale);

void camera_set_position(CameraNode *camera_node, Vector2D position);
void camera_set_rotation(CameraNode *camera_node, Number rotation);
void camera_set_scale(CameraNode *camera_node, Number scale);

Vector2D camera_get_position(CameraNode *camera_node);
Number camera_get_rotation(CameraNode *camera_node);
Number camera_get_scale(CameraNode *camera_node);

#endif /* camera_node_h */
