#include "logger.h"

int log_init(struct Logger* logger, const char* log_filename) {
    if(!logger) return -1;

    if(!log_filename) {
        logger->fp = stdout;
        return 0;
    }

    logger->fp = fopen(log_filename, "a");
    if(!logger->fp) return -1;

    return 0;
}

void log_close(struct Logger* logger) {
    if(!logger) return;
    if(logger->fp && logger->fp != stdout) {
        fclose(logger->fp);
        logger->fp = NULL;
    }
}

void log_msg(struct Logger* logger, const char* msg, ...) {
    if(!logger || !logger->fp) {
        fprintf(stderr, "Logger not found, cannot log message.\n");
        return;
    }

    va_list args;
    va_start(args, msg);
    vfprintf(logger->fp, msg, args);
    fflush(logger->fp);
    va_end(args);
}