#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "strl.h"

#ifdef STRL_DEBUG_BACKTRACE
#include <execinfo.h>
#endif

#ifdef NEED_STRLCAT

size_t strlcat(char *dest, char *src, size_t n) {
	size_t fr;
	size_t ret;

	ret = strlen(dest) + strlen(src);

	fr = n - strlen(dest) - 1;

	if (fr > 0)
		strncat(dest, src, fr);

#ifdef STRL_DEBUG
	if (ret >= n) {
		fprintf(stdout, "strlcat: buffer overflow avoided (%zd >= %zd)\n", ret, n);
#ifdef STRL_DEBUG_BACKTRACE
		{
			void* array[10];
			size_t size;
			char **strings;
			size_t i;
			char s[64];
			size_t sl;

			sl = (strlen(src) < 63)?strlen(src):63;
			strncpy(s, src, sl);
			s[63] = '\0';

			size = backtrace(array, 10);
			strings = backtrace_symbols(array, size);

			if (strings != NULL) {
				for (i=0; i<size; i++)
					fprintf(stdout, "  %s\n", strings[i]);
				free(strings);
			}
			fprintf(stdout, "Source buffer: %s%s\n", s, (ret > 63)?"...":"");
		}
#endif
		fflush(stdout);
	}
#endif

	return ret;
}

#endif

#ifdef NEED_STRLCPY

size_t strlcpy(char *dest, char *src, size_t n) {
	size_t ret;

	ret = strlen(src);
	if (n == 0)
		return ret;

	strncpy(dest, src, n-1);
	*(dest+n-1) = '\0';

#ifdef STRL_DEBUG
	if (ret >= n) {
		fprintf(stdout, "strlcpy: buffer overflow avoided (%zd >= %zd)\n", ret, n);
#ifdef STRL_DEBUG_BACKTRACE
		{
			void* array[10];
			size_t size;
			char **strings;
			size_t i;
			char s[64];
			size_t sl;

			sl = (ret < 63)?ret:63;
			strncpy(s, src, sl);
			s[63] = '\0';

			size = backtrace(array, 10);
			strings = backtrace_symbols(array, size);

			if (strings != NULL) {
				for (i=0; i<size; i++)
					fprintf(stdout, "  %s\n", strings[i]);
				free(strings);
			}
			fprintf(stdout, "Source buffer: %s%s\n", s, (ret > 63)?"...":"");
		}
#endif
		fflush(stdout);
	}
#endif

	return ret;
}

#endif
