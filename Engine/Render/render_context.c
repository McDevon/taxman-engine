#include "render_context.h"
#include "render_rect.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "engine_log.h"
#include <string.h>

void render_context_destroy(void *value)
{
    RenderContext *self = (RenderContext *)value;
    if (self->rendered_rects) {
        destroy(self->rendered_rects);
        self->rendered_rects = NULL;
    }
    if (self->rect_pool) {
        destroy(self->rect_pool);
        self->rect_pool = NULL;
    }
    if (self->active_rects) {
        destroy(self->active_rects);
        self->active_rects = NULL;
    }
    if (self->end_rects) {
        destroy(self->end_rects);
        self->end_rects = NULL;
    }
    if (self->merge_rects) {
        destroy(self->merge_rects);
        self->merge_rects = NULL;
    }
}

char *render_context_describe(void *value)
{
    return platform_strdup("[]");
}

int context_rect_compare_top_edge(const void *a, const void *b)
{
    RenderRect *sq_a = *(RenderRect**)a;
    RenderRect *sq_b = *(RenderRect**)b;
    
    int diff = sq_a->top - sq_b->top;
    
    if (diff < nb_zero) {
        return list_sorted_ascending;
    } else if (diff > nb_zero) {
        return list_sorted_descending;
    } else {
        return list_sorted_same;
    }
}

void context_background_rendered(RenderContext *self)
{
    size_t rect_count = list_count(self->rendered_rects);
    for (int i = (int)rect_count - 1; i >= 0 ; --i) {
        list_add(self->rect_pool, list_drop_index(self->rendered_rects, i));
    }
}

RenderRect *context_get_render_rect(RenderContext *ctx, int left, int right, int top, int bottom)
{
    RenderRect *rect = NULL;
    size_t pool_count;
    if (ctx && (pool_count = list_count(ctx->rect_pool) > 0)) {
        rect = list_drop_index(ctx->rect_pool, pool_count - 1);
        rect->left = left;
        rect->right = right;
        rect->top = top;
        rect->bottom = bottom;
    } else {
        rect = rrect_create(left, right, top, bottom);
    }
    
    return rect;
}

void context_release_render_rect(RenderContext *ctx, RenderRect *rect)
{
    if (ctx) {
        list_add(ctx->rect_pool, rect);
    } else {
        destroy(rect);
    }
}

void context_rect_rendered(RenderContext *self, int left, int right, int top, int bottom)
{
    if (!self->background_enabled) {
        return;
    }
    
    list_add(self->rendered_rects, context_get_render_rect(self, left, right, top, bottom));
}

void clean_union_of_rendered_rects(ArrayList *rendered_rects, ArrayList *result)
{
    context_clean_union_of_rendered_rects(NULL, rendered_rects, result);
}

void context_clean_union_of_rendered_rects(RenderContext *ctx, ArrayList *rendered_rects, ArrayList *result)
{
    if (!rendered_rects || !result) {
        LOG_ERROR("clean_union_of_rendered_rects: received null parameter");
        return;
    }
    
    if (list_count(result) != 0) {
        LOG_ERROR("clean_union_of_rendered_rects: result array not empty");
        return;
    }
    
    size_t count = list_count(rendered_rects);
    
    ArrayList *actives;
    ArrayList *merged;
    ArrayList *ends;
    
    if (ctx) {
        actives = ctx->active_rects;
        merged = ctx->merge_rects;
        ends = ctx->end_rects;
    } else {
        actives = list_create();
        merged = list_create_with_weak_references();
        ends = list_create_with_weak_references();
    }

    list_sort(rendered_rects, &context_rect_compare_top_edge);
    
    size_t i = 0;
    while (i < count || list_count(ends)) {
        RenderRect *sq = list_count(rendered_rects) ? list_get(rendered_rects, i) : NULL;
        
        size_t end_count = list_count(ends);
        RenderRect *next_end = (RenderRect *)list_get(ends, end_count - 1);
        if (next_end && (!sq || next_end->bottom < sq->top)) {
            // Square ended
            RenderRect *dropped = NULL;
            
            const size_t actives_count = list_count(actives);
            for (int32_t k = (int32_t)actives_count - 1; k >= 0; --k) {
                RenderRect *active = list_get(actives, k);
                
                if (active->left > next_end->right || active->right < next_end->left) {
                    // Active does not overlap with ended square
                    continue;
                }
                
                if (next_end->left > active->left && next_end->right < active->right && next_end->bottom < active->bottom) {
                    // Ended square completely contained in this active
                    break;
                }

                list_drop_index(actives, k);
                dropped = active;
                break;
            }
            
            list_drop_index(ends, end_count - 1);

            if (!dropped) {
                continue;
            }
            
            end_count = list_count(ends);
            for (int32_t k = 0; k < end_count; ++k) {
                RenderRect *end = list_get(ends, k);
                
                if (end->left > dropped->right || end->right < dropped->left) {
                    // This rect completely outside dropped active rect
                    continue;
                }
                
                // Find if there is a temp overlapping this one
                bool has_overlapping_temp = false;
                const size_t temp_count = list_count(merged);
                RenderRect *first_contact = NULL;
                for (int t = (int)temp_count - 1; t >= 0; --t) {
                    RenderRect *temp = list_get(merged, t);
                    if (end->left > temp->right || end->right < temp->left) {
                        // This rect completely outside temp
                        continue;
                    }
                    has_overlapping_temp = true;
                    
                    if (first_contact) {
                        // This end has already met another temp, join to it
                        if (temp->left < first_contact->left) {
                            first_contact->left = temp->left;
                        }
                        if (temp->right > first_contact->right) {
                            first_contact->right = temp->right;
                        }
                        if (temp->bottom < first_contact->bottom) {
                            first_contact->bottom = temp->bottom;
                        }
                        list_drop_index(merged, t);
                        context_release_render_rect(ctx, temp);
                        continue;
                    }
                    
                    // Overlaps with this temp, adjust temp
                    if (end->left < temp->left) {
                        temp->left = end->left;
                    }
                    if (end->right > temp->right) {
                        temp->right = end->right;
                    }
                    if (end->bottom < temp->bottom) {
                        temp->bottom = end->bottom;
                    }
                    first_contact = temp;
                }
                
                if (!has_overlapping_temp && end->bottom >= next_end->bottom + 1) {
                    // Does not overlap with existings temps, create new
                    list_add(merged, context_get_render_rect(ctx, end->left, end->right, next_end->bottom + 1, end->bottom));
                }
            }
        
            bool keep_dropped = false;
            // Add temps to actives
            for (int k = (int)list_count(merged) - 1; k >= 0; --k) {
                RenderRect *temp = list_drop_index(merged, k);
                if (dropped->left == temp->left && dropped->right == temp->right && temp->bottom > dropped->bottom) {
                    keep_dropped = true;
                    dropped->bottom = temp->bottom;
                    list_add(actives, dropped);
                    context_release_render_rect(ctx, temp);
                } else {
                    list_add(actives, temp);
                }
            }
            
            if (!keep_dropped) {
                // Ended square overlaps with active but not completely contained
                if (dropped->top <= next_end->bottom) {
                    list_add(result, context_get_render_rect(ctx, dropped->left, dropped->right, dropped->top, next_end->bottom));
                }
                context_release_render_rect(ctx, dropped);
            }
        } else {
            int k;
            for (k = (int)end_count - 1; k >= 0; --k) {
                RenderRect *end = list_get(ends, k);
                if (end->bottom > sq->bottom) {
                    break;
                }
            }
            list_insert(ends, sq, k + 1);
            
            const size_t actives_count = list_count(actives);
            int left = sq->left;
            int right = sq->right;
            int bottom = sq->bottom;
            bool sq_is_active = true;
            for (int32_t k = (int32_t)actives_count - 1; k >= 0; --k) {
                RenderRect *active = list_get(actives, k);
                
                if (active->left > sq->right || active->right < sq->left) {
                    continue;
                }
                
                if (sq->left >= active->left && sq->right <= active->right) {
                    sq_is_active = false;
                    break;
                }
                
                if (active->left < left) {
                    left = active->left;
                }
                if (active->right > right) {
                    right = active->right;
                }
                if (active->bottom < bottom) {
                    bottom = active->bottom;
                }
                          
                if (active->top <= sq->top - 1) {
                    list_add(result, context_get_render_rect(ctx, active->left, active->right, active->top, sq->top - 1));
                }
                list_drop_index(actives, k);
                context_release_render_rect(ctx, active);
            }
            
            if (sq_is_active) {
                list_add(actives, context_get_render_rect(ctx, left, right, sq->top, bottom));
            }
            
            ++i;
        }
    }
    
    if (list_count(actives) || list_count(merged) || list_count(ends)) {
        LOG_ERROR("clean_union_of_rendered_rects LIST NOT EMPTY WHEN FINISHED");
    }
    
    if (!ctx) {
        destroy(actives);
        destroy(merged);
        destroy(ends);
    }
}

BaseType RenderContextType = { "RenderContext", &render_context_destroy, &render_context_describe };
