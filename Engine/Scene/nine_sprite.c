#include "nine_sprite.h"
#include "scene_manager.h"
#include "image_storage.h"
#include "transforms.h"
#include "platform_adapter.h"
#include "engine_log.h"
#include <stdio.h>

struct ns_private {
    Image *image_top_left;
    Image *image_top_middle;
    Image *image_top_right;
    Image *image_middle_left;
    Image *image_center;
    Image *image_middle_right;
    Image *image_bottom_left;
    Image *image_bottom_middle;
    Image *image_bottom_right;
    int32_t left_split;
    int32_t right_split;
    int32_t high_split;
    int32_t low_split;
    int32_t width;
    int32_t height;
};

void nine_sprite_render(GameObject *obj, RenderContext *ctx)
{
    NineSprite *self = (NineSprite *)obj;
    
    Number anchor_x = nb_mul(obj->anchor.x, obj->size.width);
    Number anchor_y = nb_mul(obj->anchor.y, obj->size.height);

    Number anchor_x_translate = -nb_mul(anchor_x, obj->scale.x);
    Number anchor_y_translate = -nb_mul(anchor_y, obj->scale.y);

    AffineTransform pos = af_identity();
    
    pos = af_scale(pos, obj->scale);
    pos = af_translate(pos, (Vector2D){ anchor_x_translate, anchor_y_translate });
    pos = af_rotate(pos, obj->rotation);
    pos = af_translate(pos, obj->position);
    pos = af_af_multiply(ctx->render_transform, pos);
    
    int32_t left = nb_to_int(nb_floor(pos.i13));
    int32_t top = nb_to_int(nb_floor(pos.i23));
    int32_t left_split = left + self->ns_private->left_split;
    int32_t right_split = left + nb_to_int(self->size.width - nb_from_int(self->ns_private->width - self->ns_private->right_split));
    int32_t high_split = top + self->ns_private->high_split;
    int32_t low_split = top + nb_to_int(self->size.height - nb_from_int(self->ns_private->height - self->ns_private->low_split));
    
    int32_t middle_width = self->ns_private->right_split - self->ns_private->left_split;
    int32_t middle_height = self->ns_private->low_split - self->ns_private->high_split;
    Number scale_x = nb_div(self->size.width - nb_from_int(self->ns_private->width - middle_width), nb_from_int(middle_width));
    Number scale_y = nb_div(self->size.height - nb_from_int(self->ns_private->height - middle_height), nb_from_int(middle_height));

    // Top row
    context_render_rect_image(ctx,
                              self->ns_private->image_top_left,
                              (Vector2DInt){ left, top },
                              render_options_make(false,
                                                  false,
                                                  self->invert)
                              );

    context_render_scale_image(ctx,
                               self->ns_private->image_top_middle,
                               (Vector2DInt){ left_split, top },
                               vec(scale_x, nb_one),
                               render_options_make(false,
                                                   false,
                                                   self->invert)
                               );

    context_render_rect_image(ctx,
                              self->ns_private->image_top_right,
                              (Vector2DInt){ right_split, top },
                              render_options_make(false,
                                                  false,
                                                  self->invert)
                              );

    // Middle row
    context_render_scale_image(ctx,
                               self->ns_private->image_middle_left,
                               (Vector2DInt){ left, high_split },
                               vec(nb_one, scale_y),
                               render_options_make(false,
                                                   false,
                                                   self->invert)
                               );

    context_render_scale_image(ctx,
                               self->ns_private->image_center,
                               (Vector2DInt){ left_split, high_split },
                               vec(scale_x, scale_y),
                               render_options_make(false,
                                                   false,
                                                   self->invert)
                               );

    context_render_scale_image(ctx,
                               self->ns_private->image_middle_right,
                               (Vector2DInt){ right_split, high_split },
                               vec(nb_one, scale_y),
                               render_options_make(false,
                                                   false,
                                                   self->invert)
                               );
    
    // Bottom row
    context_render_rect_image(ctx,
                              self->ns_private->image_bottom_left,
                              (Vector2DInt){ left, low_split },
                              render_options_make(false,
                                                  false,
                                                  self->invert)
                              );

    context_render_scale_image(ctx,
                               self->ns_private->image_bottom_middle,
                               (Vector2DInt){ left_split, low_split },
                               vec(scale_x, nb_one),
                               render_options_make(false,
                                                   false,
                                                   self->invert)
                               );

    context_render_rect_image(ctx,
                              self->ns_private->image_bottom_right,
                              (Vector2DInt){ right_split, low_split },
                              render_options_make(false,
                                                  false,
                                                  self->invert)
                              );
}

void nine_sprite_destroy(void *nine_sprite)
{
    NineSprite *self = (NineSprite *)nine_sprite;
    
    if (self->ns_private->image_top_left) {
        destroy(self->ns_private->image_top_left);
    }
    if (self->ns_private->image_top_middle) {
        destroy(self->ns_private->image_top_middle);
    }
    if (self->ns_private->image_top_right) {
        destroy(self->ns_private->image_top_right);
    }
    if (self->ns_private->image_middle_left) {
        destroy(self->ns_private->image_middle_left);
    }
    if (self->ns_private->image_center) {
        destroy(self->ns_private->image_center);
    }
    if (self->ns_private->image_middle_right) {
        destroy(self->ns_private->image_middle_right);
    }
    if (self->ns_private->image_bottom_left) {
        destroy(self->ns_private->image_bottom_left);
    }
    if (self->ns_private->image_bottom_middle) {
        destroy(self->ns_private->image_bottom_middle);
    }
    if (self->ns_private->image_bottom_right) {
        destroy(self->ns_private->image_bottom_right);
    }
    
    platform_free(self->ns_private);

    go_destroy(nine_sprite);
}

char *nine_sprite_describe(void *nine_sprite)
{
    return go_describe(nine_sprite);
}

GameObjectType NineSpriteType = {
    { { "NineSprite", &nine_sprite_destroy, &nine_sprite_describe } },
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &nine_sprite_render
};

void nine_sprite_set_image(NineSprite *self, Image *image, int32_t x_left_split, int32_t x_right_split, int32_t y_high_split, int32_t y_low_split)
{
    self->w_image = image;
    self->size.width = nb_from_int(image->original.width);
    self->size.height = nb_from_int(image->original.height);
    
    if (x_left_split < 0 || x_left_split >= self->size.width) {
        LOG_ERROR("NineSprite: Left split out of bounds");
        return;
    }
    if (x_right_split <= x_left_split || x_right_split >= self->size.width) {
        LOG_ERROR("NineSprite: Right split out of bounds");
        return;
    }
    if (y_high_split < 0 || y_high_split >= self->size.height) {
        LOG_ERROR("NineSprite: High split out of bounds");
        return;
    }
    if (y_low_split <= y_high_split || y_low_split >= self->size.height) {
        LOG_ERROR("NineSprite: Low split out of bounds");
        return;
    }
    
    self->ns_private->left_split = x_left_split;
    self->ns_private->right_split = x_right_split;
    self->ns_private->high_split = y_high_split;
    self->ns_private->low_split = y_low_split;
    self->ns_private->width = image->original.width;
    self->ns_private->height = image->original.height;

    self->ns_private->image_top_left = image_create(image->w_image_data, (Rect2DInt){
        { image->rect.origin.x, image->rect.origin.y },
        { x_left_split - image->rect.origin.x, y_high_split - image->rect.origin.y }
    });
    self->ns_private->image_top_middle = image_create(image->w_image_data, (Rect2DInt){
        { image->rect.origin.x + x_left_split, image->rect.origin.y },
        { x_right_split - x_left_split, y_high_split - image->rect.origin.y }
    });
    self->ns_private->image_top_right = image_create(image->w_image_data, (Rect2DInt){
        { image->rect.origin.x + x_right_split, image->rect.origin.y },
        { image->rect.origin.x + image->rect.size.width - x_right_split, y_high_split - image->rect.origin.y }
    });
    
    self->ns_private->image_middle_left = image_create(image->w_image_data, (Rect2DInt){
        { image->rect.origin.x, image->rect.origin.y + y_high_split },
        { x_left_split - image->rect.origin.x, y_low_split - y_high_split }
    });
    self->ns_private->image_center = image_create(image->w_image_data, (Rect2DInt){
        { image->rect.origin.x + x_left_split, image->rect.origin.y + y_high_split },
        { x_right_split - x_left_split, y_low_split - y_high_split }
    });
    self->ns_private->image_middle_right = image_create(image->w_image_data, (Rect2DInt){
        { image->rect.origin.x + x_right_split, image->rect.origin.y + y_high_split },
        { image->rect.origin.x + image->rect.size.width - x_right_split, y_low_split - y_high_split }
    });
    
    self->ns_private->image_bottom_left = image_create(image->w_image_data, (Rect2DInt){
        { image->rect.origin.x, image->rect.origin.y + y_low_split },
        { x_left_split - image->rect.origin.x, image->rect.origin.y + image->rect.size.height - y_low_split }
    });
    self->ns_private->image_bottom_middle = image_create(image->w_image_data, (Rect2DInt){
        { image->rect.origin.x + x_left_split, image->rect.origin.y + y_low_split },
        { x_right_split - x_left_split, image->rect.origin.y + image->rect.size.height - y_low_split }
    });
    self->ns_private->image_bottom_right = image_create(image->w_image_data, (Rect2DInt){
        { image->rect.origin.x + x_right_split, image->rect.origin.y + y_low_split },
        { image->rect.origin.x + image->rect.size.width - x_right_split, image->rect.origin.y + image->rect.size.height - y_low_split }
    });
}

void nine_sprite_set_size(NineSprite *nine_sprite, Size2D size)
{
    nine_sprite->size = size;
}

NineSprite *nine_sprite_create(const char *image_name, int32_t x_left_split, int32_t x_right_split, int32_t y_high_split, int32_t y_low_split)
{
    GameObject *go = go_alloc(sizeof(NineSprite));
    NineSprite *sprite = (NineSprite *)go;
    sprite->ns_private = platform_calloc(1, sizeof(struct ns_private));
    go->w_type = &NineSpriteType;
    nine_sprite_set_image(sprite, get_image(image_name), x_left_split, x_right_split, y_high_split, y_low_split);

    sprite->invert = false;

    return sprite;
}
