#ifndef Serialiser_h
#define Serialiser_h

#include "engine.h"

typedef struct Serialiser Serialiser;
typedef void (serialise_function_t)(Serialiser *, void *);

void serialise_to_file(const char *file_name, void *, serialise_function_t *);
void *serialise_to_buffer(void *, serialise_function_t *);

void ser_write_bool(Serialiser *, bool);
void ser_write_char(Serialiser *, char);
void ser_write_int(Serialiser *, int);
void ser_write_int8(Serialiser *, int8_t);
void ser_write_int16(Serialiser *, int16_t);
void ser_write_int32(Serialiser *, int32_t);
void ser_write_float(Serialiser *, float);
void ser_write_str(Serialiser *, char *);
void ser_write_obj_with_function(Serialiser *, void *, serialise_function_t *);


typedef struct Deserialiser Deserialiser;
typedef void * (deserialise_function_t)(Deserialiser *);

void *deserialise_object(uint8_t *buffer, size_t length, deserialise_function_t *);
void deserialise_file(const char *file_name, deserialise_function_t *, object_callback_t *callback, void *context);

bool deser_read_bool(Deserialiser *);
char deser_read_char(Deserialiser *);
int deser_read_int(Deserialiser *);
int8_t deser_read_int8(Deserialiser *);
int16_t deser_read_int16(Deserialiser *);
int32_t deser_read_int32(Deserialiser *);
float deser_read_float(Deserialiser *);
char * deser_read_str(Deserialiser *);
void * deser_read_obj_with_function(Deserialiser *, deserialise_function_t *deserialise_function);

#endif /* Serialiser_h */
