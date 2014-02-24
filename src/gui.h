#if !defined(GUI_H_)
#define GUI_H_

#include <stdbool.h>

#include <gtk/gtk.h>
#include "draw.h"

typedef enum {
	TOOL_NONE,
	TOOL_LINE,
	TOOL_ARC
} tool_t;

struct _button_bar {
	GtkWidget *hbox;
	GtkWidget *select_btn;
	GtkWidget *line_btn;
	GtkWidget *arc_btn;
};

struct _status_bar {
	GtkWidget *hbox;
	GtkWidget *left_label;
	GtkWidget *right_label;
};

struct _state {
	bool draw_active;
	tool_t active_tool;
	double start_x;
	double start_y;
	double end_x;
	double end_y;
};

struct _gui {
	draw_ptr drawer;
	GtkWidget *window;
	GtkWidget *top_level_vbox;
	GtkWidget *canvas;
	struct _button_bar button_bar;
	struct _status_bar status_bar;
	
	struct _state state;
};

typedef struct _gui gui_t;

int gui_init(gui_t *self, int *argc, char ***argv);
void gui_run(gui_t *self);

#endif
