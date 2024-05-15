#ifndef _STRING_H
#define _STRING_H

#include <stddef.h>
#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void* lhs, const void* rhs, size_t count);
void* memset(void* dest, int c, size_t count);
void* memcpy(void* restrict dest, const void* restrict src, size_t count);
void* memmove(void* dest, const void* src, size_t count);

size_t strlen(const char* str);
size_t strnlen(const char* str, size_t count);

char* strcpy(char* restrict dest, const char* restrict src);
char* strncpy(char* restrict dest, const char* restrict src, size_t count);

int strcmp(const char* lhs, const char* rhs);
int strncmp(const char* lhs, const char* rhs, size_t count);

char* strcat(char* restrict dst, const char* restrict src);
char* strncat(char* restrict dst, const char* restrict src, size_t count);

#ifdef __cplusplus
}
#endif
#endif	// _STRING_H