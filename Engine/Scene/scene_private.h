#ifndef scene_private_h
#define scene_private_h

#include "grid_atlas.h"
#include "array_list.h"

struct scene_private {
    ArrayList *sprite_sheet_names;
    ArrayList *grid_atlas_infos;
};

#endif /* scene_private_h */
