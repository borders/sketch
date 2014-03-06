#if !defined(UTILS_H_)
#define UTILS_H_

#include <stdio.h>

#ifndef DEBUG_LEVEL
	#define DEBUG_LEVEL 2
#endif

/* standard stringification macros */
#define STR(x) #x
#define STR_(x) STR(x)

#define ERROR(...) \
	fprintf (stderr, "ERROR ::" __FILE__ "::" STR_(__LINE__) ":: " __VA_ARGS__); \
	fprintf (stderr, "\n");

#define INFO(...) \
	fprintf (stderr, "INFO  ::" __FILE__ "::" STR_(__LINE__) ":: " __VA_ARGS__); \
	fprintf (stderr, "\n");


#if DEBUG_LEVEL >= 1 
	#define DEBUG(...) \
		fprintf (stdout, "DEBUG ::" __FILE__ "::" STR_(__LINE__) ":: " __VA_ARGS__); \
		fprintf (stdout, "\n");
#else
	#define DEBUG(...) 
#endif

#if DEBUG_LEVEL >= 2 
	#define DEBUG2(...) \
		fprintf (stdout, "DEBUG2::" __FILE__ "::" STR_(__LINE__) ":: " __VA_ARGS__); \
		fprintf (stdout, "\n");
#else
	#define DEBUG2(...) 
#endif

#endif
