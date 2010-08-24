#ifndef __STRL_H
#define __STRL_H

#ifdef NEED_STRLCAT
size_t strlcat(char *dest, char *src, size_t n);
#endif

#ifdef NEED_STRLCPY
size_t strlcpy(char *dest, char *src, size_t n);
#endif

#endif


#ifdef WIN32
#define snprintf(b,s,f,...) _snprintf_s(b,s,_TRUNCATE,f,__VA_ARGS__)
#define vsnprintf(b,s,f,...) vsnprintf_s(b,s,_TRUNCATE,f,__VA_ARGS__)
#define strdup(s) _strdup(s)
#define stricmp(s1,s2) _stricmp(s1,s2)
#endif
