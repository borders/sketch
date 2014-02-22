#if !defined(GUI_H_)
#define GUI_H_

#include <gtk/gtk.h>

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

struct _gui {
	GtkWidget *window;
	GtkWidget *top_level_vbox;
	GtkWidget *canvas;
	struct _button_bar button_bar;
	struct _status_bar status_bar;
};

typedef struct _gui gui_t;

int gui_init(gui_t *self, int *argc, char ***argv);
void gui_run(gui_t *self);

#endif
