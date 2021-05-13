#include "render_context.h"
#include "render_square.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "engine_log.h"
#include <string.h>

void render_context_destroy(void *value)
{
    RenderContext *self = (RenderContext *)value;
    if (self->rendered_squares) {
        destroy(self->rendered_squares);
    }
    if (self->square_pool) {
        destroy(self->square_pool);
    }
}

char *render_context_describe(void *value)
{
    return platform_strdup("[]");
}

void context_square_rendered(RenderContext *self, int left, int right, int top, int bottom)
{
    if (!self->background_enabled) {
        return;
    }
    
    Square *sq = NULL;
    size_t pool_count = list_count(self->square_pool);
    if (pool_count) {
        sq = list_drop_index(self->square_pool, pool_count - 1);
        sq->left = left;
        sq->right = right;
        sq->top = top;
        sq->bottom = bottom;
    } else {
        sq = square_create(left, right, top, bottom);
    }
    list_add(self->rendered_squares, sq);
}

int context_square_compare_left_edge(const void *a, const void *b)
{
    Square *sq_a = *(Square**)a;
    Square *sq_b = *(Square**)b;
    
    int diff = sq_a->left - sq_b->left;
    
    if (diff < nb_zero) {
        return list_sorted_ascending;
    } else if (diff > nb_zero) {
        return list_sorted_descending;
    } else {
        return list_sorted_same;
    }
}

int context_square_compare_right_edge(const void *a, const void *b)
{
    Square *sq_a = *(Square**)a;
    Square *sq_b = *(Square**)b;
    
    int diff = sq_a->right - sq_b->right;
    
    if (diff < nb_zero) {
        return list_sorted_ascending;
    } else if (diff > nb_zero) {
        return list_sorted_descending;
    } else {
        return list_sorted_same;
    }
}

void context_clean_union_of_rendered_squares(ArrayList *rendered_squares, ArrayList *result)
{
    if (!rendered_squares || !result) {
        LOG_ERROR("clean_union_of_rendered_squares: received null parameter");
        return;
    }
    
    if (list_count(result) != 0) {
        LOG_ERROR("clean_union_of_rendered_squares: result array not empty");
        return;
    }
    
    size_t count = list_count(rendered_squares);
    
    ArrayList *actives = list_create();
    ArrayList *temps = list_create_with_weak_references();
    ArrayList *ends = list_create_with_weak_references();

    list_sort(rendered_squares, &context_square_compare_left_edge);
    
    size_t i = 0;
    while (i < count || list_count(ends)) {
        Square *sq = list_count(rendered_squares) ? list_get(rendered_squares, i) : NULL;
        
        if (list_count(ends)) {
            Square *next_end = (Square *)list_get(ends, 0);
            if (!sq || next_end->right < sq->left) {
                // Square ended
                Square *dropped = NULL;
                
                const size_t actives_count = list_count(actives);
                for (int32_t k = (int32_t)actives_count - 1; k >= 0; --k) {
                    Square *active = list_get(actives, k);
                    
                    if (active->top > next_end->bottom || active->bottom < next_end->top) {
                        // Active does not overlap with ended square
                        continue;
                    }
                    
                    if (next_end->top > active->top && next_end->bottom < active->bottom && next_end->right < active->right) {
                        // Ended square completely contained in this active
                        break;
                    }
                    
                    // Ended square overlaps with active but not completely contained
                    list_add(result, square_create(active->left, next_end->right, active->top, active->bottom));

                    list_drop_index(actives, k);
                    dropped = active;
                    //break;
                }
                
                list_drop_index(ends, 0);

                if (dropped) {
                    const size_t ends_count = list_count(ends);
                    for (int32_t k = (int32_t)ends_count - 1; k >= 0; --k) {
                        Square *end = list_get(ends, k);
                        
                        if (end->top > dropped->bottom || end->bottom < dropped->top) {
                            // This rect completely outside dropped active rect
                            continue;
                        }
                        
                        // This rect overlaps with dropped rect, needs to be handled
                        list_add(temps, square_create(next_end->right + 1, end->right, end->top, end->bottom));
                    }
                
                    destroy(dropped);
                }
                
                // Merge continuing rects where needed
                int t_index = (int)list_count(temps) - 1;
                while (t_index >= 0) {
                    int temps_count = (int)list_count(temps);
                    Square *continuing = list_get(temps, t_index);
                    bool continuing_is_active = true;
                    
                    int top = continuing->top;
                    int bottom = continuing->bottom;
                    for (int32_t k = (int32_t)temps_count - 1; k >= 0; --k) {
                        Square *temp = list_get(temps, k);
                        if (temp == continuing) {
                            continue;
                        }
                        
                        if (temp->top > continuing->bottom || temp->bottom < continuing->top) {
                            // No overlap
                            continue;
                        }
                        
                        if (continuing->top > temp->top && continuing->bottom < temp->bottom) {
                            continuing_is_active = false;
                            // continuing is completely contained by this rect
                            break;
                        }
                        
                        if (temp->top < top) {
                            top = temp->top;
                        }
                        if (temp->bottom > bottom) {
                            bottom = temp->bottom;
                        }
                                    
                        list_drop_index(temps, k);
                        if (k < t_index) {
                            --t_index;
                        }
                    }
                    
                    if (continuing_is_active) {
                        list_add(temps, square_create(next_end->right + 1, continuing->right, top, bottom));
                    }
                    list_drop_item(temps, continuing);
                    
                    --t_index;
                }
                
                for (int k = (int)list_count(temps) - 1; k >= 0; --k) {
                    list_add(actives, list_drop_index(temps, k));
                }
                
                continue;
            }
        }
        
        const size_t ends_count = list_count(ends);
        size_t k;
        for (k = 0; k < ends_count; ++k) {
            Square *end = list_get(ends, k);
            if (end->right > sq->right) {
                break;
            }
        }
        list_insert(ends, sq, k);
        
        const size_t actives_count = list_count(actives);
        int top = sq->top;
        int bottom = sq->bottom;
        bool sq_is_active = true;
        for (int32_t k = (int32_t)actives_count - 1; k >= 0; --k) {
            Square *active = list_get(actives, k);
            
            if (active->top > sq->bottom || active->bottom < sq->top) {
                continue;
            }
            
            if (sq->top >= active->top && sq->bottom <= active->bottom) {
                sq_is_active = false;
                break;
            }
            
            if (active->top < top) {
                top = active->top;
            }
            if (active->bottom > bottom) {
                bottom = active->bottom;
            }
                        
            list_add(result, square_create(active->left, sq->left - 1, active->top, active->bottom));
            list_drop_index(actives, k);
            destroy(active);
        }
        
        if (sq_is_active) {
            list_add(actives, square_create(sq->left, sq->right, top, bottom));
        }
        
        ++i;
    }
    
    count = list_count(actives);

    destroy(actives);
    destroy(temps);
    destroy(ends);
    
    for (int i = 0; i < (int)list_count(result); ++i) {
        Square *sq = (Square *)list_get(result, i);
        LOG("Square l: %d r: %d t: %d b: %d", sq->left, sq->right, sq->top, sq->bottom);
    }
}

BaseType RenderContextType = { "RenderContext", &render_context_destroy, &render_context_describe };
