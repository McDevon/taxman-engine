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
    
    if (self->draw_mode == drawmode_default) {
        pos = af_scale(pos, obj->scale);
        pos = af_translate(pos, (Vector2D){ anchor_x_translate, anchor_y_translate });
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);

        context_render_rect_image(ctx,
                                  self->w_image,
                                  (Vector2DInt){ nb_to_int(nb_floor(pos.i13)), nb_to_int(nb_floor(pos.i23)) },
                                  render_options_make(self->flip_x,
                                                      self->flip_y,
                                                      self->invert,
                                                      self->stamp,
                                                      self->stamp_color)
                                  );
    } else if (self->draw_mode == drawmode_scale) {
        pos = af_scale(pos, obj->scale);
        pos = af_translate(pos, (Vector2D){ anchor_x_translate, anchor_y_translate });
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
        
        AffineTransform scale_transform = af_af_multiply(pos, af_identity());
        
        context_render_scale_image(ctx,
                                   self->w_image,
                                   (Vector2DInt){ nb_to_int(nb_floor(pos.i13)), nb_to_int(nb_floor(pos.i23)) },
                                   vec(scale_transform.i11, scale_transform.i22),
                                   render_options_make(self->flip_x,
                                                       self->flip_y,
                                                       self->invert,
                                                       self->stamp,
                                                       self->stamp_color)
                                   );
    } else if (self->draw_mode == drawmode_rotate_and_scale) {
        pos = af_scale(pos, obj->scale);
        pos = af_translate(pos, (Vector2D){ anchor_x_translate, anchor_y_translate });
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
        
        ctx->render_transform = pos;

        context_render(ctx,
                       self->w_image,
                       render_options_make(self->flip_x,
                                           self->flip_y,
                                           self->invert,
                                           self->stamp,
                                           self->stamp_color)
                       );
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

    sprite->draw_mode = drawmode_default;
    sprite->flip_x = false;
    sprite->flip_y = false;
    sprite->invert = false;
    sprite->stamp = false;
    sprite->stamp_color = 0;

    return sprite;
}
