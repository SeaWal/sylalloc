#include <stdarg.h>
#include "logger.h"

int log_init(struct Logger* logger, const char* log_filename) {
    if(!log_filename) {
        logger->fp = stdout;
        return 0;
    }

    logger->fp = fopen(log_filename, "a");
    if(!logger->fp) {
        return -1;
    }

    return 0;
}

void log_close(struct Logger* logger) {
    if(logger->fp && logger->fp != stdout) {
        fclose(logger->fp);
        logger->fp = NULL;
    }
}

void log_msg(struct Logger* logger, const char* msg, ...) {
    va_list args;
    va_start(args, msg);
    if(!logger->fp) {
        vfprintf(stderr, "Logger not found, cannot log message.\n", args);
        return;
    }

    vfprintf(logger->fp, msg, args);
    va_end(args);
}

#define LOG(logger, msg) log_msg((logger), "%s:%d (%s): %s", __FILE__, __LINE__+1, __func__, (msg))