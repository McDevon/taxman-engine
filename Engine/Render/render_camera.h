#ifndef render_camera_h
#define render_camera_h

#include "base_object.h"
#include "types.h"

typedef struct RenderCamera {
    BASE_OBJECT;
    Size2D viewport_size;
    Vector2D position;
    Vector2D scale;
    Float rotation;
} RenderCamera;

RenderCamera *render_camera_create(Size2DInt viewport_size);

void render_camera_reset(RenderCamera *render_camera);
AffineTransform render_camera_get_transform(RenderCamera *render_camera);

#endif /* render_camera_h */
