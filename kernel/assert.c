#include <assert.h>
#include <log.h>

__attribute__((noreturn)) void
assert_fail(const char* message, const char* file, int line, const char* func) {
	log_warn("File: %s", file);
	log_warn("Line: %d", line);
	log_warn("Function: %s", func);

	log_fatal("%s", message);

	while (true) {}
}