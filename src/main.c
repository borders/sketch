#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constraint.h"
#include "sketch_types.h"
#include "solver.h"

int main(void) 
{
	coord_2D_t pt1, pt2;

	sketch_line_t *l1, *l2, *l3;
	l1 = sketch_line_alloc();
	l2 = sketch_line_alloc();
	l3 = sketch_line_alloc();

	pt1.x = 0.0;
	pt1.y = 0.0;
	pt2.x = 0.1;
	pt2.y = 2.0;
	sketch_line_init(l1, &pt1, &pt2);

	pt1.x = 3.0;
	pt1.y = 3.0;
	pt2.x = 4.0;
	pt2.y = 4.0;
	sketch_line_init(l2, &pt1, &pt2);

	pt1.x = 10.0;
	pt1.y = 2.0;
	pt2.x = 3.0;
	pt2.y = -1.0;
	sketch_line_init(l3, &pt1, &pt2);
	
	constraint_t *constraints[3];

	constraints[0] = constraint_alloc();
	constraints[1] = constraint_alloc();
	constraints[2] = constraint_alloc();

	constraint_init_line_length(constraints[0], l1, 2.0);
	constraint_init_line_vert  (constraints[1], l2);
	constraint_init_line_horiz (constraints[2], l3);

	double cost;

	cost = constraint_cost(constraints[0]);
	printf("c1 cost: %lg\n", cost);

	cost = constraint_cost(constraints[0]);
	printf("c1 cost: %lg\n", cost);

	cost = constraint_cost(constraints[1]);
	printf("c2 cost: %lg\n", cost);
	
	cost = constraint_cost(constraints[2]);
	printf("c3 cost: %lg\n", cost);

	parm_map_t *map;
	map = parm_map_alloc();
	parm_map_init(map, constraints, 3);
	printf("parm_map size: %d\n", map->size);

	parm_map_print(map, stdout, constraints, 3);

	return 0;
}

