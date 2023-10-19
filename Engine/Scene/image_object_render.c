#include "image_object_render.h"
#include "transforms.h"

void image_object_render(Image *image, GameObject *obj, RenderOptions render_options, DrawMode draw_mode, RenderContext *ctx)
{
    Size2D size_nb = (Size2D){nb_from_int(image->original.width), nb_from_int(image->original.height)};
    Number anchor_x = nb_mul(obj->anchor.x, size_nb.width);
    Number anchor_y = nb_mul(obj->anchor.y, size_nb.height);

    AffineTransform pos = af_identity();
    
    if (draw_mode == drawmode_default) {
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
        
        context_render_rect_image(ctx,
                                  image,
                                  (Vector2DInt){ nb_to_int(nb_floor(pos.i13 - anchor_x)), nb_to_int(nb_floor(pos.i23 - anchor_y)) },
                                  render_options
                                  );
    } else if (draw_mode == drawmode_scale) {
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
                
        Vector2D scale_measure_x = vec(nb_one, nb_zero);
        Vector2D scale_measure_y = vec(nb_zero, nb_one);
        Vector2D scale_origin = vec_zero();
        Vector2D scale_vector_x = af_vec_multiply(pos, scale_measure_x);
        Vector2D scale_vector_y = af_vec_multiply(pos, scale_measure_y);
        Vector2D origin_vector = af_vec_multiply(pos, scale_origin);
        
        Vector2D scale = vec(vec_length(vec_vec_subtract(scale_vector_x, origin_vector)), vec_length(vec_vec_subtract(scale_vector_y, origin_vector)));
        
        Number anchor_x_translate = -nb_mul(anchor_x, scale.x);
        Number anchor_y_translate = -nb_mul(anchor_y, scale.y);

        context_render_scale_image(ctx,
                                   image,
                                   (Vector2DInt){ nb_to_int(nb_floor(pos.i13 + anchor_x_translate)), nb_to_int(nb_floor(pos.i23 + anchor_y_translate)) },
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
                                    (Vector2DInt){ nb_to_int(nb_floor(pos.i13) - anchor_x), nb_to_int(nb_floor(pos.i23) - anchor_y) },
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
