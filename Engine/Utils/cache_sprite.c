#include "cache_sprite.h"
#include "scene_manager.h"
#include "image_storage.h"
#include "transforms.h"
#include "image_object_render.h"
#include <stdio.h>

void cache_sprite_render(GameObject *obj, RenderContext *ctx)
{
    CacheSprite *self = (CacheSprite *)obj;
    
    image_object_render(self->render_texture->image,
                        obj,
                        render_options_make(false,
                                            false,
                                            false),
                        drawmode_default,
                        ctx
                        );
}

void cache_sprite_destroy(void *sprite)
{
    go_destroy(sprite);
}

char *cache_sprite_describe(void *sprite)
{
    return go_describe(sprite);
}

GameObjectType CacheSpriteType =
    game_object_type("CacheSprite",
                     &cache_sprite_destroy,
                     &cache_sprite_describe,
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     &cache_sprite_render
                     );

CacheSprite *cache_sprite_create_rotated(const char *image_name, Float angle, Vector2D anchor)
{
    return cache_sprite_sprite_create_with_image_rotated(get_image(image_name), angle, anchor);
}

CacheSprite *cache_sprite_sprite_create_with_image_rotated(Image *image, Float angle, Vector2D anchor)
{
    GameObject *go = go_alloc(sizeof(CacheSprite));
    go->w_type = &CacheSpriteType;
    
    CacheSprite *sprite = (CacheSprite *)go;
    
    sprite->w_original_image = image;
    sprite->size.width = image->original.width;
    sprite->size.height = image->original.height;

    sprite->flip_x = false;
    sprite->flip_y = false;
    sprite->invert = false;
    
    sprite->original_anchor = anchor;
    
    cache_sprite_set_rotated(sprite, angle);
    
    return sprite;
}

void cache_sprite_set_rotated(CacheSprite *self, Float angle)
{
    if (self->render_texture) {
        destroy(self->render_texture);
        self->render_texture = NULL;
    }
    
    Vector2D local_anchor = vec(self->original_anchor.x * self->w_original_image->original.width, self->original_anchor.y * self->w_original_image->original.height);
    
    self->render_texture = render_texture_create_with_rotated_anchored(self->w_original_image, angle, &local_anchor);
    
    self->anchor = vec(local_anchor.x / self->render_texture->image->original.width, local_anchor.y / self->render_texture->image->original.height);
}
