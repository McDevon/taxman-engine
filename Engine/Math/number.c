#include "number.h"
#include "platform_adapter.h"
#include <string.h>
#include <stdlib.h>
#include "number_lut.h"

#define NUMBER_BITS 32
#define DECIMAL_BITS 10

#define DECIMAL_MASK 0x3ff
#define FULL_MASK -1

const FixNumber fn_max_value = INT_MAX;
const FixNumber fn_min_value = INT_MIN;
const FixNumber fn_zero = 0;
const FixNumber fn_one = 1 << DECIMAL_BITS;
const FixNumber fn_two = 1 << (DECIMAL_BITS + 1);
const FixNumber fn_half = 1 << (DECIMAL_BITS - 1);
const FixNumber fn_precision = 1;

const FixNumber fn_pi = 3216;
const FixNumber fn_pi_times_two = 6433;
const FixNumber fn_pi_over_two = 1608;
const FixNumber fn_pi_inv = 325;
const FixNumber fn_pi_over_two_inv = 651;

#define fn_rad_to_deg 58670
#define fn_180 184320

static const int32_t LUT_SIZE = 1608;

static const FixNumber ten_power_table[] = {
    1024,
    10240,
    102400,
    1024000,
    10240000,
    102400000,
    1024000000
};

FixNumber fn_mul(FixNumber v1, FixNumber v2);
FixNumber fn_div(FixNumber v1, FixNumber v2);

inline int32_t fn_sign(FixNumber value)
{
    return value < 0 ? -1 : value > 0 ? 1 : 0;
}

inline FixNumber fn_to_radians(FixNumber value) {
    return fn_div(fn_mul(value, fn_pi), fn_180);
}

inline FixNumber fn_to_degrees(FixNumber value) {
    return fn_mul(value, fn_rad_to_deg);
}

inline FixNumber fn_abs(FixNumber value) {
    const int32_t mask = value >> (NUMBER_BITS - 1);
    return (value + mask) ^ mask;
}

inline FixNumber fn_floor(FixNumber value) {
    return value & (~DECIMAL_MASK);
}

inline FixNumber fn_ceil(FixNumber value) {
    return (value + fn_one - 1) & (~DECIMAL_MASK);
}

inline FixNumber fn_round(FixNumber value) {
    return (value + fn_half) & (~DECIMAL_MASK);
}

#pragma mark - Operators

inline FixNumber fn_add(FixNumber v1, FixNumber v2) {
    return v1 + v2;
}

inline FixNumber fn_sub(FixNumber v1, FixNumber v2) {
    return v1 - v2;
}

inline FixNumber fn_mul(FixNumber v1, FixNumber v2) {
    return (int32_t)(((int64_t)v1 * v2) >> DECIMAL_BITS);
}

inline FixNumber fn_div(FixNumber v1, FixNumber v2) {
    return (int32_t)(((int64_t)v1 << DECIMAL_BITS) / v2);
}

inline FixNumber fn_mod(FixNumber v1, FixNumber v2) {
    return v1 % v2;
}

inline FixNumber fn_negate(FixNumber value) {
    return -value;
}

#pragma mark - Functions

FixNumber fn_sqrt(FixNumber value) {
    
    if (value < 0) {
        return -fn_one;
    }

    FixNumber num = value;
    FixNumber result = 0;

    FixNumber bit = 1 << (NUMBER_BITS - 2);

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
                num = (num << (DECIMAL_BITS)) - fn_half;
                result = (result << (DECIMAL_BITS)) + fn_half;
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

FixNumber fn_sin(FixNumber value) {
    
    FixNumber angle = value;
    
    int32_t clamp2pi = angle % fn_pi_times_two;
    if (angle < 0) {
        clamp2pi += fn_pi_times_two;
    }
    
    uint8_t flip_v = clamp2pi >= fn_pi;
    int32_t clamp_pi = clamp2pi;
    
    while (clamp_pi >= fn_pi) {
        clamp_pi -= fn_pi;
    }
    
    uint8_t flip_h = clamp_pi >= fn_pi_over_two;
    
    int32_t clamp_pi_per_2 = clamp_pi;
    if (clamp_pi_per_2 >= fn_pi_over_two) {
        clamp_pi_per_2 -= fn_pi_over_two;
    }
    
    if (clamp_pi_per_2 >= LUT_SIZE) {
        clamp_pi_per_2 = LUT_SIZE - 1;
    }
    
    FixNumber result = fn_lut_sin[flip_h ? LUT_SIZE - 1 - clamp_pi_per_2 : clamp_pi_per_2];
    return flip_v ? -result : result;
}

FixNumber fn_cos(FixNumber value) {
    FixNumber sin_angle = value + (value > 0 ? -fn_pi - fn_pi_over_two : fn_pi_over_two);
    return fn_sin(sin_angle);
}

FixNumber fn_tan(FixNumber value) {
    int32_t clamp_pi = value % fn_pi;
    uint8_t flip = 0;
    if (clamp_pi < 0) {
        clamp_pi = -clamp_pi;
        flip = 1;
    }
    
    if (clamp_pi > fn_pi_over_two) {
        flip = !flip;
        clamp_pi = fn_pi_over_two - (clamp_pi - fn_pi_over_two);
    }
        
    if (clamp_pi >= LUT_SIZE) {
        clamp_pi = LUT_SIZE - 1;
    }
    
    FixNumber result = fn_lut_tan[clamp_pi];
    return flip ? -result : result;
}

static const FixNumber atan2_help = 0; //Fixed.fromString("0.28");

FixNumber fn_atan2(FixNumber y, FixNumber x) {
    if (x == 0) {
        if (y > 0) {
            return fn_pi_over_two;
        }
        if (y == 0) {
            return fn_zero;
        }
        return -fn_pi_over_two;
    }
    FixNumber atan;
    FixNumber z = fn_div(y, x);

    FixNumber divider = fn_one + (fn_mul(fn_mul(atan2_help, z), z));

    if (fn_abs(z) < fn_one) {
        atan = fn_div(z, divider);
        if (x < 0) {
            if (y < 0) {
                return atan - fn_pi;
            }
            return atan + fn_pi;
        }
    }
    else {
        atan = fn_pi_over_two - (fn_div(z, (fn_mul(z, z) + atan2_help)));
        if (y < 0) {
            return atan - fn_pi;
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

FixNumber fn_from_int(int32_t value) {
    return value * fn_one;
}

FixNumber fn_from_long(int64_t value) {
    return (int32_t)value * fn_one;
}

FixNumber fn_from_float(float value) {
    return (FixNumber)(value * fn_one);
}

FixNumber fn_from_double(double value) {
    return (FixNumber)(value * fn_one);
}

FixNumber fn_from_string_n(const char *value, const size_t length) {
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
        
        FixNumber divider = ten_power_table[strlen(right_str)];
            
        FixNumber fn_left = fn_from_int(left);
        int sign = fn_left < fn_zero ? -1 : 1;

        FixNumber lr = fn_from_int(right);
        lr = fn_div(lr, divider);
        lr *= sign;
        
        return fn_left + lr;
        
    } else {
        return atoi(value);
    }
 }

FixNumber fn_from_string(const char *value) {
    int i = 0;
    for (; value[i] != '\0'; ++i);
    return fn_from_string_n(value, i);
}
 
float fn_to_float(FixNumber value) {
    return (float)value / (float)fn_one;
}

double fn_to_double(FixNumber value) {
    return (double)value / (double)fn_one;
}

int fn_to_int(FixNumber value) {
    return value >> DECIMAL_BITS;
}

char *fn_to_str(FixNumber value, int precision) {
    char res[20];
    char format[6];
    snprintf(format, 6, "%%.%df", precision);
    snprintf(res, 20, format, fn_to_double(value));
    return platform_strdup(res);
}

void fn_calculate_constants() {
    FixNumber pi = fn_from_string("3.14159265358979323846264338327950288419716939937510");
    FixNumber pi_times_two = fn_from_string("6.28318530717958647692528676655900576839433879875020");
    FixNumber pi_over_two = fn_from_string("1.57079632679489661923132169163975144209858469968755");
    FixNumber pi_inv = fn_from_string("0.31830988618379067153776752674502872406891929148091");
    FixNumber pi_over_two_inv = fn_from_string("0.63661977236758134307553505349005744813783858296183");

    FixNumber deg_to_rad = fn_from_string("0.01745329252");
    FixNumber rad_to_deg = fn_from_string("57.2957795131");

    printf("static const Number fn_pi = %d;\nstatic const Number fn_pi_times_two = %d;\nstatic const Number fn_pi_over_two = %d;\nstatic const Number fn_pi_inv = %d;\nstatic const Number fn_pi_over_two_inv = %d;\n\nstatic const Number fn_deg_to_rad = %d;\nstatic const Number fn_rad_to_deg = %d;\n", pi, pi_times_two, pi_over_two, pi_inv, pi_over_two_inv, deg_to_rad, rad_to_deg);

    char *s_pi = fn_to_str(pi, 5);
    char *s_pi_times_two = fn_to_str(pi_times_two, 5);
    char *s_pi_over_two = fn_to_str(pi_over_two, 5);
    char *s_pi_inv = fn_to_str(pi_inv, 5);
    char *s_pi_over_two_inv = fn_to_str(pi_over_two_inv, 5);
    char *s_pi_deg_to_rad = fn_to_str(deg_to_rad, 5);
    char *s_pi_rad_to_deg = fn_to_str(rad_to_deg, 5);

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
