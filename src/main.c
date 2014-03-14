#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constraint.h"
#include "sketch_types.h"
#include "solver.h"
#include "gui.h"
#include "main.h"

sketch_line_t *lines[3];

app_data_t app_data = 
{
  {0}, 0,
  {0}, 0
};

int iterate_cb(int i, void *data)
{
  //printf("iterate_cb: %d\n", i);
  printf("%g %g %g %g  %g %g %g %g  %g %g %g %g\n",
      lines[0]->v1->x, lines[0]->v1->y,
      lines[0]->v2->x, lines[0]->v2->y,
      lines[1]->v1->x, lines[1]->v1->y,
      lines[1]->v2->x, lines[1]->v2->y,
      lines[2]->v1->x, lines[2]->v1->y,
      lines[2]->v2->x, lines[2]->v2->y);
  return 0;
}

int main(void) 
{
  coord_2D_t pt1, pt2;

  printf("here 1\n");

  sketch_line_t *l1, *l2, *l3;
  lines[0] = sketch_line_alloc();
  lines[1] = sketch_line_alloc();
  lines[2] = sketch_line_alloc();

  printf("here 2\n");

  pt1.x = 0.0;
  pt1.y = 0.0;
  pt2.x = 0.1;
  pt2.y = 2.0;
  sketch_line_init(lines[0], &pt1, &pt2);

  printf("here 3\n");

  pt1.x = 3.0;
  pt1.y = 3.0;
  pt2.x = 4.0;
  pt2.y = 4.0;
  sketch_line_init(lines[1], &pt1, &pt2);

  pt1.x = 10.0;
  pt1.y = 2.0;
  pt2.x = 3.0;
  pt2.y = -1.0;
  sketch_line_init(lines[2], &pt1, &pt2);
  
  printf("here 4\n");

  constraint_t *constraints[4];

  constraints[0] = constraint_alloc();
  constraints[1] = constraint_alloc();
  constraints[2] = constraint_alloc();
  constraints[3] = constraint_alloc();

  printf("here 5\n");

  constraint_init_line_length(constraints[0], lines[0], 2.0);
  printf("here 6\n");
  constraint_init_line_vert(  constraints[1], lines[1]);
  constraint_init_line_horiz( constraints[2], lines[2]);
  constraint_init_p_p_coinc(constraints[3], lines[0]->v1, lines[1]->v2);

  printf("here 7\n");

  double cost;

#if 0
  cost = constraint_cost(constraints[0]);
  printf("c1 cost: %lg\n", cost);

  cost = constraint_cost(constraints[0]);
  printf("c1 cost: %lg\n", cost);

  cost = constraint_cost(constraints[1]);
  printf("c2 cost: %lg\n", cost);
  
  cost = constraint_cost(constraints[2]);
  printf("c3 cost: %lg\n", cost);
#endif

  solver_t *solver;
  solver = solver_alloc();
  printf("here 8\n");
  solver_init(solver, constraints, 4);
  printf("here 9\n");
  solver_set_iterate_cb(solver, &iterate_cb, (void*)solver);
  printf("here 10\n");

  solver_solve(solver);

  gui_t gui;
  gui_init(&gui, NULL, NULL);
  gui_run(&gui);

  return 0;
}

