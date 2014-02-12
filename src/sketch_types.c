#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "sketch_types.h"

const color_t color_const_black = {0.0, 0.0, 0.0};
const color_t color_const_red   = {1.0, 0.0, 0.0};
const color_t color_const_green = {0.0, 1.0, 0.0};
const color_t color_const_blue  = {0.0, 0.0, 1.0};
const color_t color_const_white = {1.0, 1.0, 1.0};

static void sketch_base_init(sketch_base_t *self, sketch_shape_type_t type)
{
	assert(type >= 0 && type < NUM_SHAPE_TYPES);
	self->type = type;

	self->constraints.head = NULL;
	self->constraints.tail = NULL;

	self->is_selected = 0;

	self->line_width = 1.0;
	self->line_type = LINE_TYPE_SOLID;
	self->line_color = color_const_black;
}

void sketch_line_init(sketch_line_t *self, 
                      coord_2D_t const *v1, coord_2D_t const *v2)
{
	sketch_shape_init( (sketch_base_t *)self, SHAPE_TYPE_LINE);
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

void sketch_circle_init(sketch_circle_t *self, 
                        coord_2D_t const *center, double radius)
{
	sketch_shape_init( (sketch_base_t *)self, SHAPE_TYPE_CIRCLE);
	if (center == NULL) {
		self->center.x = 0.0;
		self->center.y = 0.0;
	} else {
		self->center = *center;
	}
	self->radius = radius;
}

