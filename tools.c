#include "tools.h"

char *xstrncat(char *dest, const char *src, size_t n) {
	int free;

	free = n - strlen(dest) - 1;

	if (free > 0)
		strncat(dest, src, free);

	return dest;
}
