#include <stdio.h>
#include <stdarg.h>
#include "engine_log.h"

void log_print(const char *format, ...)
{
    va_list none;
    vfprintf (stdout, "[debug] ", none);
    va_list arg;
    int done;

    va_start (arg, format);
    done = vfprintf (stdout, format, arg);
    va_end (arg);
    vfprintf (stdout, "\n", none);
}

void log_print_error(const char *format, ...)
{
    va_list none;
    vfprintf (stdout, "[error] ", none);
    va_list arg;
    int done;

    va_start (arg, format);
    done = vfprintf (stdout, format, arg);
    va_end (arg);
    vfprintf (stdout, "\n", none);
}
