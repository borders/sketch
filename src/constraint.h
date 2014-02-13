#if !defined(SKETCH_CONSTRAINT_H_)
#define SKETCH_CONSTRAINT_H_

#include "sketch_types.h"

typedef enum {
	CONSTRAINT_TYPE_POINTS_COINCIDENT,
	CONSTRAINT_TYPE_POINTS_X_DIST,
	CONSTRAINT_TYPE_POINTS_Y_DIST,
	CONSTRAINT_TYPE_POINTS_DIST,

	CONSTRAINT_TYPE_LINE_HORIZ,
	CONSTRAINT_TYPE_LINE_VERT,
	CONSTRAINT_TYPE_LINE_LENGTH,

	CONSTRAINT_TYPE_LINES_ANGLE,
	CONSTRAINT_TYPE_LINES_PARALLEL,
	CONSTRAINT_TYPE_LINES_ORTHOG,
	CONSTRAINT_TYPE_LINES_COLLINEAR,

	CONSTRAINT_TYPE_LINE_POINT_MIDPOINT,
	CONSTRAINT_TYPE_LINE_POINT_COINCIDENT,
	CONSTRAINT_TYPE_LINE_POINT_DIST,

	CONSTRAINT_TYPE_ARC_RADIUS,

	CONSTRAINT_TYPE_LINE_ARC_TANGENT,
	CONSTRAINT_TYPE_ARC_ARC_TANGENT,

	CONSTRAINT_TYPE_ARCS_EQUAL,
	CONSTRAINT_TYPE_ARCS_CONCENTRIC,

	NUM_CONSTRAINT_TYPES
} constraint_type_t;

typedef struct {
	constraint_type_t type;
	char name[255];
	char suppresed;
} constraint_t;

typedef struct _constraint_list_item {
	constraint_t *constraint;
	struct _constraint_list_item *next;
} constraint_list_item_t;

typedef struct {
	constraint_list_item_t *head;
	constraint_list_item_t *tail;
} constraint_list_t;

#endif
