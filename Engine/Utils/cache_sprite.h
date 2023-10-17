#ifndef cache_sprite_h
#define cache_sprite_h

#include "game_object.h"
#include "render_texture.h"
#include "types.h"

typedef struct CacheSprite {
    GAME_OBJECT;
    Image *w_original_image;
    RenderTexture *render_texture;
    Vector2D original_anchor;
    bool flip_x;
    bool flip_y;
    DrawMode draw_mode;
    bool invert;
} CacheSprite;

extern GameObjectType CacheSpriteType;

CacheSprite *cache_sprite_create_rotated(const char *image_name, Number angle, Vector2D anchor);
CacheSprite *cache_sprite_sprite_create_with_image_rotated(Image *image, Number angle, Vector2D anchor);

void cache_sprite_set_rotated(CacheSprite *sprite, Number angle);

#endif /* cache_sprite_h */
