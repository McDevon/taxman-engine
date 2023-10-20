#include <math.h>
#include "platform_adapter.h"

int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

inline int32_t float_sign(float value) {
    return value < 0 ? -1 : value > 0 ? 1 : 0;
}

inline float float_to_radians(float value) {
    return value / 180.f * (float)M_PI;
}

inline float float_to_degrees(float value) {
    return value * 180.f / (float)M_PI;
}

char *float_to_str(float value, int precision) {
    char res[20];
    char format[6];
    snprintf(format, 6, "%%.%df", precision);
    snprintf(res, 20, format, value);
    return platform_strdup(res);
}
