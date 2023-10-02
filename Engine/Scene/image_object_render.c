#include "image_object_render.h"
#include "transforms.h"

void image_object_render(Image *image, GameObject *obj, RenderOptions render_options, DrawMode draw_mode, RenderContext *ctx)
{
    Size2D size = (Size2D){nb_from_int(image->original.width), nb_from_int(image->original.height)};
    Number anchor_x = nb_mul(obj->anchor.x, size.width);
    Number anchor_y = nb_mul(obj->anchor.y, size.height);

    Number anchor_x_translate = -nb_mul(anchor_x, obj->scale.x);
    Number anchor_y_translate = -nb_mul(anchor_y, obj->scale.y);

    AffineTransform pos = af_identity();
    
    if (draw_mode == drawmode_default) {
        pos = af_scale(pos, obj->scale);
        pos = af_translate(pos, (Vector2D){ anchor_x_translate, anchor_y_translate });
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);

        context_render_rect_image(ctx,
                                  image,
                                  (Vector2DInt){ nb_to_int(nb_floor(pos.i13)), nb_to_int(nb_floor(pos.i23)) },
                                  render_options
                                  );
    } else if (draw_mode == drawmode_scale) {
        pos = af_scale(pos, obj->scale);
        pos = af_translate(pos, (Vector2D){ anchor_x_translate, anchor_y_translate });
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
        
        AffineTransform scale_transform = af_af_multiply(pos, af_identity());
        
        context_render_scale_image(ctx,
                                   image,
                                   (Vector2DInt){ nb_to_int(nb_floor(pos.i13)), nb_to_int(nb_floor(pos.i23)) },
                                   vec(scale_transform.i11, scale_transform.i22),
                                   render_options
                                   );
    } else if (draw_mode == drawmode_rotate) {
        pos = af_scale(pos, obj->scale);
        pos = af_rotate(pos, obj->rotation);
        pos = af_translate(pos, obj->position);
        pos = af_af_multiply(ctx->render_transform, pos);
                
        context_render_rotate_image(ctx,
                                    image,
                                    (Vector2DInt){ nb_to_int(nb_floor(pos.i13) + anchor_x_translate), nb_to_int(nb_floor(pos.i23) + anchor_y_translate) },
                                    go_rotation_from_root(obj),
                                    vec(anchor_x, anchor_y),
                                    render_options
                                    );
    } else if (draw_mode == drawmode_rotate_and_scale) {
        pos = af_scale(pos, obj->scale);
        pos = af_translate(pos, (Vector2D){ anchor_x_translate, anchor_y_translate });
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
