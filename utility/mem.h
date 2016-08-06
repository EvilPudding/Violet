#ifndef VIOLET_MEM_H
#define VIOLET_MEM_H

#include <string.h>

static inline void memswp(void *a, void *b, u32 sz, void *tmp)
{
	memcpy(tmp, a, sz);
	memcpy(a, b, sz);
	memcpy(b, tmp, sz);
}

#define memswp_s(a, b, sz, tmp) \
	assert(sz <= sizeof(tmp)); \
	memswp(a, b, sz, tmp);

#endif

