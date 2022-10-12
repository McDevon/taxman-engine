#ifndef bezier_private_h
#define bezier_private_h

#include "base_object.h"

#define SPLINE_TABLE_SIZE 11

typedef struct BezierModel {
    BASE_OBJECT;
    Float control_points[4];
    Float spline_table[SPLINE_TABLE_SIZE];
} BezierModel;

typedef struct BezierPrecomputed {
    BASE_OBJECT;
    Float *table;
    size_t table_size;
} BezierPrecomputed;

#endif /* bezier_private_h */
