#include "label.h"
#include "scene_manager.h"
#include "image_storage.h"
#include "transforms.h"
#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "string_builder.h"
#include "platform_adapter.h"

void label_render(GameObject *obj, RenderContext *ctx)
{
    Label *self = (Label *)obj;

    Number anchor_x_translate = -nb_mul(nb_mul(obj->anchor.x, obj->size.width), obj->scale.x);
    Number anchor_y_translate = -nb_mul(nb_mul(obj->anchor.y, obj->size.height), obj->scale.y);

    AffineTransform pos = af_identity();

    if (self->rotate_and_scale) {
        pos = af_scale(pos, obj->scale);
        pos = af_translate(pos, (Vector2D){ anchor_x_translate, anchor_y_translate });
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
        
        Number x_offset = nb_from_int(self->w_font_atlas->item_size.width);
        Number y_offset = nb_from_int(self->w_font_atlas->item_size.height);
        
        AffineTransform left_edge = pos;
        
        char glyph;
        for (int32_t i = 0; (glyph = self->text[i]) != '\0'; ++i) {
            if (i >= self->visible_chars) {
                break;
            }
            
            if (glyph == '\n') {
                AffineTransform v_offset = af_translate(af_identity(), (Vector2D){ nb_zero, y_offset });
                left_edge = af_af_multiply(left_edge, v_offset);
                pos = left_edge;
                continue;
            }
            
            Image *img = grid_atlas_w_get_image(self->w_font_atlas, (Vector2DInt){ glyph & 0x1f, (glyph >> 5) - 1 });
            
            ctx->render_transform = pos;
            context_render(ctx, img, render_options_make(false, false, self->invert, false, 0));
            AffineTransform offset = af_translate(af_identity(), (Vector2D){ x_offset, nb_zero });
            pos = af_af_multiply(pos, offset);
        }
    } else {
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);

        int32_t x_offset = self->w_font_atlas->item_size.width;
        int32_t y_offset = self->w_font_atlas->item_size.height;
        int32_t col = 0;
        int32_t row = 0;

        char glyph;
        for (int32_t i = 0; (glyph = self->text[i]) != '\0'; ++i) {
            if (i >= self->visible_chars) {
                break;
            }
            
            if (glyph == '\n') {
                col = 0;
                ++row;
                continue;
            }
            
            Image *img = grid_atlas_w_get_image(self->w_font_atlas, (Vector2DInt){ glyph & 0x1f, (glyph >> 5) - 1 });
            
            context_render_rect_image(ctx, img, (Vector2DInt){ nb_to_int(pos.i13 + anchor_x_translate) + col * x_offset, nb_to_int(pos.i23 + anchor_y_translate) + row * y_offset }, render_options_make(false, false, self->invert, false, 0));
            ++col;
        }
    }
}

void label_destroy(void *value)
{
    Label *label = (Label *)value;
    
    go_destroy(value);
    
    if (label->text) {
        platform_free(label->text);
        label->text = NULL;
    }
}

char *label_describe(void *value)
{
    Label *self = (Label*)value;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "text: ");
    sb_append_string(sb, self->text);
    sb_append_string(sb, " ");
    char *go_description = go_describe(self);
    sb_append_string(sb, go_description);
    platform_free(go_description);

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

GameObjectType LabelType = {
    { { "Label", &label_destroy, &label_describe } },
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    &label_render
};

void label_set_text(Label *label, const char *text)
{
    if (label->text) {
        platform_free(label->text);
        label->text = NULL;
    }
    if (text) {
        int32_t col = 0;
        int32_t len = 0;
        int32_t longest = 0;
        int32_t rows = text[0] == '\0' ? 0 : 1;
        char glyph;
        for (len = 0; (glyph = text[len]) != '\0'; ++len) {
            if (glyph == '\n') {
                ++rows;
                if (col > longest) {
                    longest = col;
                }
                col = 0;
            } else {
                ++col;
            }
        }
        label->size.width = nb_from_int(label->w_font_atlas->item_size.width * max(col, longest));
        label->size.height = nb_from_int(label->w_font_atlas->item_size.height * rows);
        label->text = platform_strdup(text);
        label->text_length = len;
        label->visible_chars = len;
    }
}

Label *label_create(const char *image_name, const char *text)
{
    GameObject *go = go_alloc(sizeof(Label));
    Label *label = (Label *)go;
    label->w_type = &LabelType;
    label->w_font_atlas = get_grid_atlas(image_name);
    label->rotate_and_scale = false;
    label->invert = false;

    label_set_text(label, text);

    return label;
}
