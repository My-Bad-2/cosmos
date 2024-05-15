#include <string.h>

typedef int word;

#undef wsize
#define wsize sizeof(word)

#undef wmask
#define wmask (wsize - 1)

void* memmove(void* dest, const void* src, size_t length) {
	register const char* from = src;
	register char* to = dest;
	register size_t t = 0;

	if (length == 0 || dest == src) {
		return dest;
	}

#undef TLOOP
#define TLOOP(s)                                                               \
	if (t) {                                                                   \
		TLOOP1(s);                                                             \
	}

#undef TLOOP1
#define TLOOP1(s)                                                              \
	do {                                                                       \
		s;                                                                     \
	} while (--t)

	if ((size_t)to < (size_t)from)	// Copy forward
	{
		// only need low bits
		t = (size_t)from;

		if ((t | (size_t)to) & wmask) {
			// Try to align operands. This cannot be done unless lower bits match.
			if ((t ^ (size_t)to) & wmask || (length < wsize)) {
				t = length;
			} else {
				t = wsize - (t & wmask);
			}

			length -= t;
			TLOOP1(*to++ = *from++);
		}

		// Copy whole words, then mop up any trailing bytes
		t = length / wsize;
		TLOOP(*(word*)to = *(word*)from; from += wsize; to += wsize);
		t = length & wmask;
		TLOOP(*to++ = *from++);
	} else	// Copy backwords
	{
		from += length;
		to += length;
		t = (size_t)from;

		// Try to align operands. This cannot be done unless lower bits match.
		if ((t | (size_t)to) & wmask) {
			if ((t ^ (size_t)to) & wmask || length <= wsize) {
				t = length;
			} else {
				t &= wmask;
			}

			length -= t;
			TLOOP1(*--to = *--from);
		}

		// Copy whole words, then mop up any trailing bytes
		t = length / wsize;
		TLOOP(from -= wsize; to -= wsize; *(word*)to = *(word*)from);
		t = length & wmask;
		TLOOP(*--to = *--from);
	}

	return dest;
}