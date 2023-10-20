#include "image_object_render.h"
#include "transforms.h"
#include <math.h>

void image_object_render(Image *image, GameObject *obj, RenderOptions render_options, DrawMode draw_mode, RenderContext *ctx)
{
    Size2D size_nb = (Size2D){ image->original.width, image->original.height };
    Float anchor_x = obj->anchor.x * size_nb.width;
    Float anchor_y = obj->anchor.y * size_nb.height;

    AffineTransform pos = af_identity();
    
    if (draw_mode == drawmode_default) {
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
        
        context_render_rect_image(ctx,
                                  image,
                                  (Vector2DInt){ (int32_t)floorf(pos.i13 - anchor_x), (int32_t)floorf(pos.i23 - anchor_y) },
                                  render_options
                                  );
    } else if (draw_mode == drawmode_scale) {
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
        
        Vector2D scale_measure_x = vec(1.f, 0.f);
        Vector2D scale_measure_y = vec(0.f, 1.f);
        Vector2D scale_origin = vec_zero();
        Vector2D scale_vector_x = af_vec_multiply(pos, scale_measure_x);
        Vector2D scale_vector_y = af_vec_multiply(pos, scale_measure_y);
        Vector2D origin_vector = af_vec_multiply(pos, scale_origin);
        
        Vector2D scale = vec(vec_length(vec_vec_subtract(scale_vector_x, origin_vector)), vec_length(vec_vec_subtract(scale_vector_y, origin_vector)));
        
        Float anchor_x_translate = -(anchor_x * scale.x);
        Float anchor_y_translate = -(anchor_y * scale.y);

        context_render_scale_image(ctx,
                                   image,
                                   (Vector2DInt){ (int32_t)floorf(pos.i13 + anchor_x_translate), (int32_t)floorf(pos.i23 + anchor_y_translate) },
                                   scale,
                                   render_options
                                   );
    } else if (draw_mode == drawmode_rotate) {
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
                
        context_render_rotate_image(ctx,
                                    image,
                                    (Vector2DInt){ (int32_t)floorf(pos.i13) - anchor_x, (int32_t)floorf(pos.i23) - anchor_y },
                                    go_rotation_from_root(obj),
                                    vec(anchor_x, anchor_y),
                                    render_options
                                    );
    } else if (draw_mode == drawmode_rotate_and_scale) {
        pos = af_translate(pos, (Vector2D){ -anchor_x, -anchor_y });
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
        
        ctx->render_transform = pos;

        context_render(ctx,
                       image,
                       render_options
                       );
    }
}
