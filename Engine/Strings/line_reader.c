#include "line_reader.h"
#include "platform_adapter.h"
#include "array_list.h"
#include <stdlib.h>
#include <string.h>

struct token_reader_context {
    void *context;
    tokens_callback_t tokens_callback;
    line_callback_t line_callback;
    char *file_name;
    char *delimeters;
    size_t delimeter_count;
};

struct line_reader_context {
    void *context;
    line_callback_t line_callback;
};

void read_token_line(const char *line, int32_t row_number, bool last_line, void *context)
{
    struct token_reader_context *token_ctx = (struct token_reader_context *)context;
    
    const size_t delimeter_count = token_ctx->delimeter_count;
    const char *delimeters = token_ctx->delimeters;
    
    int token_start = 0;
    
    ArrayList *tokens = list_create_with_destructor(&platform_free);
    
    size_t length = strlen(line);
    
    for (int32_t i = 0; i < length; ++i) {
        char chr = line[i];
        bool delimeter_found = false;
        for (int32_t j = 0; j < delimeter_count; ++j) {
            if (chr == delimeters[j]) {
                delimeter_found = true;
                break;
            }
        }
        if (!delimeter_found && i == length - 1) {
            delimeter_found = true;
            ++i;
        }
        
        if (delimeter_found) {
            const int token_length = i - token_start;
            if (token_length > 0) {
                char *found_token = platform_strndup(line + token_start, token_length);
                if (found_token) {
                    list_add(tokens, found_token);
                }
            }
            token_start = i + 1;
        }
    }
    size_t token_count = list_count(tokens);
    
    if (token_count > 0) {
        char *token_array[token_count];
        
        for (size_t i = 0; i < token_count; ++i) {
            token_array[i] = list_get(tokens, i);
        }
        
        token_ctx->tokens_callback(token_array, (int)token_count, row_number, last_line, token_ctx->context);
    } else {
        token_ctx->tokens_callback(NULL, (int)0, row_number, last_line, token_ctx->context);
    }
        
    destroy(tokens);
    
    if (last_line) {
        platform_free(token_ctx->delimeters);
        if (token_ctx->file_name) {
            platform_free(token_ctx->file_name);            
        }
        platform_free(token_ctx);
    }
}

void string_tokenize(const char *string, const char delimeters[], const size_t delimeter_count, tokens_callback_t tokens_callback, void *context)
{
    struct token_reader_context *token_ctx = platform_calloc(sizeof(struct token_reader_context), 1);
    token_ctx->context = context;
    token_ctx->tokens_callback = tokens_callback;
    token_ctx->delimeters = platform_strdup(delimeters);
    token_ctx->delimeter_count = delimeter_count;
    read_token_line(string, 0, true, context);
}

void file_read_lines_tokenize(const char *file_name, const char delimeters[], const size_t delimeter_count, tokens_callback_t tokens_callback, void *context)
{
    struct token_reader_context *token_ctx = platform_calloc(sizeof(struct token_reader_context), 1);
    token_ctx->context = context;
    token_ctx->tokens_callback = tokens_callback;
    token_ctx->delimeters = platform_strdup(delimeters);
    token_ctx->delimeter_count = delimeter_count;
    token_ctx->file_name = platform_strdup(file_name);
    file_read_lines(file_name, &read_token_line, token_ctx);
}

void read_full_file_callback(const char *file_name, const char *file_data, void *context)
{
    struct line_reader_context *line_ctx = (struct line_reader_context *)context;
    int row = 0;
    int row_length = 0;
    int row_start = 0;
    
    char chr;
    
    for (int32_t i = 0;; ++i) {
        chr = file_data[i];
        if (chr == '\n' || chr == '\0') {
            
            char *line = platform_strndup(file_data + row_start, row_length);
            line_ctx->line_callback(line, row, chr == '\0', line_ctx->context);
            platform_free(line);
            
            if (chr == '\0') {
                break;
            }
            
            row_length = 0;
            row_start = i + 1;
            ++row;
        } else {
            ++row_length;
        }
    }
    platform_free(line_ctx);
}

void file_read_lines(const char *file_name, line_callback_t line_callback, void *context)
{
    struct line_reader_context *line_ctx = platform_calloc(sizeof(struct line_reader_context), 1);
    line_ctx->context = context;
    line_ctx->line_callback = line_callback;
    platform_read_text_file(file_name, &read_full_file_callback, line_ctx);
}
