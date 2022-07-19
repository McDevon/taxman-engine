#include "sprite.h"
#include "scene_manager.h"
#include "image_storage.h"
#include "transforms.h"
#include <stdio.h>

void sprite_render(GameObject *obj, RenderContext *ctx)
{
    Sprite *self = (Sprite *)obj;

    Number anchor_x_translate = -nb_mul(nb_mul(obj->anchor.x, obj->size.width), obj->scale.x);
    Number anchor_y_translate = -nb_mul(nb_mul(obj->anchor.y, obj->size.height), obj->scale.y);

    AffineTransform pos = af_identity();
    
    uint8_t flip_flags = self->flip_x + 2 * self->flip_y;

    if (self->rotate_and_scale) {
        pos = af_scale(pos, obj->scale);
        pos = af_translate(pos, (Vector2D){ anchor_x_translate, anchor_y_translate });
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->camera_matrix, pos);
        
        ctx->camera_matrix = pos;

        context_render(ctx, self->w_image, flip_flags, self->invert);
    } else {
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->camera_matrix, pos);

        context_render_rect_image(ctx, self->w_image, (Vector2DInt){ nb_to_int(pos.i13 + anchor_x_translate), nb_to_int(pos.i23 + anchor_y_translate) }, flip_flags, self->invert);
    }
}

void sprite_destroy(void *sprite)
{
    go_destroy(sprite);
}

char *sprite_describe(void *sprite)
{
    return go_describe(sprite);
}

GameObjectType SpriteType = {
    { { "Sprite", &sprite_destroy, &sprite_describe } },
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &sprite_render
};

void sprite_set_image(Sprite *self, Image *image)
{
    self->w_image = image;
    self->size.width = nb_from_int(image->original.width);
    self->size.height = nb_from_int(image->original.height);
}

Sprite *sprite_create(const char *image_name)
{
    GameObject *go = go_alloc(sizeof(Sprite));
    Sprite *sprite = (Sprite *)go;
    go->w_type = &SpriteType;
    sprite_set_image(sprite, get_image(image_name));

    sprite->rotate_and_scale = true;
    sprite->flip_x = false;
    sprite->flip_y = false;
    sprite->invert = false;

    return sprite;
}
