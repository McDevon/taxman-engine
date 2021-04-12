#ifndef game_object_private_h
#define game_object_private_h

struct go_private {
    ArrayList *children;
    ArrayList *components;
    struct GameObject *w_parent;
    struct SceneManager *w_scene_manager;
    int32_t z_order;
    bool z_order_dirty;
    bool start_called;
};

#endif /* game_object_private_h */
