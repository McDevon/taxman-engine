#ifndef crank_utils_h
#define crank_utils_h

typedef struct FloatPair {
    float a;
    float b;
} FloatPair;

FloatPair to_same_half_circle_degrees(float a, float b);
FloatPair to_same_half_circle_radians(float a, float b);

#endif /* crank_utils_h */
