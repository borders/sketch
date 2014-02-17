#include <stdio.h>
#include <stdlib.h>

#include "solver.h"
#include "utils.h"

static double 
f_eval(const gsl_vector *v, void *params)
{
	return 0.0;
}

static void 
df_eval(const gsl_vector *v, void *params, gsl_vector *df)
{
	int i;
	solver_t *s = params;
	for(i=0; i < s->size; i++) {
		gsl_vector_set(df, i, 0.0);
	}
}

static void 
fdf_eval(const gsl_vector *x, void *params, double *f, gsl_vector *df) 
{
	*f = f_eval(x, params);
	df_eval(x, params, df);
}

solver_t *solver_alloc(void)
{
	solver_t *self = calloc(1, sizeof(solver_t));
	if(self == NULL) {
		ERROR("Out of memory");
		return NULL;
	}

	self->map = parm_map_alloc();
	if(self->map == NULL) {
		free(self);
		ERROR("Out of memory");
		return NULL;
	}
	return self;
}

int solver_init(solver_t *self, constraint_t *c[], int c_count)
{
	int i;

	// store references to constraints
	self->c = c;
	self->c_count = c_count;

	// initialize the parm_map, which will determine the size of the solver
	parm_map_init(self->map, (const constraint_t **)c, c_count);

	self->size = self->map->size;
	self->s = gsl_multimin_fdfminimizer_alloc(
			gsl_multimin_fdfminimizer_vector_bfgs2, self->size);
	self->x = gsl_vector_alloc(self->size);
	self->x_0 = gsl_vector_alloc(self->size);

	// configure func struct
	self->func.n = self->size;
	self->func.f = f_eval;
	self->func.df = df_eval;
	self->func.fdf = fdf_eval;
	self->func.params = self;

	gsl_multimin_fdfminimizer_set(self->s, &(self->func), self->x, 0.01, 1e-4);

	// make the vector that will hold the initial parameter values
	for(i=0; i < self->map->size; i++) {
		gsl_vector_set(self->x_0, i, *(self->map->values[i]) );
	}
	
	return 0;
}

void solver_free(solver_t *self)
{
	free(self);
}

int solver_fini(solver_t *self)
{
	return 0;
}

parm_map_t *parm_map_alloc(void)
{
	parm_map_t *self = calloc(1, sizeof(parm_map_t));
	if(self == NULL) {
		ERROR("Out of memory");
		return NULL;
	}
	return self;
}

void parm_map_free(parm_map_t *self)
{
	free(self);
}

int parm_map_fini(parm_map_t *self)
{
	return 0;
}

static void map_add(parm_map_t *self, const double *val)
{
	int i;
	for(i=0; i < self->size; i++) {
		if(val == self->values[i]) {
			// value already in map -> we're done here...
			DEBUG2("Entry %p already exists in parm_map", (void *)val);
			return;
		}
	}
	if(self->size >= MAP_CAPACITY) {
		ERROR("parm_map overflow");
		return;
	}
	DEBUG2("Adding entry to parm_map: %p", (void *)val);
	self->values[self->size++] = (double *)val;
	return;
}

void parm_map_print(parm_map_t *self, FILE *fp, constraint_t *c[], int c_count)
{
	int i;
	for(i=0; i < self->size; i++) {
		fprintf(fp, "param %d: %p\n", i, (void*)(self->values[i]));
		if(c != NULL) {
			int j;
			for(j=0; j < c_count; j++) {
				if(self->values[i] == &(c[j]->line1->v1.x) ||
						self->values[i] == &(c[j]->line1->v1.y) ||
						self->values[i] == &(c[j]->line1->v2.x) ||
						self->values[i] == &(c[j]->line1->v2.y) ||
						self->values[i] == &(c[j]->point1->x) ||
						self->values[i] == &(c[j]->point1->y) ||
						self->values[i] == &(c[j]->point2->x) ||
						self->values[i] == &(c[j]->point2->y) ||
						self->values[i] == &(c[j]->arc1->v1.x) ||
						self->values[i] == &(c[j]->arc1->v1.y) ||
						self->values[i] == &(c[j]->arc1->v2.x) ||
						self->values[i] == &(c[j]->arc1->v2.y) ||
						self->values[i] == &(c[j]->arc1->center.x) ||
						self->values[i] == &(c[j]->arc1->center.y) ||
						self->values[i] == &(c[j]->arc2->v1.x) ||
						self->values[i] == &(c[j]->arc2->v1.y) ||
						self->values[i] == &(c[j]->arc2->v2.x) ||
						self->values[i] == &(c[j]->arc2->v2.y) ||
						self->values[i] == &(c[j]->arc2->center.x) ||
						self->values[i] == &(c[j]->arc2->center.y) ) 
				{
					fprintf(fp, "constraint match: %d\n", j);
				}
			}
		}
	}
}

int parm_map_init(parm_map_t *self, const constraint_t *c[], int c_count)
{
	DEBUG2("init'ing parm_map...");
	int i;
	self->size = 0;
	for(i=0; i < c_count; i++) {
		switch(c[i]->type) {
		case CT_LINE_LENGTH:
			map_add(self, &(c[i]->line1->v1.x) );
			map_add(self, &(c[i]->line1->v1.y) );
			map_add(self, &(c[i]->line1->v2.x) );
			map_add(self, &(c[i]->line1->v2.y) );
			break;
		case CT_LINE_HORIZ:
			map_add(self, &(c[i]->line1->v1.y) );
			map_add(self, &(c[i]->line1->v2.y) );
			break;
		case CT_LINE_VERT:
			map_add(self, &(c[i]->line1->v1.x) );
			map_add(self, &(c[i]->line1->v2.x) );
			break;
		case CT_POINT_POINT_COINCIDENT:
		case CT_POINT_POINT_DIST:
			map_add(self, &(c[i]->point1->x) );
			map_add(self, &(c[i]->point1->y) );
			map_add(self, &(c[i]->point2->x) );
			map_add(self, &(c[i]->point2->y) );
			break;
		case CT_POINT_POINT_X_DIST:
			map_add(self, &(c[i]->point1->x) );
			map_add(self, &(c[i]->point2->x) );
			break;
		case CT_POINT_POINT_Y_DIST:
			map_add(self, &(c[i]->point1->y) );
			map_add(self, &(c[i]->point2->y) );
			break;
		default:
			ERROR("Constraint type not yet supported");
		}
		
	}
}
