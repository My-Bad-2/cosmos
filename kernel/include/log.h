#ifndef LIBS_LOG_H
#define LIBS_LOG_H

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

// supported colors
#define LOG_COLOR_NORMAL "\x1B[0m"
#define LOG_COLOR_RED "\x1B[31m"
#define LOG_COLOR_GREEN "\x1B[32m"
#define LOG_COLOR_YELLOW "\x1B[33m"
#define LOG_COLOR_BLUE "\x1B[34m"
#define LOG_COLOR_MAGENTA "\x1B[35m"
#define LOG_COLOR_CYAN "\x1B[36m"
#define LOG_COLOR_WHITE "\x1B[37m"
#define LOG_COLOR_RESET "\033[0m"

#define LOG_EMPTY ""

enum log_flags {
	LOG_NOTAG = (1 << 0),
	LOG_NOTE = (1 << 1),
	LOG_INFO = (1 << 2),
	LOG_WARN = (1 << 3),
	LOG_DEBUG = (1 << 4),
	LOG_TRACE = (1 << 5),
	LOG_ERROR = (1 << 6),
	LOG_FATAL = (1 << 7)
};

#ifdef __cplusplus
extern "C" {
#endif

void logger_log(enum log_flags tag, const char* file, int line, const char* fmt,
				...) __attribute__((format(PRINTF, 4, 5)));

#define log_notag(fmt, ...)                                                    \
	logger_log(LOG_NOTAG, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define log_note(fmt, ...)                                                     \
	logger_log(LOG_NOTE, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)                                                     \
	logger_log(LOG_INFO, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)                                                     \
	logger_log(LOG_WARN, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...)                                                    \
	logger_log(LOG_DEBUG, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define log_trace(fmt, ...)                                                    \
	logger_log(LOG_TRACE, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...)                                                    \
	logger_log(LOG_ERROR, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define log_fatal(fmt, ...)                                                    \
	logger_log(LOG_FATAL, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif	// LIBS_LOG_H