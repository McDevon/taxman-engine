#ifndef Number_h
#define Number_h

#include <stdio.h>
#include <limits.h>

#define NUMBER_TYPE_FIXED_POINT
//#undef NUMBER_TYPE_FIXED_POINT

#ifndef NUMBER_TYPE_FIXED_POINT
#define NUMBER_TYPE_FLOATING_POINT
#endif

#ifdef NUMBER_TYPE_FIXED_POINT
typedef int32_t Number;
#elif defined NUMBER_TYPE_FLOATING_POINT
typedef float Number;
#endif

extern Number nb_max_value;
extern Number nb_min_value;
extern Number nb_zero;
extern Number nb_one;
extern Number nb_two;
extern Number nb_half;
extern Number nb_precision;

extern const Number nb_pi;
extern const Number nb_pi_times_two;
extern const Number nb_pi_over_two;
extern const Number nb_pi_inv;
extern const Number nb_pi_over_two_inv;

int32_t nb_sign(Number value);
Number nb_to_radians(Number value);
Number nb_to_degrees(Number value);
Number nb_abs(Number value);
Number nb_floor(Number value);
Number nb_ceil(Number value);
Number nb_round(Number value);

Number nb_add(Number v1, Number v2);
Number nb_sub(Number v1, Number v2);
Number nb_mul(Number v1, Number v2);
Number nb_div(Number v1, Number v2);
Number nb_mod(Number v1, Number v2);
Number nb_negate(Number value);

Number nb_sqrt(Number value);
Number nb_sin(Number value);
Number nb_cos(Number value);
Number nb_tan(Number value);
Number nb_atan2(Number y, Number x);

Number nb_from_int(int32_t value);
Number nb_from_long(int64_t value);
Number nb_from_float(float value);
Number nb_from_double(double value);
Number nb_from_string(const char *value);

float nb_to_float(Number value);
double nb_to_double(Number value);
int nb_to_int(Number value);
char *nb_to_str(Number value, int precision);

void nb_calculate_constants(void);

#endif /* Number_h */
