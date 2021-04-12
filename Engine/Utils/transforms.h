#ifndef transforms_h
#define transforms_h

#include "types.h"
#include "number.h"

Vector2D vec_vec_add(Vector2D, Vector2D);
Vector2D vec_vec_subtract(Vector2D, Vector2D);
Vector2D vec_scale(Vector2D v, Number n);
Vector2D vec_f_scale(Vector2D, Float);
Vector2D vec_f_lerp(Vector2D, Vector2D, Float);
Vector2D vec_inverse(Vector2D);

Vector2D af_vec_multiply(AffineTransform, Vector2D);
AffineTransform af_af_multiply(AffineTransform, AffineTransform);

AffineTransform af_translate(AffineTransform a, Vector2D v);
AffineTransform af_scale(AffineTransform a, Vector2D v);
AffineTransform af_rotate(AffineTransform a, Number rad);

AffineTransform af_inverse(AffineTransform a);

AffineTransformFloat faf_faf_multiply(AffineTransformFloat, AffineTransformFloat);

AffineTransformFloat faf_translate(AffineTransformFloat a, Vector2DFloat v);
AffineTransformFloat faf_scale(AffineTransformFloat a, Vector2DFloat v);
AffineTransformFloat faf_rotate(AffineTransformFloat a, Float rad);

AffineTransformFloat faf_inverse(AffineTransformFloat a);

AffineTransformFloat faf_identity(void);

AffineTransformFloat af_to_faf(AffineTransform);

Vector2D vec(Number x, Number y);

Vector2D vec_unit(void);
Vector2D vec_zero(void);
AffineTransform af_identity(void);
Number vec_length(Vector2D v);

Direction dir_opposite(Direction d);
bool dir_positive(Direction d);
bool dir_horizontal(Direction d);

#endif /* transforms_h */
