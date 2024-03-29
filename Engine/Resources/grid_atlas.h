#ifndef grid_atlas_h
#define grid_atlas_h

#include "image.h"
#include "types.h"

typedef struct GridAtlasInfo {
    char *file_name;
    Size2DInt tile_size;
} GridAtlasInfo;

#define GA_CONTENTS \
    BASE_OBJECT; \
    ImageData *w_atlas; \
    Image *last_image; \
    Size2DInt atlas_size; \
    Size2DInt item_size

typedef struct GridAtlas {
    GA_CONTENTS;
} GridAtlas;

#define GRID_ATLAS union { \
    struct { GA_CONTENTS; }; \
    GridAtlas ga_base; \
}

GridAtlas *grid_atlas_create(ImageData *w_image_data, Size2DInt item_size);
Image *grid_atlas_w_get_image(GridAtlas *atlas, Vector2DInt position);

GridAtlasInfo *grid_atlas_info(const char *file_name, Size2DInt tile_size);
void grid_atlas_info_destroy(void *info);

#endif /* grid_atlas_h */
