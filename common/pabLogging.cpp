#include "common/pabLogging.h"
#include <stdarg.h>
#include <pthread.h>

// Static mutex equivalent to the private member
// PTHREAD_MUTEX_INITIALIZER avoids the need for an explicit init function
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
    // Lock (std::lock_guard equivalent)
    pthread_mutex_lock(&log_mutex);

    // Print the level tag
    printf("[%s] ", level_to_string(level));

    // Handle variadic arguments (Args&&... equivalent)
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args); // vprintf sends formatted output to stdout
    va_end(args);

    // Newline (std::endl equivalent)
    printf("\n");

    // Unlock
    pthread_mutex_unlock(&log_mutex);
}
