#include "render_context.h"
#include "render_rect.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include "engine_log.h"
#include "transforms.h"
#include <string.h>

void render_context_destroy(void *value)
{
    RenderContext *self = (RenderContext *)value;
    destroy(self->render_camera);
    self->render_camera = NULL;
    
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
    RenderRect *rect_a = *(RenderRect**)a;
    RenderRect *rect_b = *(RenderRect**)b;
    
    int diff = rect_a->top - rect_b->top;
    
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
        RenderRect *next_begin = list_get(rendered_rects, i);
        
        size_t end_count = list_count(ends);
        RenderRect *next_end = (RenderRect *)list_get(ends, end_count - 1);
        
        // Line sweep to handle either next beginning of rect or next ending of rect
        if (next_end && (!next_begin || next_end->bottom < next_begin->top)) {
            RenderRect *dropped_active = NULL;
            
            // Find the active rect which ends along with this ending rect, if any
            const size_t actives_count = list_count(actives);
            for (int32_t k = (int32_t)actives_count - 1; k >= 0; --k) {
                RenderRect *active = list_get(actives, k);
                
                if (active->left > next_end->right || active->right < next_end->left) {
                    // Active does not overlap with ending rect
                    continue;
                }
                
                if (next_end->left > active->left && next_end->right < active->right && next_end->bottom < active->bottom) {
                    // Ending rect is completely contained in this active
                    break;
                }

                dropped_active = list_drop_index(actives, k);
                break;
            }
            
            list_drop_index(ends, end_count - 1);

            if (!dropped_active) {
                continue;
            }
            
            // Merge remaining ending rects into actives
            end_count = list_count(ends);
            for (int32_t k = 0; k < end_count; ++k) {
                RenderRect *end = list_get(ends, k);
                
                if (end->left > dropped_active->right || end->right < dropped_active->left) {
                    // This rect completely outside dropped active rect
                    continue;
                }
                
                // Find if there is a temp overlapping this one
                bool has_overlapping_temp = false;
                const size_t temp_count = list_count(merged);
                RenderRect *first_contact = NULL;
                for (int t = (int)temp_count - 1; t >= 0; --t) {
                    RenderRect *mergable = list_get(merged, t);
                    if (end->left > mergable->right || end->right < mergable->left) {
                        // This rect completely outside temp
                        continue;
                    }
                    has_overlapping_temp = true;
                    
                    if (first_contact) {
                        // This end has already met another mergable, merge to it
                        if (mergable->left < first_contact->left) {
                            first_contact->left = mergable->left;
                        }
                        if (mergable->right > first_contact->right) {
                            first_contact->right = mergable->right;
                        }
                        if (mergable->bottom < first_contact->bottom) {
                            first_contact->bottom = mergable->bottom;
                        }
                        list_drop_index(merged, t);
                        destroy(mergable);
                        continue;
                    }
                    
                    // Overlaps with this mergable, adjust mergable
                    if (end->left < mergable->left) {
                        mergable->left = end->left;
                    }
                    if (end->right > mergable->right) {
                        mergable->right = end->right;
                    }
                    if (end->bottom < mergable->bottom) {
                        mergable->bottom = end->bottom;
                    }
                    first_contact = mergable;
                }
                
                if (!has_overlapping_temp && end->bottom >= next_end->bottom + 1) {
                    // Does not overlap with existings mergables, create new
                    list_add(merged, rrect_create(end->left, end->right, next_end->bottom + 1, end->bottom));
                }
            }
        
            bool keep_dropped = false;
            // Add merged rects to actives
            for (int k = (int)list_count(merged) - 1; k >= 0; --k) {
                RenderRect *active_candidate = list_drop_index(merged, k);
                
                // This candidate would continue as exactly the same width rect as dropped, so continue dropped instead
                if (dropped_active->left == active_candidate->left && dropped_active->right == active_candidate->right && active_candidate->bottom > dropped_active->bottom) {
                    keep_dropped = true;
                    dropped_active->bottom = active_candidate->bottom;
                    list_add(actives, dropped_active);
                    destroy(active_candidate);
                } else {
                    list_add(actives, active_candidate);
                }
            }
            
            if (!keep_dropped) {
                // Ended rect overlaps with active but not completely contained
                if (dropped_active->top <= next_end->bottom) {
                    list_add(result, rrect_create(dropped_active->left, dropped_active->right, dropped_active->top, next_end->bottom));
                }
                destroy(dropped_active);
            }
        } else {
            
            // Add beginning rect to the list of ending rects
            // Everything that has a beginning, has an end
            int k;
            for (k = (int)end_count - 1; k >= 0; --k) {
                RenderRect *end = list_get(ends, k);
                if (end->bottom > next_begin->bottom) {
                    break;
                }
            }
            list_insert(ends, next_begin, k + 1);
            
            // Compare to existing actives, and finish any overlapping ones if not contained
            const size_t actives_count = list_count(actives);
            int left = next_begin->left;
            int right = next_begin->right;
            int bottom = next_begin->bottom;
            bool rect_is_active = true;
            for (int32_t k = (int32_t)actives_count - 1; k >= 0; --k) {
                RenderRect *active = list_get(actives, k);
                
                if (active->left > next_begin->right || active->right < next_begin->left) {
                    continue;
                }
                
                if (next_begin->left >= active->left && next_begin->right <= active->right) {
                    rect_is_active = false;
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
                          
                if (active->top <= next_begin->top - 1) {
                    list_add(result, rrect_create(active->left, active->right, active->top, next_begin->top - 1));
                }

                destroy(list_drop_index(actives, k));
            }
            
            if (rect_is_active) {
                list_add(actives, rrect_create(left, right, next_begin->top, bottom));
            }
            
            ++i;
        }
    }
    
    if (!ctx) {
        destroy(actives);
        destroy(merged);
        destroy(ends);
    }
}

BaseType RenderContextType = { "RenderContext", &render_context_destroy, &render_context_describe };

RenderContext *render_context_create(ImageData *target_buffer, bool background_enabled)
{
    RenderContext *ctx = platform_calloc(1, sizeof(RenderContext));
    ctx->w_type = &RenderContextType;
    ctx->w_target_buffer = target_buffer;
    ctx->render_transform = af_identity();
    ctx->render_camera = render_camera_create(target_buffer->size);
    ctx->is_screen_context = false;
    
    ctx->background_enabled = background_enabled;
    if (background_enabled) {
        ctx->rendered_rects = list_create();
        ctx->rect_pool = list_create();
        ctx->active_rects = list_create();
        ctx->merge_rects = list_create_with_weak_references();
        ctx->end_rects = list_create_with_weak_references();
    }
    
    return ctx;
}
