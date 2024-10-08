#include "crank_utils.h"
#include <math.h>

FloatPair to_same_half_circle_degrees(float a, float b)
{
    while (a - b > 180.f) {
        b += 360.f;
    }
    while (a - b < -180.f) {
        a += 360.f;
    }
    
    return (FloatPair){a, b};
}

FloatPair to_same_half_circle_radians(float a, float b)
{
    while (a - b > (float)M_PI) {
        b += (float)(M_PI * 2);
    }
    while (a - b < -(float)M_PI) {
        a += (float)(M_PI * 2);
    }
    
    return (FloatPair){a, b};
}
