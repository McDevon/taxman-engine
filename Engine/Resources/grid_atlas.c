#include "grid_atlas.h"
#include "engine_log.h"
#include "string_builder.h"
#include "platform_adapter.h"

void grid_atlas_destroy(void *value);
char *grid_atlas_describe(void *value);

BaseType GridAtlasType = { "GridAtlas", &grid_atlas_destroy, &grid_atlas_describe };

GridAtlas *grid_atlas_create(ImageData *w_image_data, Size2DInt item_size)
{
    if (item_size.width > w_image_data->size.width || item_size.height > w_image_data->size.height) {
        LOG_ERROR("GridAtlas item size larger than atlas size");
        return NULL;
    }
    
    GridAtlas *atlas = platform_calloc(1, sizeof(GridAtlas));
    atlas->w_type = &GridAtlasType;
    atlas->w_atlas = w_image_data;
    atlas->item_size = item_size;
    
    atlas->atlas_size.width = w_image_data->size.width / item_size.width;
    atlas->atlas_size.height = w_image_data->size.height / item_size.height;
    
    atlas->last_image = image_create(w_image_data, int_rect_make(0, 0, item_size.width, item_size.height));

    return atlas;
}

void grid_atlas_destroy(void *value)
{
    GridAtlas *self = (GridAtlas *)value;
    destroy(self->last_image);
}

char *grid_atlas_describe(void *value)
{
    GridAtlas *self = (GridAtlas *)value;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "image: ");
    char *image_description = describe(self->w_atlas);
    sb_append_string(sb, image_description);
    sb_append_string(sb, " grid size: ");
    sb_append_int_size(sb, self->atlas_size);
    sb_append_string(sb, " item size: ");
    sb_append_int_size(sb, self->item_size);

    char *description = sb_get_string(sb);
    platform_free(image_description);
    destroy(sb);

    return description;
}

Image *grid_atlas_w_get_image(GridAtlas *atlas, Vector2DInt position)
{
    atlas->last_image->rect.origin.x = position.x * atlas->item_size.width;
    atlas->last_image->rect.origin.y = position.y * atlas->item_size.height;
    
    return atlas->last_image;
}
