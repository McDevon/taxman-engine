#include "render_camera.h"
#include "string_builder.h"
#include "platform_adapter.h"
#include "transforms.h"

void render_camera_destroy(void *value)
{
}

char *render_camera_describe(void *value)
{
    RenderCamera *self = (RenderCamera *)value;

    return sb_string_with_format("Position (%.2f, %.2f) Scale (%.2f, %.2f) Rotation %.2f", self->position.x, self->position.y, self->scale.x, self->scale.y, self->rotation);
}

BaseType RenderCameraType = { "RenderCamera", &render_camera_destroy, &render_camera_describe };

RenderCamera *render_camera_create(Size2DInt viewport_size)
{
    RenderCamera *rt = platform_calloc(1, sizeof(RenderCamera));
    rt->w_type = &RenderCameraType;
    rt->viewport_size = (Size2D){ viewport_size.width, viewport_size.height };
    
    render_camera_reset(rt);
    
    return rt;
}

void render_camera_reset(RenderCamera *self)
{
    self->position = vec_zero();
    self->scale = vec(1.f, 1.f);
    self->rotation = 0.f;
}

AffineTransform render_camera_get_transform(RenderCamera *self)
{
    AffineTransform af = af_identity();
    
    af = af_translate(af, vec(-self->position.x, -self->position.y));
    af = af_rotate(af, self->rotation);
    af = af_scale(af, self->scale);
    af = af_translate(af, vec(self->viewport_size.width / 2, self->viewport_size.height / 2));
    
    return af;
}
