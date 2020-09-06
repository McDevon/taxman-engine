#ifndef engine_log_h
#define engine_log_h

#ifdef DEBUG

void log_print(const char *, ...);
void log_print_error(const char *, ...);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"

#define __ENGINE_LOG(s, ...) \
log_print(s, ##__VA_ARGS__)

#define __ENGINE_ERROR_LOG(s, ...) \
log_print_error(s, ##__VA_ARGS__)

#pragma clang diagnostic pop

#define ENABLE_ENGINELOG
//#undef ENABLE_ENGINELOG

#endif

#ifdef ENABLE_ENGINELOG
#   define LOG(...) __ENGINE_LOG(__VA_ARGS__)
#   define LOG_ERROR(...) __ENGINE_ERROR_LOG(__VA_ARGS__)
#else
#   define LOG(...) do {} while (0)
#   define LOG_ERROR(...) do {} while (0)
#endif

#endif /* engine_log_h */
