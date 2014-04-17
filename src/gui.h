#if !defined(GUI_H_)
#define GUI_H_

#include <stdbool.h>

#include <gtk/gtk.h>
#include "draw.h"

#define MAX_SELECTIONS 100

typedef enum 
{
  TOOL_NONE,
  TOOL_LINE,
  TOOL_ARC
} tool_t;

struct _button_bar 
{
  GtkWidget *hbox;

  GtkWidget *select_btn;
  GtkWidget *line_btn;
  GtkWidget *arc_btn;
};

struct _tools_tb 
{
  GtkWidget *tb;

  GtkWidget *select_btn;
  GtkWidget *line_btn;
  GtkWidget *arc_btn;
};

struct _constraint_tb 
{
  GtkWidget *tb;

  GtkWidget *coinc_btn;
  GtkWidget *horiz_btn;
  GtkWidget *vert_btn;
  GtkWidget *parallel_btn;
  GtkWidget *perp_btn;
  GtkWidget *equal_btn;
};

struct _status_bar 
{
  GtkWidget *hbox;
  GtkWidget *left_label;
  GtkWidget *right_label;
};

typedef enum
{
  SELECT_TYPE_NONE,
  SELECT_TYPE_LINE,
  SELECT_TYPE_POINT,
  SELECT_TYPE_CONSTRAINT,

  NUM_SELECT_TYPES
} select_type_t;

typedef struct 
{
  select_type_t type;
  void *object;
} selection_t;


struct _state 
{
  bool draw_active;
  tool_t active_tool;
  double start_x;
  double start_y;
  double end_x;
  double end_y;

  selection_t selections[MAX_SELECTIONS];
  int selection_count;
};

#define MAX_NUM_MAJOR_TICS (50)
#define MAJOR_TIC_LABEL_SIZE  (150)
#define NUM_REQ_TICS (10)

typedef struct axis_t 
{
	int type; // 0=x, 1=y
	double major_tic_values[MAX_NUM_MAJOR_TICS];
	char major_tic_labels[MAX_NUM_MAJOR_TICS][MAJOR_TIC_LABEL_SIZE];
	double major_tic_delta;
	char tic_label_format_string[100];
	int num_actual_major_tics;
} axis_t;

struct _gui {
	draw_ptr drawer;
	GtkWidget *window;
	GtkWidget *top_level_vbox;
	GtkWidget *canvas;
	struct _button_bar button_bar;
  struct _tools_tb tools_tb;
  struct _constraint_tb constraint_tb;
	struct _status_bar status_bar;
	
	struct _state state;

	/* coefficients to convert from user coords to pixles and vice versa */
	double x_m;
	double y_m;
	double x_b;
	double y_b;

	/* user-space coordinate bounds */
	double xmin;
	double xmax;
	double ymin;
	double ymax;

	axis_t x_axis;
	axis_t y_axis;

	char panning;
	double pan_start_x;
	double pan_start_y;
	double pan_start_xmin;
	double pan_start_xmax;
	double pan_start_ymin;
	double pan_start_ymax;

	char dragging;
	double drag_start_x;
	double drag_start_y;
};

typedef struct _gui gui_t;

int gui_init(gui_t *self, int *argc, char ***argv);
void gui_run(gui_t *self);

#endif
