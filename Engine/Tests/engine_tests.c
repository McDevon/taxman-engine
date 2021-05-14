#include "engine_tests.h"
#include "engine_log.h"
#include "engine_rect_cleanup_test.h"

void engine_run_all_tests()
{
    int result = 0;
    
    result = engine_rect_cleanup_test();
    
    const char *test_result_string = result ? "FAILED" : "PASSED";
    
    LOG("TEST SUITE: %s", test_result_string);
}
