#ifndef Number_h
#define Number_h

#include <stdio.h>
#include <limits.h>

typedef int32_t FixNumber;

extern const FixNumber fn_max_value;
extern const FixNumber fn_min_value;
extern const FixNumber fn_zero;
extern const FixNumber fn_one;
extern const FixNumber fn_two;
extern const FixNumber fn_half;
extern const FixNumber fn_precision;

extern const FixNumber fn_pi;
extern const FixNumber fn_pi_times_two;
extern const FixNumber fn_pi_over_two;
extern const FixNumber fn_pi_inv;
extern const FixNumber fn_pi_over_two_inv;

int32_t fn_sign(FixNumber value);
FixNumber fn_to_radians(FixNumber value);
FixNumber fn_to_degrees(FixNumber value);
FixNumber fn_abs(FixNumber value);
FixNumber fn_floor(FixNumber value);
FixNumber fn_ceil(FixNumber value);
FixNumber fn_round(FixNumber value);

FixNumber fn_add(FixNumber v1, FixNumber v2);
FixNumber fn_sub(FixNumber v1, FixNumber v2);
FixNumber fn_mul(FixNumber v1, FixNumber v2);
FixNumber fn_div(FixNumber v1, FixNumber v2);
FixNumber fn_mod(FixNumber v1, FixNumber v2);
FixNumber fn_negate(FixNumber value);

FixNumber fn_sqrt(FixNumber value);
FixNumber fn_sin(FixNumber value);
FixNumber fn_cos(FixNumber value);
FixNumber fn_tan(FixNumber value);
FixNumber fn_atan2(FixNumber y, FixNumber x);

FixNumber fn_from_int(int32_t value);
FixNumber fn_from_long(int64_t value);
FixNumber fn_from_float(float value);
FixNumber fn_from_double(double value);
FixNumber fn_from_string(const char *value);

float fn_to_float(FixNumber value);
double fn_to_double(FixNumber value);
int fn_to_int(FixNumber value);
char *fn_to_str(FixNumber value, int precision);

void fn_calculate_constants(void);

#endif /* Number_h */
