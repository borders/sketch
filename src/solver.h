#if !defined(SOLVER_H_)
#define SOLVER_H_

#include <gsl/gsl_multimin.h>
#include "constraint.h"

#define MAP_CAPACITY 100

struct _parm_map {
	double *values[MAP_CAPACITY];
	int size;
};
typedef struct _parm_map parm_map_t;

struct _solver {
	int size;
	gsl_multimin_fdfminimizer *s;
	gsl_multimin_function_fdf func;
	gsl_vector *x;
	gsl_vector *x_0;
	parm_map_t *map;
	constraint_t **c;
	int c_count;
};
typedef struct _solver solver_t;

solver_t *solver_alloc(void);
int solver_init(solver_t *self, constraint_t *c[], int c_count);
int solver_fini(solver_t *self);
void solver_free(solver_t *self);


parm_map_t *parm_map_alloc(void);
int parm_map_init(parm_map_t *self, const constraint_t *c[], int c_count);
int parm_map_fini(parm_map_t *self);
void parm_map_free(parm_map_t *self);

void parm_map_print(parm_map_t *self, FILE *fp, constraint_t *c[], int c_count);
#endif
