#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "constraint.h"
#include "utils.h"

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
	double dx = self->line1->v2->x - self->line1->v1->x;
	double dy = self->line1->v2->y - self->line1->v1->y;
	double L = sqrt(dx * dx + dy * dy);
	double err = self->arg1 - L;
	return (err*err);
}

static double cost_line_horiz(constraint_t *self)
{
	/*
		y1 = y2
	*/
	double err = self->line1->v2->y - self->line1->v1->y;
	return (err*err);
}

static double cost_line_vert(constraint_t *self)
{
	/*
		x1 = x2
	*/
	double err = self->line1->v2->x - self->line1->v1->x;
	return (err*err);
}


static double cost_p_p_coinc(constraint_t *self)
{
	/*
		x1 = x2
		y1 = y2

		C = (x2-x1)^2 + (y2-y1)^2
	*/
	double dx = self->point2->x - self->point1->x;
	double dy = self->point2->y - self->point1->y;
	return (dx*dx + dy*dy);
}

static double cost_p_p_dist(constraint_t *self)
{
	/*
		sqrt( (x2-x1)^2 + (y2-y1)^2 ) = L
	*/
	double dx = self->point2->x - self->point1->x;
	double dy = self->point2->y - self->point1->y;
	double L = sqrt(dx * dx + dy * dy);
	double err = self->arg1 - L;
	return (err*err);
}

static double cost_l_l_angle(constraint_t *self)
{
	/*
		A . B = |A| * |B| * cos(q) 
	*/
	return (0.0);
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
		case CT_POINT_POINT_COINCIDENT:
			self->cost = &cost_p_p_coinc;
			break;
		case CT_POINT_POINT_DIST:
			self->cost = &cost_p_p_dist;
			break;
		case CT_LINE_LINE_ANGLE:
			self->cost = &cost_l_l_angle;
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

int constraint_init_p_p_coinc(constraint_t *self, sketch_point_t *p1, sketch_point_t *p2)
{
	int ret = constraint_init(self, CT_POINT_POINT_COINCIDENT);
	if (ret != 0)
		return ret;
	self->point1 = p1;
	self->point2 = p2;
	return 0;
}

int constraint_init_p_p_dist(constraint_t *self, sketch_point_t *p1, sketch_point_t *p2, double dist)
{
	int ret = constraint_init(self, CT_POINT_POINT_DIST);
	if (ret != 0)
		return ret;
	self->point1 = p1;
	self->point2 = p2;
	self->arg1 = dist;
	return 0;
}
