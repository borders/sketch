#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <math.h>
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

	double (*cost)(constraint_t *);
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

static double cost_point_x_coord(constraint_t *self)
{
	return 0.0;
}

static double cost_point_y_coord(constraint_t *self)
{
	return 0.0;
}

static double cost_line_length(constraint_t *self)
{
	/*
		arg1 = sqrt( (x2-x1)^2 + (y2-y1)^2 )
	*/
	double dx = self->line1->v2.x - self->line1->v1.x;
	double dy = self->line1->v2.y - self->line1->v1.y;
	double L = sqrt(dx * dx + dy * dy);
	double err = self->arg1 - L;
	return (err*err);
}

static double cost_line_horiz(constraint_t *self)
{
	/*
		y1 = y2
	*/
	double err = self->line1->v2.y - self->line1->v1.y;
	return (err*err);
}

static double cost_line_vert(constraint_t *self)
{
	/*
		x1 = x2
	*/
	double err = self->line1->v2.x - self->line1->v1.x;
	return (err*err);
}

static double cost_dummy(constraint_t *self)
{
	return 0.0;
}

int constraint_init(constraint_t *self, constraint_type_t type)
{
	assert(self != NULL);
	assert(type >= 0 && type < NUM_CONSTRAINT_TYPES);

	self->type = type;
	strcpy(self->name, "");
	self->suppressed = 0;

	switch(self->type) {
		case CT_POINT_X_COORD:
			self->cost = &cost_point_x_coord;
			break;
		case CT_POINT_Y_COORD:
			self->cost = &cost_point_y_coord;
			break;
		case CT_LINE_LENGTH:
			self->cost = &cost_line_length;
			break;
		case CT_LINE_HORIZ:
			self->cost = &cost_line_horiz;
			break;
		case CT_LINE_VERT:
			self->cost = &cost_line_vert;
			break;
		default:
			self->cost = &cost_dummy;
	}
	
	return 0;
}

int constraint_fini(constraint_t *self)
{
	return 0;
}

void constraint_free(constraint_t *self)
{
	free(self);
}


double constraint_cost(constraint_t *self)
{
	return self->cost(self);
}

int constraint_init_line_length(constraint_t *self, sketch_line_t *line, 
                                double length)
{
	int ret = constraint_init(self, CT_LINE_LENGTH);
	if (ret != 0)
		return ret;
	self->line1 = line;
	self->arg1 = length;
	return 0;
}


int constraint_init_line_horiz(constraint_t *self, sketch_line_t *line)
{
	int ret = constraint_init(self, CT_LINE_HORIZ);
	if (ret != 0)
		return ret;
	self->line1 = line;
	return 0;
}

int constraint_init_line_vert(constraint_t *self, sketch_line_t *line)
{
	int ret = constraint_init(self, CT_LINE_VERT);
	if (ret != 0)
		return ret;
	self->line1 = line;
	return 0;
}
