#include "number.h"
#include "platform_adapter.h"
#include <string.h>
#include <stdlib.h>

#ifdef NUMBER_TYPE_FIXED_POINT

#include "number_lut.h"

#define NUMBER_BITS 32
#define DECIMAL_BITS 10

#define DECIMAL_MASK 0x3ff
#define FULL_MASK -1

const Number nb_max_value = INT_MAX;
const Number nb_min_value = INT_MIN;
const Number nb_zero = 0;
const Number nb_one = 1 << DECIMAL_BITS;
const Number nb_two = 1 << (DECIMAL_BITS + 1);
const Number nb_half = 1 << (DECIMAL_BITS - 1);
const Number nb_precision = 1;

const Number nb_pi = 3216;
const Number nb_pi_times_two = 6433;
const Number nb_pi_over_two = 1608;
const Number nb_pi_inv = 325;
const Number nb_pi_over_two_inv = 651;

#define nb_rad_to_deg 58670
#define nb_180 184320

static const int32_t LUT_SIZE = 1608;

static const Number ten_power_table[] = {
    1024,
    10240,
    102400,
    1024000,
    10240000,
    102400000,
    1024000000
};

Number nb_mul(Number v1, Number v2);
Number nb_div(Number v1, Number v2);

inline int32_t nb_sign(Number value)
{
    return value < 0 ? -1 : value > 0 ? 1 : 0;
}

inline Number nb_to_radians(Number value) {
    return nb_div(nb_mul(value, nb_pi), nb_180);
}

inline Number nb_to_degrees(Number value) {
    return nb_mul(value, nb_rad_to_deg);
}

inline Number nb_abs(Number value) {
    const int32_t mask = value >> (NUMBER_BITS - 1);
    return (value + mask) ^ mask;
}

inline Number nb_floor(Number value) {
    return value & (~DECIMAL_MASK);
}

inline Number nb_ceil(Number value) {
    return (value + nb_one - 1) & (~DECIMAL_MASK);
}

inline Number nb_round(Number value) {
    return (value + nb_half) & (~DECIMAL_MASK);
}

#pragma mark - Operators

inline Number nb_add(Number v1, Number v2) {
    return v1 + v2;
}

inline Number nb_sub(Number v1, Number v2) {
    return v1 - v2;
}

inline Number nb_mul(Number v1, Number v2) {
    return (int32_t)(((int64_t)v1 * v2) >> DECIMAL_BITS);
}

inline Number nb_div(Number v1, Number v2) {
    return (int32_t)(((int64_t)v1 << DECIMAL_BITS) / v2);
}

inline Number nb_mod(Number v1, Number v2) {
    return v1 % v2;
}

inline Number nb_negate(Number value) {
    return -value;
}

#pragma mark - Functions

Number nb_sqrt(Number value) {
    
    if (value < 0) {
        return -nb_one;
    }

    Number num = value;
    Number result = 0;

    Number bit = 1 << (NUMBER_BITS - 2);

    while (bit > num) {
        bit >>= 2;
    }

    for (int i = 0; i < 2; ++i) {
        while (bit != 0) {
            if (num >= result + bit) {
                num -= result + bit;
                result = (result >> 1) + bit;
            }
            else {
                result = result >> 1;
            }
            bit >>= 2;
        }

        if (i == 0) {
            if (num > (1 << (DECIMAL_BITS)) - 1) {

                num -= result;
                num = (num << (DECIMAL_BITS)) - nb_half;
                result = (result << (DECIMAL_BITS)) + nb_half;
            }
            else {
                num <<= (DECIMAL_BITS);
                result <<= (DECIMAL_BITS);
            }

            bit = 1 << (DECIMAL_BITS - 2);
        }
    }

    if (num > result) {
        ++result;
    }
    return result;
}

Number nb_sin(Number value) {
    
    Number angle = value;
    
    int32_t clamp2pi = angle % nb_pi_times_two;
    if (angle < 0) {
        clamp2pi += nb_pi_times_two;
    }
    
    uint8_t flip_v = clamp2pi >= nb_pi;
    int32_t clamp_pi = clamp2pi;
    
    while (clamp_pi >= nb_pi) {
        clamp_pi -= nb_pi;
    }
    
    uint8_t flip_h = clamp_pi >= nb_pi_over_two;
    
    int32_t clamp_pi_per_2 = clamp_pi;
    if (clamp_pi_per_2 >= nb_pi_over_two) {
        clamp_pi_per_2 -= nb_pi_over_two;
    }
    
    if (clamp_pi_per_2 >= LUT_SIZE) {
        clamp_pi_per_2 = LUT_SIZE - 1;
    }
    
    Number result = nb_lut_sin[flip_h ? LUT_SIZE - 1 - clamp_pi_per_2 : clamp_pi_per_2];
    return flip_v ? -result : result;
}

Number nb_cos(Number value) {
    Number sin_angle = value + (value > 0 ? -nb_pi - nb_pi_over_two : nb_pi_over_two);
    return nb_sin(sin_angle);
}

Number nb_tan(Number value) {
    int32_t clamp_pi = value % nb_pi;
    uint8_t flip = 0;
    if (clamp_pi < 0) {
        clamp_pi = -clamp_pi;
        flip = 1;
    }
    
    if (clamp_pi > nb_pi_over_two) {
        flip = !flip;
        clamp_pi = nb_pi_over_two - (clamp_pi - nb_pi_over_two);
    }
        
    if (clamp_pi >= LUT_SIZE) {
        clamp_pi = LUT_SIZE - 1;
    }
    
    Number result = nb_lut_tan[clamp_pi];
    return flip ? -result : result;
}

static const Number atan2_help = 0; //Fixed.fromString("0.28");

Number nb_atan2(Number y, Number x) {
    if (x == 0) {
        if (y > 0) {
            return nb_pi_over_two;
        }
        if (y == 0) {
            return nb_zero;
        }
        return -nb_pi_over_two;
    }
    Number atan;
    Number z = nb_div(y, x);

    Number divider = nb_one + (nb_mul(nb_mul(atan2_help, z), z));

    if (nb_abs(z) < nb_one) {
        atan = nb_div(z, divider);
        if (x < 0) {
            if (y < 0) {
                return atan - nb_pi;
            }
            return atan + nb_pi;
        }
    }
    else {
        atan = nb_pi_over_two - (nb_div(z, (nb_mul(z, z) + atan2_help)));
        if (y < 0) {
            return atan - nb_pi;
        }
    }
    return atan;
}

/*

 
 private static final int oneBitHighMask = MIN_VALUE;
 private static final int fourBitHighMask = oneBitHighMask >> 3;
 
 private static int leadingZeroes(int x) {
     int result = 0;
     while ((x & fourBitHighMask) == 0) { result += 4; x <<= 4; }
     while ((x & oneBitHighMask) == 0) { result += 1; x <<= 1; }
     return result;
 }
 
 */

Number nb_from_int(int32_t value) {
    return value * nb_one;
}

Number nb_from_long(int64_t value) {
    return (int32_t)value * nb_one;
}

Number nb_from_float(float value) {
    return (Number)(value * nb_one);
}

Number nb_from_double(double value) {
    return (Number)(value * nb_one);
}

Number nb_from_string_n(const char *value, const size_t length) {
    int comma = -1;
    for (int i = 0; i < length; ++i) {
        if (value[i] == '.' || value[i] == ',') {
            comma = i;
            break;
        }
    }
    
    if (comma > 0) {
        char left_str[length];
        char right_str[length];
        
        strncpy(left_str, value, comma);
        strncpy(right_str, value + comma + 1, length - comma - 1);
        left_str[comma] = '\0';
        right_str[length - comma - 1] = '\0';
        
        const int right_max_length = 4;

        if (strlen(right_str) > right_max_length) {
            right_str[4] = '\0';
        }
        
        int left = atoi(left_str);
        int right = atoi(right_str);
        
        Number divider = ten_power_table[strlen(right_str)];
            
        Number nb_left = nb_from_int(left);
        int sign = nb_left < nb_zero ? -1 : 1;

        Number lr = nb_from_int(right);
        lr = nb_div(lr, divider);
        lr *= sign;
        
        return nb_left + lr;
        
    } else {
        return atoi(value);
    }
 }

Number nb_from_string(const char *value) {
    int i = 0;
    for (; value[i] != '\0'; ++i);
    return nb_from_string_n(value, i);
}
 
float nb_to_float(Number value) {
    return (float)value / (float)nb_one;
}

double nb_to_double(Number value) {
    return (double)value / (double)nb_one;
}

int nb_to_int(Number value) {
    return value >> DECIMAL_BITS;
}

char *nb_to_str(Number value, int precision) {
    char res[20];
    char format[6];
    snprintf(format, 6, "%%.%df", precision);
    snprintf(res, 20, format, nb_to_double(value));
    return platform_strdup(res);
}

void nb_calculate_constants() {
    Number pi = nb_from_string("3.14159265358979323846264338327950288419716939937510");
    Number pi_times_two = nb_from_string("6.28318530717958647692528676655900576839433879875020");
    Number pi_over_two = nb_from_string("1.57079632679489661923132169163975144209858469968755");
    Number pi_inv = nb_from_string("0.31830988618379067153776752674502872406891929148091");
    Number pi_over_two_inv = nb_from_string("0.63661977236758134307553505349005744813783858296183");

    Number deg_to_rad = nb_from_string("0.01745329252");
    Number rad_to_deg = nb_from_string("57.2957795131");

    printf("static const Number nb_pi = %d;\nstatic const Number nb_pi_times_two = %d;\nstatic const Number nb_pi_over_two = %d;\nstatic const Number nb_pi_inv = %d;\nstatic const Number nb_pi_over_two_inv = %d;\n\nstatic const Number nb_deg_to_rad = %d;\nstatic const Number nb_rad_to_deg = %d;\n", pi, pi_times_two, pi_over_two, pi_inv, pi_over_two_inv, deg_to_rad, rad_to_deg);

    char *s_pi = nb_to_str(pi, 5);
    char *s_pi_times_two = nb_to_str(pi_times_two, 5);
    char *s_pi_over_two = nb_to_str(pi_over_two, 5);
    char *s_pi_inv = nb_to_str(pi_inv, 5);
    char *s_pi_over_two_inv = nb_to_str(pi_over_two_inv, 5);
    char *s_pi_deg_to_rad = nb_to_str(deg_to_rad, 5);
    char *s_pi_rad_to_deg = nb_to_str(rad_to_deg, 5);

    printf("\n%s\n%s\n%s\n%s\n%s\n\n%s\n%s\n",
           s_pi,
           s_pi_times_two,
           s_pi_over_two,
           s_pi_inv,
           s_pi_over_two_inv,
           s_pi_deg_to_rad,
           s_pi_rad_to_deg);
    
    platform_free(s_pi);
    platform_free(s_pi_times_two);
    platform_free(s_pi_over_two);
    platform_free(s_pi_inv);
    platform_free(s_pi_over_two_inv);
    platform_free(s_pi_deg_to_rad);
    platform_free(s_pi_rad_to_deg);
}

#elif defined NUMBER_TYPE_FLOATING_POINT
#include <float.h>
#include <math.h>

const Number nb_max_value = FLT_MAX;
const Number nb_min_value = FLT_MIN;
const Number nb_zero = 0;
const Number nb_one = 1.f;
const Number nb_two = 2.f;
const Number nb_half = 0.5f;
const Number nb_precision = FLT_EPSILON;

const Number nb_pi = (float)M_PI;
const Number nb_pi_times_two = (float)(M_PI * 2);
const Number nb_pi_over_two = (float)M_PI_2;
const Number nb_pi_inv = (float)M_1_PI;
const Number nb_pi_over_two_inv = (float)M_2_PI;

inline int32_t nb_sign(Number value) {
    return value < 0 ? -1 : value > 0 ? 1 : 0;
}

inline Number nb_to_radians(Number value) {
    return value / 180.f * nb_pi;
}

inline Number nb_to_degrees(Number value) {
    return value * 180.f / nb_pi;
}

inline Number nb_abs(Number value) {
    return fabsf(value);
}

inline Number nb_floor(Number value) {
    return floorf(value);
}

inline Number nb_ceil(Number value) {
    return ceilf(value);
}

inline Number nb_round(Number value) {
    return roundf(value);
}

inline Number nb_add(Number v1, Number v2) {
    return v1 + v2;
}

inline Number nb_sub(Number v1, Number v2) {
    return v1 - v2;
}

inline Number nb_mul(Number v1, Number v2) {
    return v1 * v2;
}

inline Number nb_div(Number v1, Number v2) {
    return v1 / v2;
}

inline Number nb_mod(Number v1, Number v2) {
    return fmodf(v1, v2);
}

inline Number nb_negate(Number value) {
    return -value;
}

Number nb_sqrt(Number value) {
    return sqrtf(value);
}

Number nb_sin(Number value) {
    return sinf(value);
}

Number nb_cos(Number value) {
    return cosf(value);
}

Number nb_tan(Number value) {
    return tanf(value);
}

Number nb_atan2(Number y, Number x) {
    return atan2f(y, x);
}

Number nb_from_int(int32_t value) {
    return (Number)value;
}

Number nb_from_long(int64_t value) {
    return (Number)value;
}

Number nb_from_float(float value) {
    return (Number)value;
}

Number nb_from_double(double value) {
    return (Number)value;
}

Number nb_from_string(const char *value) {
    return (Number)atof(value);
}

float nb_to_float(Number value) {
    return (float)value;
}

double nb_to_double(Number value) {
    return (double)value;
}

int nb_to_int(Number value) {
    return (int)value;
}

char *nb_to_str(Number value, int precision) {
    char res[20];
    char format[6];
    snprintf(format, 6, "%%.%df", precision);
    snprintf(res, 20, format, value);
    return platform_strdup(res);
}

void nb_calculate_constants() {}

#endif
