#ifndef off_screen_renderer_h
#define off_screen_renderer_h

#include "game_object_component.h"
#include "render_texture.h"
#include "scene_manager.h"
#include "sprite.h"

typedef struct OffScreenRenderer {
    GAME_OBJECT_COMPONENT;
    RenderTexture *render_texture;
    SceneManager *internal_scene_manager;
    GameObject *root_object;
} OffScreenRenderer;

extern GameObjectComponentType OffScreenRendererType;

OffScreenRenderer *off_screen_renderer_create(Size2DInt size, int32_t channels);

void off_screen_renderer_add_go(OffScreenRenderer *off_screen_renderer, void *go);
void off_screen_renderer_render_rect_image(OffScreenRenderer *off_screen_renderer, const Image *image, const Vector2DInt position, const uint8_t flip_flags_xy, const bool invert);

#endif /* off_screen_renderer_h */
