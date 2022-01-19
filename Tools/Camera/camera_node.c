#include "camera_node.h"

typedef struct CameraNode {
    GAME_OBJECT;
    Size2D viewport_size;
    Vector2D camera_position;
} CameraNode;

void camera_destroy(void *object)
{
    go_destroy(object);
}

char *camera_describe(void *object)
{
    CameraNode *obj = (CameraNode *)object;
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "Camera set to: ");
    sb_append_vector2d(sb, obj->camera_position);
    sb_append_string(sb, " ");
    char *go_description = go_describe(obj);
    sb_append_string(sb, go_description);
    platform_free(go_description);

    char *description = sb_get_string(sb);
    destroy(sb);
    
    return description;
}

GameObjectType CameraNodeType = {
    { { "CameraNode", &camera_destroy, &camera_describe } },
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

void camera_calculate_values(CameraNode *camera_node)
{
    AffineTransform transform = af_identity();
    transform = af_translate(transform, vec(-camera_node->camera_position.x, -camera_node->camera_position.y));
    transform = af_rotate(transform, camera_node->rotation);
    transform = af_scale(transform, camera_node->scale);
    transform = af_translate(transform, vec(camera_node->viewport_size.width / 2, camera_node->viewport_size.height / 2));

    camera_node->position = vec(transform.i13, transform.i23);
}

CameraNode *camera_create(Size2D viewport_size)
{
    GameObject *go = go_alloc(sizeof(CameraNode));
    CameraNode *camera = (CameraNode *)go;
    camera->w_type = &CameraNodeType;
    camera->viewport_size = viewport_size;
    camera->camera_position = vec_zero();
    camera->scale = vec(nb_one, nb_one);
    camera->rotation = nb_zero;
    
    camera_calculate_values(camera);

    return camera;
}

void camera_move(CameraNode *camera_node, Vector2D position)
{
    camera_node->camera_position = vec_vec_add(camera_node->camera_position, position);
    camera_calculate_values(camera_node);
}

void camera_rotate(CameraNode *camera_node, Number rotation)
{
    camera_node->rotation = camera_node->rotation - rotation;
    camera_calculate_values(camera_node);
}

void camera_scale(CameraNode *camera_node, Number scale)
{
    camera_node->scale = vec_vec_add(camera_node->scale, vec(scale, scale));
    camera_calculate_values(camera_node);
}

void camera_set_position(CameraNode *camera_node, Vector2D position)
{
    camera_node->camera_position = vec(nb_floor(position.x), nb_floor(position.y));
    camera_calculate_values(camera_node);
}

void camera_set_rotation(CameraNode *camera_node, Number rotation)
{
    camera_node->rotation = -rotation;
    camera_calculate_values(camera_node);
}

void camera_set_scale(CameraNode *camera_node, Number scale)
{
    camera_node->scale = vec(scale, scale);
    camera_calculate_values(camera_node);
}

Vector2D camera_get_position(CameraNode *camera_node)
{
    return camera_node->camera_position;
}

Number camera_get_rotation(CameraNode *camera_node)
{
    return -camera_node->rotation;
}

Number camera_get_scale(CameraNode *camera_node)
{
    return camera_node->scale.x;
}
