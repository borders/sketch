#if !defined(SKETCH_TYPES_H_)
#define SKETCH_TYPES_H_

#include "constraint.h"

typedef enum {
	SHAPE_TYPE_LINE = 0,
	SHAPE_TYPE_ARC,
	SHAPE_TYPE_CIRCLE,

	NUM_SHAPE_TYPES
} sketch_shape_type_t;

typedef enum {
	LINE_TYPE_SOLID = 0,
	LINE_TYPE_DASHED,
	LINE_TYPE_DOTTED,

	NUM_LINE_TYPES
} line_type_t;

typedef struct {
	double r;
	double g;
	double b;
} color_t;

typedef struct _sketch_base {
	sketch_shape_type_t type;

	constraint_list_t constraints;

	char is_selected;

	double line_width;
	line_type_t line_type;
	color_t line_color;
} sketch_base_t;

typedef struct {
	double x;
	double y;
} coord_2D_t;

typedef struct {
	sketch_base_t base;
	coord_2D_t v1;
	coord_2D_t v2;
} sketch_line_t;
	
typedef struct {
	sketch_base_t base;
	coord_2D_t v1;
	coord_2D_t v2;
	coord_2D_t center;
} sketch_arc_t;
	
typedef struct {
	sketch_base_t base;
	coord_2D_t center;
	double radius;
} sketch_circle_t;

/* color constants */	
extern const color_t color_const_black;
extern const color_t color_const_red;
extern const color_t color_const_green;
extern const color_t color_const_blue;
extern const color_t color_const_white;

/* func protos */
void sketch_line_init(sketch_line_t *self, 
                      coord_2D_t const *v1, coord_2D_t const *v2);
void sketch_circle_init(sketch_circle_t *self, 
                        coord_2D_t const *center, double radius);

#endif
