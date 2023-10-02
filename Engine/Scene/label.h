#ifndef label_h
#define label_h

#include "game_object.h"
#include "grid_atlas.h"
#include "types.h"
#include "render_texture.h"

#define LABEL_CONTENTS \
    GAME_OBJECT; \
    GridAtlas *w_font_atlas; \
    RenderTexture *render_cache; \
    char *text; \
    int32_t text_length; \
    int32_t visible_chars; \
    DrawMode draw_mode; \
    bool invert

typedef struct Label {
    LABEL_CONTENTS;
} Label;

#define LABEL union { \
    struct { LABEL_CONTENTS; }; \
    Label label_base; \
}

extern GameObjectType LabelType;

Label *label_create(const char *atlas_name, const char *text);
void label_set_text(Label *label, const char *text);
void label_set_visible_chars(Label *label, int32_t visible_chars);

#endif /* label_h */
