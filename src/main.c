#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constraint.h"
#include "sketch_types.h"

int main(void) 
{
	printf("hello world\n");

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
	
	constraint_t *c1, *c2, *c3;

	c1 = constraint_alloc();
	c2 = constraint_alloc();
	c3 = constraint_alloc();

	constraint_init_line_length(c1, l1, 2.0);
	constraint_init_line_vert(c2, l2);
	constraint_init_line_horiz(c3, l3);

	double cost;

	cost = constraint_cost(c1);
	printf("c1 cost: %lg\n", cost);

	cost = constraint_cost(c1);
	printf("c1 cost: %lg\n", cost);

	cost = constraint_cost(c2);
	printf("c2 cost: %lg\n", cost);
	
	cost = constraint_cost(c3);
	printf("c3 cost: %lg\n", cost);
	return 0;
}

