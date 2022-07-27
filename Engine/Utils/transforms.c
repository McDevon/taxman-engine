#include "transforms.h"
#include <math.h>

inline Vector2D vec_vec_add(Vector2D a, Vector2D b)
{
    return (Vector2D) {
        a.x + b.x,
        a.y + b.y
    };
}

inline Vector2D vec_vec_subtract(Vector2D a, Vector2D b)
{
    return (Vector2D) {
        a.x - b.x,
        a.y - b.y
    };
}

inline Vector2D vec_scale(Vector2D v, Number n)
{
    return (Vector2D) {
        nb_mul(v.x, n),
        nb_mul(v.y, n)
    };
}

inline Vector2D vec_scale_to_length(Vector2D v, Number length)
{
    Number curr_length = vec_length(v);
    return (Vector2D) {
        nb_div(nb_mul(v.x, length), curr_length),
        nb_div(nb_mul(v.y, length), curr_length)
    };
}

inline Vector2D vec_normalize(Vector2D v)
{
    Number curr_length = vec_length(v);
    return (Vector2D) {
        nb_div(v.x, curr_length),
        nb_div(v.y, curr_length)
    };
}

inline Vector2D vec_f_scale(Vector2D v, Float f)
{
    return (Vector2D) {
        (Number)(v.x * f),
        (Number)(v.y * f)
    };
}

inline Vector2D vec_f_lerp(Vector2D a, Vector2D b, Float f)
{
    return (Vector2D) {
        (Number)(a.x * (1.f - f) + b.x * f),
        (Number)(a.y * (1.f - f) + b.y * f)
    };
}

inline Vector2D vec_inverse(Vector2D v)
{
    return (Vector2D) {
        -v.x,
        -v.y
    };
}

inline Vector2D vec_round(Vector2D v)
{
    return (Vector2D) {
        nb_round(v.x),
        nb_round(v.y)
    };
}

inline Vector2D vec(Number x, Number y)
{
    return (Vector2D){ x, y };
}

inline Vector2D vec_angle(Number angle)
{
    return (Vector2D){ nb_cos(angle), nb_sin(angle) };
}

inline Vector2D vec_zero()
{
    return (Vector2D){ nb_zero, nb_zero };
}

inline Number vec_length(Vector2D v)
{
    return nb_sqrt(nb_mul(v.x, v.x) + nb_mul(v.y, v.y));
}

inline Number vec_length_sq(Vector2D v)
{
    return nb_mul(v.x, v.x) + nb_mul(v.y, v.y);
}

Vector2D af_vec_multiply(AffineTransform trf, Vector2D pos)
{
    /*
     [a11 a12 a13] [x]
     [a21 a22 a23] [y]
     [0   0   1  ] [1]
     */
    
    return (Vector2D) {
        nb_mul(pos.x, trf.i11) + nb_mul(pos.y, trf.i12) + trf.i13,
        nb_mul(pos.x, trf.i21) + nb_mul(pos.y, trf.i22) + trf.i23
    };
}

AffineTransform af_af_multiply(AffineTransform a, AffineTransform b)
{
    /*
     [a11 a12 a13] [b11 b12 b13]
     [a21 a22 a23] [b21 b22 b23]
     [0   0   1  ] [0   0   1  ]
     */
    
    return (AffineTransform) {
        nb_mul(a.i11, b.i11) + nb_mul(a.i12, b.i21),
        nb_mul(a.i11, b.i12) + nb_mul(a.i12, b.i22),
        nb_mul(a.i11, b.i13) + nb_mul(a.i12, b.i23) + a.i13,

        nb_mul(a.i21, b.i11) + nb_mul(a.i22, b.i21),
        nb_mul(a.i21, b.i12) + nb_mul(a.i22, b.i22),
        nb_mul(a.i21, b.i13) + nb_mul(a.i22, b.i23) + a.i23
    };
}

AffineTransform af_translate(AffineTransform a, Vector2D v)
{
    /*
     [1   0   x  ] [a11 a12 a13]
     [0   1   y  ] [a21 a22 a23]
     [0   0   1  ] [0   0   1  ]
     */

    return (AffineTransform) {
        a.i11,
        a.i12,
        a.i13 + v.x,

        a.i21,
        a.i22,
        a.i23 + v.y
    };
}

AffineTransform af_scale(AffineTransform a, Vector2D v)
{
    /*
    [x   0   0  ] [a11 a12 a13]
    [0   y   0  ] [a21 a22 a23]
    [0   0   1  ] [0   0   1  ]
    */
    
    return (AffineTransform) {
        nb_mul(v.x, a.i11),
        nb_mul(v.x, a.i12),
        nb_mul(v.x, a.i13),

        nb_mul(v.y, a.i21),
        nb_mul(v.y, a.i22),
        nb_mul(v.y, a.i23)
    };
}

AffineTransform af_rotate(AffineTransform a, Number rad)
{
    /*
    [cos(rad) -sin(rad) 0  ] [a11 a12 a13]
    [sin(rad) cos(rad)  0  ] [a21 a22 a23]
    [0        0         1  ] [0   0   1  ]
    */
    
    Number cos = nb_cos(rad);
    Number sin = nb_sin(rad);

    return (AffineTransform) {
        nb_mul(cos, a.i11) + nb_mul(-sin, a.i21),
        nb_mul(cos, a.i12) + nb_mul(-sin, a.i22),
        nb_mul(cos, a.i13) + nb_mul(-sin, a.i23),

        nb_mul(sin, a.i11) + nb_mul(cos, a.i21),
        nb_mul(sin, a.i12) + nb_mul(cos, a.i22),
        nb_mul(sin, a.i13) + nb_mul(cos, a.i23)
    };
}

AffineTransform af_inverse(AffineTransform a)
{
    /*
     [a11 a12 a13]
     [a21 a22 a23]
     [0   0   1  ]
     */
    
    Number det = nb_mul(a.i11, a.i22) - nb_mul(a.i12, a.i21);
    
    AffineTransform res;
    res.i11 = nb_div(a.i22, det);
    res.i12 = -nb_div(a.i12, det);
    
    res.i21 = -nb_div(a.i21, det);
    res.i22 = nb_div(a.i11, det);
    
    res.i13 = -nb_mul(res.i11, a.i13) - nb_mul(res.i12, a.i23);
    res.i23 = -nb_mul(res.i21, a.i13) - nb_mul(res.i22, a.i23);

    return res;
}

inline AffineTransform af_identity()
{
    /*
     [1   0   0  ]
     [0   1   0  ]
     [0   0   1  ]
     */

    return (AffineTransform) {
        nb_one, 0,       0,
        0,      nb_one,  0
    };
}

AffineTransformFloat faf_faf_multiply(AffineTransformFloat a, AffineTransformFloat b)
{
    /*
     [a11 a12 a13] [b11 b12 b13]
     [a21 a22 a23] [b21 b22 b23]
     [0   0   1  ] [0   0   1  ]
     */
    
    return (AffineTransformFloat) {
        a.i11 * b.i11 + a.i12 * b.i21,
        a.i11 * b.i12 + a.i12 * b.i22,
        a.i11 * b.i13 + a.i12 * b.i23 + a.i13,

        a.i21 * b.i11 + a.i22 * b.i21,
        a.i21 * b.i12 + a.i22 * b.i22,
        a.i21 * b.i13 + a.i22 * b.i23 + a.i23
    };
}

AffineTransformFloat faf_translate(AffineTransformFloat a, Vector2DFloat v)
{
    /*
     [1   0   x  ] [a11 a12 a13]
     [0   1   y  ] [a21 a22 a23]
     [0   0   1  ] [0   0   1  ]
     */

    return (AffineTransformFloat) {
        a.i11,
        a.i12,
        a.i13 + v.x,

        a.i21,
        a.i22,
        a.i23 + v.y
    };
}

AffineTransformFloat faf_scale(AffineTransformFloat a, Vector2DFloat v)
{
    /*
    [x   0   0  ] [a11 a12 a13]
    [0   y   0  ] [a21 a22 a23]
    [0   0   1  ] [0   0   1  ]
    */
    
    return (AffineTransformFloat) {
        v.x * a.i11,
        v.x * a.i12,
        v.x * a.i13,

        v.y * a.i21,
        v.y * a.i22,
        v.y * a.i23
    };
}

AffineTransformFloat faf_rotate(AffineTransformFloat a, Float rad)
{
    /*
    [cos(rad) -sin(rad) 0  ] [a11 a12 a13]
    [sin(rad) cos(rad)  0  ] [a21 a22 a23]
    [0        0         1  ] [0   0   1  ]
    */
    
    Float cos = cosf(rad);
    Float sin = sinf(rad);

    return (AffineTransformFloat) {
        cos * a.i11 - sin * a.i21,
        cos * a.i12 - sin * a.i22,
        cos * a.i13 - sin * a.i23,

        sin * a.i11 + cos * a.i21,
        sin * a.i12 + cos * a.i22,
        sin * a.i13 + cos * a.i23
    };
}

AffineTransformFloat faf_inverse(AffineTransformFloat a)
{
    /*
     [a11 a12 a13]
     [a21 a22 a23]
     [0   0   1  ]
     */
    
    Float det = a.i11 * a.i22 - a.i12 * a.i21;
    
    AffineTransformFloat res;
    res.i11 = a.i22 / det;
    res.i12 = -a.i12 / det;
    
    res.i21 = -a.i21 / det;
    res.i22 = a.i11 / det;
    
    res.i13 = -res.i11 * a.i13 - res.i12 * a.i23;
    res.i23 = -res.i21 * a.i13 - res.i22 * a.i23;

    return res;
}

inline AffineTransformFloat faf_identity()
{
    /*
     [1   0   0  ]
     [0   1   0  ]
     [0   0   1  ]
     */

    return (AffineTransformFloat) {
        1.f, 0,       0,
        0,      1.f,  0
    };
}

AffineTransformFloat af_to_faf(AffineTransform a)
{
    return (AffineTransformFloat) {
        nb_to_float(a.i11), nb_to_float(a.i12), nb_to_float(a.i13),
        nb_to_float(a.i21), nb_to_float(a.i22), nb_to_float(a.i23)
    };
}

inline Direction dir_opposite(Direction d)
{
    if (d == dir_left) {
        return dir_right;
    } else if (d == dir_right) {
        return dir_left;
    } else if (d == dir_up) {
        return dir_down;
    } else {
        return dir_up;
    }
}

inline bool dir_positive(Direction d)
{
    return d == dir_right || d == dir_down;
}

inline bool dir_horizontal(Direction d)
{
    return d == dir_right || d == dir_left;
}
