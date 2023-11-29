#ifndef action_animator_private_h
#define action_animator_private_h

#include "action_animator.h"

void action_call_start(ActionObject *, GameObject *);
Float action_call_update(ActionObject *, GameObject *, Float);
void action_call_finish(ActionObject *, GameObject *);

#endif /* action_animator_private_h */
