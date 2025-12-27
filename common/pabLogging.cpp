#include "common/pabLogging.h"
#include <stdarg.h>
#include <pthread.h>

static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char* level_to_string(LogLevel level) {
    switch (level) {
        case PAB_LOGLEVEL__INFO:    return "INFO";
        case PAB_LOGLEVEL__WARNING: return "WARNING";
        case PAB_LOGLEVEL__ERROR:   return "ERROR";
        default:          return "UNKNOWN";
    }
}

void logger_log(LogLevel level, const char* fmt, ...) {
    pthread_mutex_lock(&log_mutex);
    printf("[%s] ", level_to_string(level));
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    pthread_mutex_unlock(&log_mutex);
}
