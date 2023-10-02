#include "label.h"
#include "scene_manager.h"
#include "image_storage.h"
#include "transforms.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "utils.h"
#include "string_builder.h"
#include "platform_adapter.h"
#include "image_object_render.h"

void label_render(GameObject *obj, RenderContext *ctx)
{
    Label *self = (Label *)obj;
    image_object_render(self->render_cache->image, obj, render_options_make(false, false, self->invert), self->draw_mode, ctx);
}

void label_destroy(void *value)
{
    Label *label = (Label *)value;
    
    go_destroy(value);
    
    if (label->text) {
        platform_free(label->text);
        label->text = NULL;
    }
    if (label->render_cache) {
        destroy(label->render_cache);
        label->render_cache = NULL;
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

void label_render_cached_image(Label *self, int32_t start_index) {
    int32_t x_offset = self->w_font_atlas->item_size.width;
    int32_t y_offset = self->w_font_atlas->item_size.height;
    int32_t col = 0;
    int32_t row = 0;
        
    if (start_index < 0) {
        start_index = 0;
    } else if (start_index >= self->text_length) {
        return;
    }
    
    char glyph;
    if (start_index > 0) {
        for (int32_t i = 0; i < start_index && (glyph = self->text[i]) != '\0'; ++i) {
            if (glyph == '\n') {
                col = 0;
                ++row;
                continue;
            }
            ++col;
        }
    }

    for (int32_t i = start_index; (glyph = self->text[i]) != '\0'; ++i) {
        if (i >= self->visible_chars) {
            break;
        }
        
        if (glyph == '\n') {
            col = 0;
            ++row;
            continue;
        }
        
        Image *img = grid_atlas_w_get_image(self->w_font_atlas, (Vector2DInt){ glyph & 0x1f, (glyph >> 5) - 1 });
        
        context_render_rect_image(self->render_cache->render_context, img, (Vector2DInt){ col * x_offset, row * y_offset }, render_options_make(false, false, self->invert));
        ++col;
    }
}

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
        
        if (label->render_cache == NULL) {
            label->render_cache = render_texture_create((Size2DInt){nb_to_int(label->size.width), nb_to_int(label->size.height)}, image_data_channel_count(label->w_font_atlas->w_atlas));
        } else {
            render_texture_resize(label->render_cache, (Size2DInt){nb_to_int(label->size.width), nb_to_int(label->size.height)});
        }
        context_clear_transparent_white(label->render_cache->render_context);
        label_render_cached_image(label, 0);
    }
}

void label_set_visible_chars(Label *label, const int32_t visible_chars)
{
    if (visible_chars == label->visible_chars) {
        return;
    }
    if (visible_chars < 0) {
        label->visible_chars = 0;
        context_clear_transparent_white(label->render_cache->render_context);
    } else if (visible_chars > label->text_length) {
        int32_t previously_visible_chars = label->visible_chars;
        label->visible_chars = label->text_length;
        label_render_cached_image(label, previously_visible_chars);
    } else if (visible_chars > label->visible_chars) {
        int32_t previously_visible_chars = label->visible_chars;
        label->visible_chars = visible_chars;
        label_render_cached_image(label, previously_visible_chars);
    } else {
        label->visible_chars = visible_chars;
        context_clear_transparent_white(label->render_cache->render_context);
        label_render_cached_image(label, 0);
    }
}

Label *label_create(const char *image_name, const char *text)
{
    GameObject *go = go_alloc(sizeof(Label));
    Label *label = (Label *)go;
    label->w_type = &LabelType;
    label->w_font_atlas = get_grid_atlas(image_name);
    label->draw_mode = drawmode_default;
    label->invert = false;

    label_set_text(label, text);

    return label;
}
