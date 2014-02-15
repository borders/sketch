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

typedef struct _constraint_list_item {
	constraint_t *constraint;
	struct _constraint_list_item *next;
} constraint_list_item_t;

typedef struct {
	constraint_list_item_t *head;
	constraint_list_item_t *tail;
	int length;
} constraint_list_t;

#endif
