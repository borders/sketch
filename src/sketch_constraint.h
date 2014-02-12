#if !defined(SKETCH_CONSTRAINT_H_)
#define SKETCH_CONSTRAINT_H_

#include "sketch_types.h"

typedef enum {
	CONSTRAINT_TYPE_POINT_ON_POINT,

	NUM_CONSTRAINT_TYPES
} sketch_constraint_type_t;

struct _sketch_base;

typedef struct {
	sketch_constraint_type_t type;
	struct _sketch_base *shape_1;
	struct _sketch_base *shape_2;
} sketch_constraint_t;

typedef struct _sketch_constraint_list_item {
	sketch_constraint_t *constraint;
	struct _sketch_constraint_list_item *next;
} sketch_constraint_list_item_t;

typedef struct {
	sketch_constraint_list_item_t *head;
	sketch_constraint_list_item_t *tail;
} sketch_constraint_list_t;

#endif
