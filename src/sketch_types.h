#if !defined(SKETCH_TYPES_H_)
#define SKETCH_TYPES_H_

/* some forward declarations */
typedef struct _coord_2D      coord_2D_t;
typedef struct _sketch_line   sketch_line_t;
typedef struct _sketch_arc    sketch_arc_t;
typedef struct _sketch_circle sketch_circle_t;

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

	//constraint_list_t constraints;

	char is_selected;
	char is_highlighted;

	double line_width;
	line_type_t line_type;
	color_t line_color;
} sketch_base_t;

struct _coord_2D {
	double x;
	double y;
};

struct _sketch_line {
	sketch_base_t base;
	coord_2D_t v1;
	coord_2D_t v2;
};
	
struct _sketch_arc {
	sketch_base_t base;
	coord_2D_t v1;
	coord_2D_t v2;
	coord_2D_t center;
};
	
struct _sketch_circle {
	sketch_base_t base;
	coord_2D_t center;
	double radius;
};

/* color constants */	
extern const color_t color_const_black;
extern const color_t color_const_red;
extern const color_t color_const_green;
extern const color_t color_const_blue;
extern const color_t color_const_white;

/* func protos */
sketch_line_t *sketch_line_alloc(void);
sketch_arc_t *sketch_arc_alloc(void);
sketch_circle_t *sketch_circle_alloc(void);

void sketch_line_init(sketch_line_t *self, 
                      coord_2D_t const *v1, coord_2D_t const *v2);
void sketch_circle_init(sketch_circle_t *self, 
                        coord_2D_t const *center, double radius);

void sketch_line_get_point_angle_len(sketch_line_t *self, 
                         coord_2D_t *point, double *theta, double *length);

int sketch_line_fini(sketch_line_t *self);
int sketch_arc_fini(sketch_arc_t *self);
int sketch_circle_fini(sketch_circle_t *self);

void sketch_line_free(sketch_line_t *self);
void sketch_arc_free(sketch_arc_t *self);
void sketch_circle_free(sketch_circle_t *self);
#endif
