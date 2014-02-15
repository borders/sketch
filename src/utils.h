#if !defined(UTILS_H_)
#define UTILS_H_

#include <stdio.h>


#if 0
#define ERROR(fs, ...) \
	fprintf (stderr, "ERROR::" __FILE__ "::" __LINE__ "::" fs, __VA_ARGS__)
#else
#define ERROR(fs, ...) \
	fprintf (stderr,fs, __VA_ARGS__)
#endif

#endif
