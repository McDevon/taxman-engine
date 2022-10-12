#ifndef bezier_h
#define bezier_h

#include "base_object.h"
#include "types.h"

extern BaseType BezierModelType;
extern BaseType BezierPrecomputedType;

typedef struct BezierModel BezierModel;
typedef struct BezierPrecomputed BezierPrecomputed;

BezierModel *bezier_model_create(Float[4]);
Float bezier_compute(BezierModel *model, Float x);

BezierPrecomputed *bezier_precomputed_create(Float control_points[4], size_t table_size);
BezierPrecomputed *bezier_precomputed_create_from_table(Float *table, size_t table_size);
Float bezier_precomputed_get(BezierPrecomputed *data, Float x);

char *bezier_precomputed_get_table(BezierPrecomputed *data);

#endif /* bezier_h */
