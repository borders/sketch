#if !defined(SOLVER_H_)
#define SOLVER_H_

#include "constraint.h"

#define MAP_CAPACITY 100

struct _parm_map {
	double *values[MAP_CAPACITY];
	int size;
};
typedef struct _parm_map parm_map_t;

parm_map_t *parm_map_alloc(void);
int parm_map_init(parm_map_t *self, const constraint_t *c[], int c_count);
int parm_map_fini(parm_map_t *self);
void parm_map_free(parm_map_t *self);
#endif
