#ifndef ASSERT_H
#define ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#define assert_debug(cond, msg)                                                \
	(!(cond) ? assert_fail((msg), __FILE__, __LINE__, __PRETTY_FUNCTION__)     \
			 : (void)((char*)(msg)))
#define assert(cond)                                                           \
	(!(cond) ? assert_fail("Assertion failed: " #cond, __FILE__, __LINE__,     \
						   __PRETTY_FUNCTION__)                                \
			 : (void)(0))

// clang-format off
__attribute__((noreturn)) 
void assert_fail(const char* message, const char* file, int line, const char* func);
// clang-format on

#ifdef __cplusplus
}
#endif

#endif	// ASSERT_H