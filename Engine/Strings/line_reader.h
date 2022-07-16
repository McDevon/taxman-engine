#ifndef line_reader_h
#define line_reader_h

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "types.h"
#include "array_list.h"

typedef void (*tokens_callback_t)(char *tokens[], int32_t token_count, int32_t row_number, bool last_row, void *context);
typedef void (*line_callback_t)(const char *line, int32_t row_number, bool last_row, void *context);

ArrayList * string_tokenize(const char *string, const char delimeters[], const size_t delimeter_count);
void file_read_lines_tokenize(const char *file_name, const char delimeters[], const size_t delimeter_count, tokens_callback_t tokens_callback, void *context);
void file_read_lines(const char *file_name, line_callback_t line_callback, void *context);

#endif /* line_reader_h */
