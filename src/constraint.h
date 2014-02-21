#if !defined(SKETCH_CONSTRAINT_H_)
#define SKETCH_CONSTRAINT_H_

#include "sketch_types.h"

typedef enum {
	CT_POINT_X_COORD = 0,
	CT_POINT_Y_COORD,

	CT_LINE_LENGTH,
	CT_LINE_HORIZ,
	CT_LINE_VERT,

	CT_POINT_POINT_COINCIDENT,
	CT_POINT_POINT_DIST,
	CT_POINT_POINT_X_DIST,
	CT_POINT_POINT_Y_DIST,

	CT_LINE_LINE_ANGLE,
	CT_LINE_LINE_PARALLEL,
	CT_LINE_LINE_ORTHOG,
	CT_LINE_LINE_COLLINEAR,
	CT_LINE_LINE_EQUAL,

	CT_LINE_POINT_COINCIDENT,
	CT_LINE_POINT_DIST,
	CT_LINE_POINT_MIDPOINT,

	CT_ARC_RADIUS,

	CT_LINE_ARC_TANGENT,
	CT_ARC_ARC_TANGENT,

	CT_ARCS_EQUAL,
	CT_ARCS_CONCENTRIC,

	NUM_CONSTRAINT_TYPES
} constraint_type_t;

typedef struct _constraint constraint_t;

struct _constraint {
	constraint_type_t type;
	char name[255];
	char suppressed;
	
	sketch_line_t *line1;
	sketch_line_t *line2;
	coord_2D_t *point1;
	coord_2D_t *point2;
	sketch_arc_t *arc1;
	sketch_arc_t *arc2;

	double arg1;
	double arg2;

	double (*cost)(constraint_t *);
};

typedef struct _constraint_list_item {
	constraint_t *constraint;
	struct _constraint_list_item *next;
} constraint_list_item_t;

typedef struct {
	constraint_list_item_t *head;
	constraint_list_item_t *tail;
	int length;
} constraint_list_t;

/***********************************************/

constraint_t *constraint_alloc(void);
int constraint_init(constraint_t *self, constraint_type_t type);
int constraint_fini(constraint_t *self);
void constraint_free(constraint_t *self);

double constraint_cost(constraint_t *self);

int constraint_init_line_length(constraint_t *self, sketch_line_t *line, 
                                double length);
int constraint_init_line_horiz(constraint_t *self, sketch_line_t *line);
int constraint_init_line_vert(constraint_t *self, sketch_line_t *line);
int constraint_init_p_p_coinc(constraint_t *self, coord_2D_t *p1, coord_2D_t *p2);
int constraint_init_p_p_dist(constraint_t *self, coord_2D_t *p1, coord_2D_t *p2, double dist);

#endif
