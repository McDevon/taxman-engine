#include "render_texture.h"

#include "string_builder.h"
#include "platform_adapter.h"
#include "game_object_private.h"
#include "transforms.h"

void render_texture_destroy(void *value)
{
    RenderTexture *self = (RenderTexture *)value;
    
    destroy(self->render_context);
    destroy(self->image);
    destroy(self->image_data);
}

char *render_texture_describe(void *value)
{
    RenderTexture *self = (RenderTexture *)value;

    return sb_string_with_format("Size (%d, %d)", self->image_data->size.width, self->image_data->size.height);
}

BaseType RenderTextureType = { "RenderTexture", &render_texture_destroy, &render_texture_describe };

RenderTexture *render_texture_create(Size2DInt size, int32_t channels)
{
    RenderTexture *rt = platform_calloc(1, sizeof(RenderTexture));
    rt->w_type = &RenderTextureType;
    
    rt->image_data = image_data_create(platform_calloc(size.width * size.height * channels, sizeof(uint8_t)), size, 0);
    rt->image = image_from_data(rt->image_data);
    rt->render_context = render_context_create(rt->image_data, false);
    
    return rt;
}

void render_texture_render_go(RenderTexture *self, GameObject *object)
{
    self->render_context->render_transform = render_camera_get_transform(self->render_context->render_camera);
    go_render(object, self->render_context);
}
