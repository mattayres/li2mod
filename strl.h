#ifndef __STRL_H
#define __STRL_H

#ifdef NEED_STRLCAT
size_t strlcat(char *dest, char *src, size_t n);
#endif

#ifdef NEED_STRLCPY
size_t strlcpy(char *dest, char *src, size_t n);
#endif

#endif
