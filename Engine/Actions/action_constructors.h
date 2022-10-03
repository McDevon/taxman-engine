#ifndef action_constructors_h
#define action_constructors_h

#include "action_animator.h"
#include "types.h"
#include "array_list.h"

ActionObject *action_move_by_create(Vector2D movement, Float length);
ActionObject *action_move_to_create(Vector2D position, Float length);

ActionObject *action_scale_by_create(Vector2D scale, Float length);
ActionObject *action_scale_to_create(Vector2D scale, Float length);

ActionObject *action_rotate_by_create(Number offset, Float length);
ActionObject *action_rotate_to_create(Number target, Float length);

ActionObject *action_resize_to_create(Size2D size, Float length);

ActionObject *action_repeat_create(ActionObject *action, int count);
ActionObject *action_sequence_create(ArrayList *actions);

ActionObject *action_delay_create(Float length);

ActionObject *action_ease_in_create(ActionObject *action);
ActionObject *action_ease_out_create(ActionObject *action);
ActionObject *action_ease_in_out_create(ActionObject *action);
ActionObject *action_ease_bezier_create(ActionObject *action, Float control_points[4]);
ActionObject *action_ease_bezier_prec_create(ActionObject *action, Float control_points[4], size_t table_size);
ActionObject *action_ease_bezier_prec_table_create(ActionObject *action, Float *table, size_t table_size);

ActionObject *action_callback_create(void (*callback)(void *obj, void *context), void *context);
ActionObject *action_function_create(void (*callback)(void *obj, void *context, Float position), void *context, Float length);
ActionObject *action_function_lerp_create(void (*callback)(void *obj, void *context, Float position), void *context, Float length, Float start, Float end);

#endif /* action_constructors_h */
