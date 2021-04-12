#ifndef game_object_component_private_h
#define game_object_component_private_h

struct go_comp_private {
    struct GameObject *w_parent;
    bool start_called;
};

#endif /* game_object_component_private_h */
