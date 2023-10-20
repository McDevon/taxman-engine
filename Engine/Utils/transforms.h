#ifndef transforms_h
#define transforms_h

#include "types.h"

Vector2D vec(Float x, Float y);
Vector2D vec_angle(Float angle);

Vector2D vec_zero(void);
Float vec_length(Vector2D v);
Float vec_length_sq(Vector2D v);

Vector2D vec_vec_add(Vector2D, Vector2D);
Vector2D vec_vec_subtract(Vector2D, Vector2D);
Vector2D vec_scale(Vector2D v, Float n);
Vector2D vec_scale_to_length(Vector2D v, Float length);
Vector2D vec_normalize(Vector2D v);
Vector2D vec_lerp(Vector2D, Vector2D, Float);
Vector2D vec_inverse(Vector2D);

Vector2D vec_round(Vector2D);
Vector2D vec_floor(Vector2D);
Vector2D vec_ceil(Vector2D);

Vector2D af_vec_multiply(AffineTransform, Vector2D);
AffineTransform af_af_multiply(AffineTransform, AffineTransform);

AffineTransform af_translate(AffineTransform a, Vector2D v);
AffineTransform af_scale(AffineTransform a, Vector2D v);
AffineTransform af_rotate(AffineTransform a, Float rad);

AffineTransform af_inverse(AffineTransform a);

AffineTransform af_identity(void);

Direction dir_opposite(Direction d);
bool dir_positive(Direction d);
bool dir_horizontal(Direction d);

#endif /* transforms_h */
