#include <stdio.h>
#include <stdlib.h>
#include "solver.h"
#include "utils.h"

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
