#if !defined(MAIN_H_)
#define MAIN_H_

#include "constraint.h"
#include "sketch_types.h"
#include "solver.h"

#define MAX_NUM_SHAPES 100
#define MAX_NUM_CONSTRAINTS 100

typedef struct 
{
	sketch_base_t *sketch[MAX_NUM_SHAPES];
	int sketch_count;

	constraint_t *constraints[MAX_NUM_CONSTRAINTS];
	int constraint_count;
	int constraints_dirty;

  solver_t *solver;
} app_data_t;

extern app_data_t app_data;

#endif
