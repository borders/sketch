#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constraint.h"
#include "sketch_types.h"
#include "solver.h"
#include "gui.h"
#include "main.h"

app_data_t app_data = 
{
  .sketch = {0}, 
  .sketch_count = 0,

  .constraints = {0}, 
  .constraint_count = 0,
  .constraints_dirty = 1,

  .solver = NULL
};

int main(void) 
{
  app_data.solver = solver_alloc();

  gui_t gui;
  gui_init(&gui, NULL, NULL);
  gui_run(&gui);

  return 0;
}

