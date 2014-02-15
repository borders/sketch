#if !defined(UTILS_H_)
#define UTILS_H_

#include <stdio.h>

/* standard stringification macros */
#define STR(x) #x
#define STR_(x) STR(x)

#define ERROR(...) \
	fprintf (stderr, "ERROR::" __FILE__ "::" STR_(__LINE__) "::" __VA_ARGS__)

#endif
