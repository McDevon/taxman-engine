#include "sprite.h"
#include "scene_manager.h"
#include "image_storage.h"
#include "transforms.h"
#include "image_object_render.h"
#include <stdio.h>

void sprite_render(GameObject *obj, RenderContext *ctx)
{
    Sprite *self = (Sprite *)obj;
    
    image_object_render(self->w_image,
                        obj,
                        render_options_make(self->flip_x,
                                            self->flip_y,
                                            self->invert),
                        self->draw_mode,
                        ctx
                        );
}

void sprite_destroy(void *sprite)
{
    go_destroy(sprite);
}

char *sprite_describe(void *sprite)
{
    return go_describe(sprite);
}

GameObjectType SpriteType =
    game_object_type("Sprite",
                     &sprite_destroy,
                     &sprite_describe,
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     NULL,
                     &sprite_render
                     );

void sprite_set_image(Sprite *self, Image *image)
{
    self->w_image = image;
    self->size.width = image->original.width;
    self->size.height = image->original.height;
}

Sprite *sprite_create(const char *image_name)
{
    GameObject *go = go_alloc(sizeof(Sprite));
    Sprite *sprite = (Sprite *)go;
    go->w_type = &SpriteType;
    sprite_set_image(sprite, get_image(image_name));

    sprite->draw_mode = drawmode_default;
    sprite->flip_x = false;
    sprite->flip_y = false;
    sprite->invert = false;

    return sprite;
}

Sprite *sprite_create_with_image(Image *image)
{
    GameObject *go = go_alloc(sizeof(Sprite));
    Sprite *sprite = (Sprite *)go;
    go->w_type = &SpriteType;
    sprite_set_image(sprite, image);

    sprite->draw_mode = drawmode_default;
    sprite->flip_x = false;
    sprite->flip_y = false;
    sprite->invert = false;

    return sprite;
}

