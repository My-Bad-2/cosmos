#include "arch.h"
#include <log.h>

static inline const char* logger_get_tag(enum log_flags tag) {
	switch (tag) {
		case LOG_NOTE:
			return "[N]";
		case LOG_INFO:
			return "[I]";
		case LOG_WARN:
			return "[W]";
		case LOG_DEBUG:
			return "[D]";
		case LOG_ERROR:
			return "[E]";
		case LOG_TRACE:
			return "[T]";
		case LOG_FATAL:
			return "[F]";
		default:
			break;
	}

	return NULL;
}

static inline const char* logger_get_color(enum log_flags tag) {
	switch (tag) {
		case LOG_NOTAG:
		case LOG_NOTE:
			return LOG_EMPTY;
		case LOG_INFO:
			return LOG_COLOR_GREEN;
		case LOG_WARN:
			return LOG_COLOR_YELLOW;
		case LOG_DEBUG:
			return LOG_COLOR_BLUE;
		case LOG_ERROR:
			return LOG_COLOR_RED;
		case LOG_TRACE:
			return LOG_COLOR_CYAN;
		case LOG_FATAL:
			return LOG_COLOR_MAGENTA;
		default:
			break;
	}

	return LOG_EMPTY;
}

static void logger_vlog(const char* prefix, const char* color, const char* file,
						int line, const char* fmt, va_list args) {
	fprintf(stderr, "%s%s <%s, %d> ", color, prefix, file, line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, LOG_COLOR_RESET "\n");
}

void logger_log(enum log_flags tag, const char* file, int line, const char* fmt,
				...) {
	const char* color = logger_get_color(tag);
	const char* tag_prefix = logger_get_tag(tag);

	if (tag_prefix == NULL) {
		tag_prefix = "";
	}

	va_list args;
	va_start(args, fmt);

	logger_vlog(tag_prefix, color, file, line, fmt, args);

	va_end(args);

	if (tag == LOG_FATAL) {
		arch_halt(false);
	}
}