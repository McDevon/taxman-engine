#include "line_reader.h"
#include "platform_adapter.h"
#include "array_list.h"
#include <stdlib.h>
#include <string.h>

struct token_reader_context {
    void *context;
    tokens_callback_t tokens_callback;
    const char *delimeters;
    size_t delimeter_count;
};

void read_token_line(const char *line, int32_t row_number, void *context)
{
    struct token_reader_context *token_ctx = (struct token_reader_context *)context;
    
    const size_t delimeter_count = token_ctx->delimeter_count;
    const char *delimeters = token_ctx->delimeters;
    
    int token_length = 0;
    int token_start = 0;
    
    ArrayList *tokens = list_create_with_destructor(&platform_free);
    
    size_t length = strlen(line);
    
    for (int32_t i = 0; i < length; ++i) {
        char chr = line[i];
        Bool delimeter_found = False;
        for (int32_t j = 0; j < delimeter_count; ++j) {
            if (chr == delimeters[j]) {
                delimeter_found = True;
                break;
            }
        }
        if (!delimeter_found && i == length - 1) {
            delimeter_found = True;
            ++token_length;
        }
        
        if (delimeter_found) {
            if (token_length > 0) {
                char *found_token = platform_strndup(line + token_start, token_length);
                if (found_token) {
                    list_add(tokens, found_token);
                }
            }
            token_length = 0;
            token_start = i + 1;
        } else {
            ++token_length;
        }
    }
    size_t token_count = list_count(tokens);
    
    if (token_count > 0) {
        char *token_array[token_count];
        
        for (size_t i = 0; i < token_count; ++i) {
            token_array[i] = list_get(tokens, i);
        }
        
        token_ctx->tokens_callback(token_array, (int)token_count, row_number, token_ctx->context);
    }
    
    destroy(tokens);
}

void string_tokenize(const char *string, const char delimeters[], const size_t delimeter_count, tokens_callback_t tokens_callback, void *context)
{
    struct token_reader_context token_ctx;
    token_ctx.context = context;
    token_ctx.tokens_callback = tokens_callback;
    token_ctx.delimeters = delimeters;
    token_ctx.delimeter_count = delimeter_count;
    read_token_line(string, 0, context);
}

void file_read_lines_tokenize(const char *file_name, const char delimeters[], const size_t delimeter_count, tokens_callback_t tokens_callback, void *context)
{
    struct token_reader_context token_ctx;
    token_ctx.context = context;
    token_ctx.tokens_callback = tokens_callback;
    token_ctx.delimeters = delimeters;
    token_ctx.delimeter_count = delimeter_count;
    file_read_lines(file_name, &read_token_line, &token_ctx);
}

void file_read_lines(const char *file_name, line_callback_t line_callback, void *context)
{
    char *file_data = platform_read_text_file(file_name);
    
    int row = 0;
    int row_length = 0;
    int row_start = 0;
    
    char chr;
    
    for (int32_t i = 0; (chr = file_data[i]) != '\0'; ++i) {
        if (chr == '\n') {
            
            char *line = platform_strndup(file_data + row_start, row_length);
            line_callback(line, row, context);
            platform_free(line);
            
            row_length = 0;
            row_start = i + 1;
            ++row;
        } else {
            ++row_length;
        }
    }
    
    platform_close_text_file(file_data);
}
