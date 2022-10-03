#include "off_screen_renderer.h"

void off_screen_renderer_destroy(void *comp)
{
    OffScreenRenderer *osr = (OffScreenRenderer *)comp;
    destroy(osr->root_object);
    destroy(osr->internal_scene_manager);
    destroy(osr->render_texture);
    comp_destroy(comp);
}

char *off_screen_renderer_describe(void *comp)
{
    return comp_describe(comp);
}

void off_screen_renderer_start(GameObjectComponent *comp)
{
    OffScreenRenderer *self = (OffScreenRenderer *)comp;
    
    go_start(self->root_object);
}

void off_screen_renderer_update(GameObjectComponent *comp, Number dt_ms)
{
    OffScreenRenderer *self = (OffScreenRenderer *)comp;
    
    go_update(self->root_object, dt_ms);
    
    size_t count = list_count(self->internal_scene_manager->go_destroy_queue);
    
    if (count > 0) {
        for (size_t i = 0; i < count; ++i) {
            GameObject *obj = list_get(self->internal_scene_manager->go_destroy_queue, i);
            go_remove_from_parent(obj);
            destroy(obj);
        }
        
        list_clear(self->internal_scene_manager->go_destroy_queue);
    }
    
    count = list_count(self->internal_scene_manager->comp_destroy_queue);
    
    if (count > 0) {
        for (size_t i = 0; i < count; ++i) {
            GameObjectComponent *comp = list_get(self->internal_scene_manager->comp_destroy_queue, i);
            comp_remove_from_parent(comp);
            destroy(comp);
        }
        
        list_clear(self->internal_scene_manager->comp_destroy_queue);
    }
    
    render_texture_render_go(self->render_texture, self->root_object);
}

void off_screen_renderer_fixed_update(GameObjectComponent *comp, Number dt_ms)
{
    OffScreenRenderer *self = (OffScreenRenderer *)comp;
    
    go_fixed_update(self->root_object, dt_ms);
}


GameObjectComponentType OffScreenRendererComponentType = {
    { { "OffScreenRendererComponent", &off_screen_renderer_destroy, &off_screen_renderer_describe } },
    NULL,
    NULL,
    &off_screen_renderer_start,
    &off_screen_renderer_update,
    &off_screen_renderer_fixed_update
};

OffScreenRenderer *off_screen_renderer_create(Size2DInt size, int32_t channels)
{
    OffScreenRenderer *osr = (OffScreenRenderer *)comp_alloc(sizeof(OffScreenRenderer));
    
    osr->w_type = &OffScreenRendererComponentType;
    
    osr->render_texture = render_texture_create(size, channels);
    osr->internal_scene_manager = scene_manager_create();
    osr->root_object = go_create_empty();
    
    go_initialize(osr->root_object, osr->internal_scene_manager);
    
    return osr;
}

void off_screen_renderer_add_go(OffScreenRenderer *self, void *child)
{
    go_add_child(self->root_object, child);
}

void off_screen_renderer_render_rect_image(OffScreenRenderer *self, const Image *image, const Vector2DInt position, const RenderOptions render_options)
{
    context_render_rect_image(self->render_texture->render_context, image, position, render_options);
}
