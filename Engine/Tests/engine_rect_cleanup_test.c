#include "engine_rect_cleanup_test.h"
#include "array_list.h"
#include "render_rect.h"
#include "constants.h"
#include "render_context.h"
#include "platform_adapter.h"
#include "engine_log.h"
#include "random.h"
#include "string_builder.h"

void engine_rect_cleanup_test_fill_canvas(uint8_t *canvas, ArrayList *squares)
{
    size_t count = list_count(squares);
    
    for (size_t i = 0; i < count; ++i) {
        RenderRect *sq = list_get(squares, i);
        for (int k = sq->top; k <= sq->bottom; ++k) {
            for (int j = sq->left; j <= sq->right; ++j) {
                int32_t t_index = j + k * SCREEN_WIDTH;
                canvas[t_index] = 1;
            }
        }
    }
}

ArrayList * engine_rect_cleanup_test_array_to_square_list(int squares[][4], int len)
{
    ArrayList *list = list_create();
    
    for (int i = 0; i < len; ++i) {
        RenderRect *sq = rrect_create(squares[i][0],
                                   squares[i][1],
                                   squares[i][2],
                                   squares[i][3]);
        list_add(list, sq);
    }
    
    return list;
}

int engine_rect_cleanup_test_run_test_case(ArrayList *start, const char *test_name)
{
    ArrayList *end = list_create();
    context_clean_union_of_rendered_rects(NULL, start, end);

    uint8_t *start_canvas = platform_calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(uint8_t));
    uint8_t *end_canvas = platform_calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(uint8_t));

    engine_rect_cleanup_test_fill_canvas(start_canvas, start);
    engine_rect_cleanup_test_fill_canvas(end_canvas, end);

    bool fills_passed = true;
    
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i) {
        if (start_canvas[i] != end_canvas[i]) {
            fills_passed = false;
            break;
        }
    }
    
    if (!fills_passed) {
        LOG_ERROR("Rect test FILLS %s FAILED", test_name);
    }
    
    platform_free(start_canvas);
    platform_free(end_canvas);
    
    bool overlap_passed = true;
    bool form_passed = true;
    size_t end_count = list_count(end);
    for (int i = 0; i < end_count; ++i) {
        RenderRect *a = list_get(end, i);
        for (int k = i + 1; k < end_count; ++k) {
            RenderRect *b = list_get(end, k);
            if (!(a->left > b->right
                || a->right < b->left
                || a->top > b->bottom
                || a->bottom < b->top)) {
                overlap_passed = false;
            }
        }
        if (a->left > a->right || a->top > a->bottom) {
            form_passed = false;
        }
    }
    
    if (!overlap_passed) {
        LOG_ERROR("Rect test OVERLAP %s FAILED", test_name);
    }
    if (!form_passed) {
        LOG_ERROR("Rect test RECTS PROPERLY FORMED %s FAILED", test_name);
    }

    destroy(end);
    
    return (fills_passed ? 0 : 1) + (overlap_passed ? 0 : 1) + (form_passed ? 0 : 1);
}

int engine_rect_cleanup_test_run_generated_test_case(Random *state, int index)
{
    int square_count = random_next_int_limit(state, 5) + 10;
    
    ArrayList *start = list_create();
    ArrayList *end = list_create();

    for (int i = 0; i < square_count; ++i) {
        int left = random_next_int_limit(state, SCREEN_WIDTH - 20);
        int right = left + random_next_int_limit(state, SCREEN_WIDTH - left);
        int top = random_next_int_limit(state, SCREEN_HEIGHT - 20);
        int bottom = top + random_next_int_limit(state, SCREEN_HEIGHT - top);
        list_add(start,
                 rrect_create(left, right, top, bottom));
    }
    
    context_clean_union_of_rendered_rects(NULL, start, end);
    
    StringBuilder *sb = sb_create();
    sb_append_string(sb, "generated_squares_");
    sb_append_int(sb, index);
    char *test_name = sb_get_string(sb);
    destroy(sb);
    
    int result = engine_rect_cleanup_test_run_test_case(start, test_name);
    
    platform_free(test_name);
    destroy(start);
    destroy(end);
    
    return result;
}

int engine_rect_cleanup_test()
{
    int result = 0;
    
    int ending_valley_rects[][4] = {
        {100, 300, 50, 200},
        {240, 340, 40, 100},
        {280, 330, 170, 220}
    };
    ArrayList *ending_valley_list = engine_rect_cleanup_test_array_to_square_list(ending_valley_rects, 3);
    result += engine_rect_cleanup_test_run_test_case(ending_valley_list, "Ending valley");
    destroy(ending_valley_list);
    
    int long_thin_rects[][4] = {
        {100, 300, 50, 200},
        {140, 340, 40, 70},
        {50, 380, 55, 58},
        {150, 320, 60, 80}
    };
    ArrayList *long_thin_list = engine_rect_cleanup_test_array_to_square_list(long_thin_rects, 4);
    result += engine_rect_cleanup_test_run_test_case(long_thin_list, "Long thin rect");
    destroy(long_thin_list);
    
    Random *rect_random = random_create(4608090406132658590LL, 5588768554981732228LL);
    
    for (int i = 1; i <= 200; ++i) {
        result += engine_rect_cleanup_test_run_generated_test_case(rect_random, i);
    }
    
    destroy(rect_random);
    
    return result;
}
