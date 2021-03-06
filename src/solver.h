#if !defined(SOLVER_H_)
#define SOLVER_H_

#include <gsl/gsl_multimin.h>
#include "constraint.h"

#define MAP_CAPACITY 100

#define FDF 0

#define MAX_ITERATIONS 100000
#define STOP_THRESH 1.0e-10

struct _parm_map 
{
	double *values[MAP_CAPACITY];
	int size;
};
typedef struct _parm_map parm_map_t;

typedef int iterate_cb_t(int i, void *data);

struct _solver 
{
	int size;
#if FDF
	gsl_multimin_fdfminimizer *s;
	gsl_multimin_function_fdf func;
#else
	gsl_multimin_fminimizer *s;
	gsl_multimin_function func;
#endif
	gsl_vector *x;
	gsl_vector *x_0;
	parm_map_t *map;
	constraint_t **c;
	int c_count;

	iterate_cb_t *iterate_cb;
	void *iterate_cb_data;
};
typedef struct _solver solver_t;

solver_t *solver_alloc(void);
int solver_init(solver_t *self, constraint_t *c[], int c_count);
int solver_fini(solver_t *self);
void solver_free(solver_t *self);
int solver_solve(solver_t *self);
int solver_set_initial(solver_t *self);

int solver_set_iterate_cb(solver_t *self, iterate_cb_t *cb, void *data);

parm_map_t *parm_map_alloc(void);
int parm_map_init(parm_map_t *self, const constraint_t *c[], int c_count);
int parm_map_fini(parm_map_t *self);
void parm_map_free(parm_map_t *self);

void parm_map_print(parm_map_t *self, FILE *fp, constraint_t *c[], int c_count);
#endif
