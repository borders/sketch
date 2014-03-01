#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "sketch_types.h"
#include "utils.h"

const color_t color_const_black = {0.0, 0.0, 0.0};
const color_t color_const_red   = {1.0, 0.0, 0.0};
const color_t color_const_green = {0.0, 1.0, 0.0};
const color_t color_const_blue  = {0.0, 0.0, 1.0};
const color_t color_const_white = {1.0, 1.0, 1.0};

static void sketch_base_init(sketch_base_t *self, sketch_shape_type_t type)
{
	assert(type >= 0 && type < NUM_SHAPE_TYPES);
	self->type = type;

	//self->constraints.head = NULL;
	//self->constraints.tail = NULL;

	self->is_selected = 0;
	self->is_highlighted = 0;

	self->line_width = 1.0;
	self->line_type = LINE_TYPE_SOLID;
	self->line_color = color_const_black;
}

sketch_line_t *sketch_line_alloc(void)
{
	sketch_line_t *self = calloc(1, sizeof(sketch_line_t));
	if(self == NULL) {
		ERROR("Out of memory");
		return NULL;
	}
	return self;
}

sketch_arc_t *sketch_arc_alloc(void)
{
	sketch_arc_t *self = calloc(1, sizeof(sketch_arc_t));
	if(self == NULL) {
		ERROR("Out of memory");
		return NULL;
	}
	return self;
}

sketch_circle_t *sketch_circle_alloc(void)
{
	sketch_circle_t *self = calloc(1, sizeof(sketch_circle_t));
	if(self == NULL) {
		ERROR("Out of memory");
		return NULL;
	}
	return self;
}

int sketch_line_fini(sketch_line_t *self)
{
	return 0;
}

int sketch_arc_fini(sketch_arc_t *self)
{
	return 0;
}

int sketch_circle_fini(sketch_circle_t *self)
{
	return 0;
}

void sketch_line_free(sketch_line_t *self)
{
	free(self);
}

void sketch_arc_free(sketch_arc_t *self)
{
	free(self);
}

void sketch_circle_free(sketch_circle_t *self)
{
	free(self);
}

void sketch_line_init(sketch_line_t *self, 
                      coord_2D_t const *v1, coord_2D_t const *v2)
{
	sketch_base_init( (sketch_base_t *)self, SHAPE_TYPE_LINE);
	if (v1 == NULL) {
		self->v1.x = 0.0;
		self->v1.y = 0.0;
	} else {
		self->v1 = *v1;
	}
	if (v2 == NULL) {
		self->v2.x = 0.0;
		self->v2.y = 0.0;
	} else {
		self->v2 = *v2;
	}
}

void sketch_line_get_point_angle_len(sketch_line_t *self, 
                         coord_2D_t *point, double *theta, double *length)
{
	double dx = self->v2.x - self->v1.x;
	double dy = self->v2.y - self->v1.y;
	point->x = self->v1.x;
	point->y = self->v1.y;
	if(dx == 0.0) {
		if(dy == 0.0) {
			*length = 0.0;
			*theta = 0.0;
		} else if(dy > 0.0) {
			*length = dy;
			*theta = M_PI / 2.0;
		} else { // dy < 0.0
			*length = -dy;
			*theta = -M_PI / 2.0;
		}
	} else if(dy == 0.0) {
		if(dx > 0.0) {
			*length = dx;
			*theta = 0.0;
		} else { // dx < 0.0
			*length = -dx;
			*theta = M_PI;
		}
	} else {
		*theta = atan2(dy, dx);
		*length = sqrt(dx*dx + dy*dy);
	}
}

void sketch_circle_init(sketch_circle_t *self, 
                        coord_2D_t const *center, double radius)
{
	sketch_base_init( (sketch_base_t *)self, SHAPE_TYPE_CIRCLE);
	if (center == NULL) {
		self->center.x = 0.0;
		self->center.y = 0.0;
	} else {
		self->center = *center;
	}
	self->radius = radius;
}

