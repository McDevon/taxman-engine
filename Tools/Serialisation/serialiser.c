#include "serialiser.h"

typedef struct Serialiser {
    BASE_OBJECT;
    uint8_t *q_buffer;
    size_t length;
    size_t capacity;
    bool buffer_owned;
} Serialiser;

#define SERIALISER_DEFAULT_INITIAL_CAPACITY 128

void serialiser_destroy(void *value);
char *serialiser_describe(void *value);

Serialiser *ser_create(void);

void serialiser_write_to_file_callback(const char *file_name, bool success, void *context)
{
    Serialiser *self = (Serialiser *)context;
    destroy(self);
}

void serialise_to_file(const char *file_name, void *obj, serialise_function_t *serialise_function)
{
    Serialiser *self = ser_create();
    
    serialise_function(self, obj);
    
    if (self->length == 0) {
        LOG("Serialiser error: nothing serialised");
        destroy(self);
        return;
    }
    
    platform_write_data_file(file_name, self->q_buffer, self->length, &serialiser_write_to_file_callback, self);
}

void *serialise_to_buffer(void *obj, serialise_function_t *serialise_function)
{
    Serialiser *self = ser_create();
    self->buffer_owned = false;
    
    serialise_function(self, obj);
    
    uint8_t *buffer = self->q_buffer;
    
    destroy(self);
    
    return buffer;
}

BaseType SerialiserType = { "Serialiser", &serialiser_destroy, &serialiser_describe };

int serialiser_ensure_can_add(Serialiser *self, size_t size)
{
    if (self->length + size > self->capacity) {
        size_t new_capacity = self->capacity * 2;
        void *new_buffer = platform_realloc(self->q_buffer, sizeof(uint8_t) * new_capacity);
        if (!new_buffer) {
            LOG("Failed to realloc serialiser buffer");
            return 1;
        }
        
        self->capacity = new_capacity;
        self->q_buffer = new_buffer;
    }
    return 0;
}

void serializer_write_value(Serialiser *self, void *buffer, size_t size)
{
    // TODO: General implementation
}

void ser_write_bool(Serialiser *self, bool value)
{
    size_t size = sizeof(bool);
    if (serialiser_ensure_can_add(self, size) != 0) {
        return;
    }
    memcpy(self->q_buffer + self->length, &value, size);
    self->length += size;
}

void ser_write_char(Serialiser *self, char value)
{
    size_t size = sizeof(char);
    if (serialiser_ensure_can_add(self, size) != 0) {
        return;
    }
    memcpy(self->q_buffer + self->length, &value, size);
    self->length += size;
}

void ser_write_int(Serialiser *self, int value)
{
    size_t size = sizeof(int);
    if (serialiser_ensure_can_add(self, size) != 0) {
        return;
    }
    memcpy(self->q_buffer + self->length, &value, size);
    self->length += size;
}

void ser_write_int8(Serialiser *self, int8_t value)
{
    size_t size = sizeof(int8_t);
    if (serialiser_ensure_can_add(self, size) != 0) {
        return;
    }
    memcpy(self->q_buffer + self->length, &value, size);
    self->length += size;
}

void ser_write_int16(Serialiser *self, int16_t value)
{
    size_t size = sizeof(int16_t);
    if (serialiser_ensure_can_add(self, size) != 0) {
        return;
    }
    memcpy(self->q_buffer + self->length, &value, size);
    self->length += size;
}

void ser_write_int32(Serialiser *self, int32_t value)
{
    size_t size = sizeof(int32_t);
    if (serialiser_ensure_can_add(self, size) != 0) {
        return;
    }
    memcpy(self->q_buffer + self->length, &value, size);
    self->length += size;
}

void ser_write_float(Serialiser *self, float value)
{
    size_t size = sizeof(float);
    if (serialiser_ensure_can_add(self, size) != 0) {
        return;
    }
    memcpy(self->q_buffer + self->length, &value, size);
    self->length += size;
}

void ser_write_str(Serialiser *self, char *value)
{
    size_t size = sizeof(char) * strlen(value);
    ser_write_int16(self, (int16_t)size);
    if (serialiser_ensure_can_add(self, size) != 0) {
        return;
    }
    memcpy(self->q_buffer + self->length, value, size);
    self->length += size;
}

void ser_write_obj_with_function(Serialiser *self, void *obj, serialise_function_t *func)
{
    func(self, obj);
}

typedef struct {
    resource_callback_t *callback;
    void *context;
} SerialiserFileContext;

Serialiser *ser_create()
{
    void *buffer = platform_calloc(SERIALISER_DEFAULT_INITIAL_CAPACITY, sizeof(uint8_t));
    if (!buffer) { return NULL; }
    
    Serialiser *ser = platform_calloc(1, sizeof(Serialiser));
    ser->capacity = SERIALISER_DEFAULT_INITIAL_CAPACITY;
    ser->length = 0;
    ser->q_buffer = buffer;
    ser->w_type = &SerialiserType;
    ser->buffer_owned = true;
    
    return ser;
}

void serialiser_destroy(void *value)
{
    Serialiser *self = (Serialiser *)value;
    if (self->buffer_owned) {
        platform_free(self->q_buffer);
    }
    self->q_buffer = NULL;
}

char *serialiser_describe(void *value)
{
    Serialiser *self = (Serialiser *)value;
    return sb_string_with_format("Length: %d, Capacity: %d", self->length, self->capacity);
}


typedef struct Deserialiser {
    BASE_OBJECT;
    const uint8_t *w_buffer;
    size_t length;
    size_t position;
} Deserialiser;

#define SERIALISER_DEFAULT_INITIAL_CAPACITY 128

void deserialiser_destroy(void *value);
char *deserialiser_describe(void *value);

BaseType DeserialiserType = { "Deserialiser", &deserialiser_destroy, &deserialiser_describe };

void *deserialise_object(uint8_t *buffer, size_t length, deserialise_function_t *deserialise_function)
{
    Deserialiser *self = platform_calloc(1, sizeof(Deserialiser));
    self->w_type = &DeserialiserType;
    self->length = length;
    self->w_buffer = buffer;
    self->position = 0;
    
    void *obj = deserialise_function(self);
    
    destroy(self);
    
    return obj;
}

typedef struct {
    deserialise_function_t *deserialise_function;
    object_callback_t *callback;
    void *context;
} DeserialiseFileContext;

void deserialise_file_callback(const char *file_name, const uint8_t *buffer, const size_t length, void *context)
{
    DeserialiseFileContext *deser_context = (DeserialiseFileContext *)context;
    if (!buffer || length == 0) {
        deser_context->callback(NULL, deser_context->context);
        return;
    }
    
    Deserialiser *self = platform_calloc(1, sizeof(Deserialiser));
    self->w_type = &DeserialiserType;
    self->w_buffer = buffer;
    self->length = length;
    self->position = 0;
    
    void *obj = deser_context->deserialise_function(self);
    
    destroy(self);
    
    deser_context->callback(obj, deser_context->context);
}

void deserialise_file(const char *file_name, deserialise_function_t *deserialise_function, object_callback_t *callback, void *context)
{
    DeserialiseFileContext deser_context;
    deser_context.context = context;
    deser_context.callback = callback;
    deser_context.deserialise_function = deserialise_function;
    
    platform_read_data_file(file_name, true, &deserialise_file_callback, &deser_context);
}

bool deser_read_bool(Deserialiser *self)
{
    size_t size = sizeof(bool);
    if (self->position + size > self->length) {
        LOG("Deserialise error, out of bounds");
        return 0;
    }
    bool value = 0;
    memcpy(&value, self->w_buffer + self->position, size);
    self->position += size;
    return value;
}

char deser_read_char(Deserialiser *self)
{
    size_t size = sizeof(char);
    if (self->position + size > self->length) {
        LOG("Deserialise error, out of bounds");
        return 0;
    }
    char value = 0;
    memcpy(&value, self->w_buffer + self->position, size);
    self->position += size;
    return value;
}

int deser_read_int(Deserialiser *self)
{
    size_t size = sizeof(int);
    if (self->position + size > self->length) {
        LOG("Deserialise error, out of bounds");
        return 0;
    }
    int value = 0;
    memcpy(&value, self->w_buffer + self->position, size);
    self->position += size;
    return value;
}

int8_t deser_read_int8(Deserialiser *self)
{
    size_t size = sizeof(int8_t);
    if (self->position + size > self->length) {
        LOG("Deserialise error, out of bounds");
        return 0;
    }
    int8_t value = 0;
    memcpy(&value, self->w_buffer + self->position, size);
    self->position += size;
    return value;
}

int16_t deser_read_int16(Deserialiser *self)
{
    size_t size = sizeof(int16_t);
    if (self->position + size > self->length) {
        LOG("Deserialise error, out of bounds");
        return 0;
    }
    int16_t value = 0;
    memcpy(&value, self->w_buffer + self->position, size);
    self->position += size;
    return value;
}

int32_t deser_read_int32(Deserialiser *self)
{
    size_t size = sizeof(int32_t);
    if (self->position + size > self->length) {
        LOG("Deserialise error, out of bounds");
        return 0;
    }
    int32_t value = 0;
    memcpy(&value, self->w_buffer + self->position, size);
    self->position += size;
    return value;
}

float deser_read_float(Deserialiser *self)
{
    size_t size = sizeof(float);
    if (self->position + size > self->length) {
        LOG("Deserialise error, out of bounds");
        return 0;
    }
    float value = 0;
    memcpy(&value, self->w_buffer + self->position, size);
    self->position += size;
    return value;
}

char * deser_read_str(Deserialiser *self)
{
    size_t length = (size_t)deser_read_int16(self);
    size_t size = length * sizeof(char);
    if (self->position + size > self->length) {
        LOG("Deserialise error, out of bounds");
        return 0;
    }
    char *value = platform_calloc(length + 1, sizeof(char));
    for (size_t i = 0; i < length; ++i) {
        value[i] = (char)*(self->w_buffer + self->position + i);
    }
    self->position += size;
    return value;
}

void * deser_read_obj_with_function(Deserialiser *deser, deserialise_function_t *deserialise_function)
{
    return deserialise_function(deser);
}

void deserialiser_destroy(void *value)
{
}

char *deserialiser_describe(void *value)
{
    Deserialiser *self = (Deserialiser *)value;
    return sb_string_with_format("Length: %d Position: %d", self->length, self->position);
}
