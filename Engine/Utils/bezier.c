#include "bezier.h"
#include "bezier_private.h"
#include "types.h"
#include "platform_adapter.h"
#include "string_builder.h"
#include <math.h>
#include <string.h>

#include "engine_log.h"

#define BEZIER_ITERATIONS 4
#define BEZIER_DELTA_MIN 0.001f
#define BEZIER_SUB_PRECISION 0.000001f
#define BEZIER_SUB_ITERATIONS 10

Float sample_step = 1.f / (SPLINE_TABLE_SIZE - 1.f);

#define CP_X_0 0
#define CP_Y_0 1
#define CP_X_1 2
#define CP_Y_1 3

void bezier_model_destroy(void *object)
{
}

char *bezier_model_describe(void *object)
{
    BezierModel *model = (BezierModel *)object;
    
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "Control points: ");
    sb_append_float(sb, model->control_points[0], 3);
    sb_append_string(sb, ", ");
    sb_append_float(sb, model->control_points[1], 3);
    sb_append_string(sb, ", ");
    sb_append_float(sb, model->control_points[2], 3);
    sb_append_string(sb, ", ");
    sb_append_float(sb, model->control_points[3], 3);
    
    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

BaseType BezierModelType = { "BezierModel", &bezier_model_destroy, &bezier_model_describe };

void bezier_precomputed_destroy(void *object)
{
    BezierPrecomputed *self = (BezierPrecomputed *)object;
    platform_free(self->table);
}

char *bezier_precomputed_describe(void *object)
{
    BezierPrecomputed *self = (BezierPrecomputed *)object;
    
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "Table size: ");
    sb_append_int(sb, (int)self->table_size);
    
    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

BaseType BezierPrecomputedType = { "BezierPrecomputed", &bezier_precomputed_destroy, &bezier_precomputed_describe };

Float bezier_a_fn(Float a1, Float a2) {
    return 1.f - 3.f * a2 + 3.f * a1;
}

Float bezier_b_fn(Float a1, Float a2) {
    return 3.f * a2 - 6.f * a1;
}

Float bezier_c_fn(Float a1) {
    return 3.f * a1;
}

Float bezier_compute_value(Float t, Float a1, Float a2) {
    return ((bezier_a_fn(a1, a2) * t + bezier_b_fn(a1, a2)) * t + bezier_c_fn(a1)) * t;
}

Float bezier_compute_delta(Float t, Float a1, Float a2) {
    return 3.f * bezier_a_fn(a1, a2) * t * t + 2.f * bezier_b_fn(a1, a2) * t + bezier_c_fn(a1);
}

Float bezier_sub(Float x, Float a, Float b, Float x1, Float x2)
{
    Float current_x, current_t, i = 0.f;
    do {
      current_t = a + (b - a) * .5f;
      current_x = bezier_compute_value(current_t, x1, x2) - x;
      if (current_x > 0.0) {
        b = current_t;
      } else {
        a = current_t;
      }
    } while (fabsf(current_x) > BEZIER_SUB_PRECISION && ++i < BEZIER_SUB_ITERATIONS);
    return current_t;
}

Float bezier_newton_raphson(Float x, Float guess_t, Float x1, Float x2)
{
    for (int i = 0; i < BEZIER_ITERATIONS; ++i) {
      Float delta = bezier_compute_delta(guess_t, x1, x2);
      if (delta == 0.f) {
        return guess_t;
      }
      Float current_x = bezier_compute_value(guess_t, x1, x2) - x;
      guess_t -= current_x / delta;
    }
    return guess_t;
}

BezierModel *bezier_model_create(Float control_points[4])
{
    BezierModel *model = platform_calloc(1, sizeof(BezierModel));
    memcpy(model->control_points, control_points, sizeof(Float[4]));
    
    for (int i = 0; i < SPLINE_TABLE_SIZE; ++i) {
        model->spline_table[i] = bezier_compute_value(i * sample_step, control_points[CP_X_0], control_points[CP_X_1]);
    }
    model->w_type = &BezierModelType;
    
    return model;
}

Float bezier_compute_t(BezierModel *model, Float x)
{
    int current_sample = 1;
    int last_sample = SPLINE_TABLE_SIZE - 1;
    Float interval_start = 0.f;
    
    Float *table = model->spline_table;

    while (current_sample < last_sample && table[current_sample] <= x) {
      interval_start += sample_step;
        ++current_sample;
    }
    --current_sample;

    Float dist = (x - table[current_sample]) / (table[current_sample + 1] - table[current_sample]);
    Float guess = interval_start + dist * sample_step;

    Float start_delta = bezier_compute_delta(guess, model->control_points[CP_X_0], model->control_points[CP_X_1]);
    if (start_delta >= BEZIER_DELTA_MIN) {
      return bezier_newton_raphson(x, guess, model->control_points[CP_X_0], model->control_points[CP_X_1]);
    } else if (start_delta == 0.f) {
      return guess;
    } else {
      return bezier_sub(x, interval_start, interval_start + sample_step, model->control_points[CP_X_0], model->control_points[CP_X_1]);
    }
}

Float bezier_compute(BezierModel *model, Float x)
{
    if (x == 0.f || x == 1.f) {
      return x;
    }
    return bezier_compute_value(bezier_compute_t(model, x), model->control_points[CP_Y_0], model->control_points[CP_Y_1]);
}

BezierPrecomputed *bezier_precomputed_create(Float control_points[4], size_t table_size)
{
    BezierPrecomputed *data = platform_calloc(1, sizeof(BezierPrecomputed));
    data->table_size = table_size;
    data->table = platform_calloc(table_size, sizeof(Float));
    data->w_type = &BezierPrecomputedType;
    
    BezierModel *model = bezier_model_create(control_points);
    
    for (size_t i = 0; i < table_size; ++i) {
        data->table[i] = bezier_compute(model, i * (1.f / (table_size - 1)));
    }
    
    destroy(model);
    
    return data;
}

BezierPrecomputed *bezier_precomputed_create_from_table(Float *table, size_t table_size)
{
    BezierPrecomputed *data = platform_calloc(1, sizeof(BezierPrecomputed));
    data->w_type = &BezierPrecomputedType;
    data->table_size = table_size;
    data->table = platform_calloc(table_size, sizeof(Float));
    
    for (size_t i = 0; i < table_size; ++i) {
        data->table[i] = table[i];
    }
    
    return data;
}

Float bezier_precomputed_get(BezierPrecomputed *data, Float x)
{
    if (x <= 0.f) {
        return 0.f;
    }
    if (x >= 1.f) {
        return 1.f;
    }
    
    Float position = x * (data->table_size - 1);
    size_t index = (size_t)position;
    Float sub_pos = position - index;
    
    return data->table[index] + (data->table[index + 1] - data->table[index]) * sub_pos;
}

char *bezier_precomputed_get_table(BezierPrecomputed *self)
{
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "bezier_precomputed_create_from_table((float[]){ ");
    for (int i = 0; i < self->table_size; ++i) {
        sb_append_float(sb, self->table[i], 6);
        sb_append_string(sb, "f");
        if (i < self->table_size - 1) {
            sb_append_string(sb, ", ");
        }
    }
    sb_append_string(sb, " }, ");
    sb_append_int(sb, (int)self->table_size);
    sb_append_string(sb, ");");

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}
