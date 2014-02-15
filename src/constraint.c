#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include "constraint.h"
#include "utils.h"

struct _constraint {
	constraint_type_t type;
	char name[255];
	char suppressed;
	
	sketch_line_t *line1;
	sketch_line_t *line2;
	coord_2D_t point1;
	coord_2D_t point2;
	sketch_arc_t arc1;
	sketch_arc_t arc2;

	double arg1;
	double arg2;
};


constraint_t *constraint_alloc(void)
{
	constraint_t *c;
	c = calloc(1, sizeof(constraint_t));
	if (c == NULL) {
		ERROR("Out of memory");
		return NULL;
	}
	return c;
}

int constraint_init(constraint_t *self, constraint_type_t type)
{
	assert(self != NULL);
	assert(type >= 0 && type < NUM_CONSTRAINT_TYPES);

	self->type = type;
	strcpy(self->name, "");
	self->suppressed = 0;
	
	return 0;
}
