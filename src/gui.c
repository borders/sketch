#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <gtk/gtk.h>

#include "gui.h"
#include "main.h"

static void select_cb(GtkButton *b, gpointer data)
{
	printf("select button clicked!\n");
}

static void line_cb(GtkButton *b, gpointer data)
{
	printf("line button clicked!\n");
}

static void arc_cb(GtkButton *b, gpointer data)
{
	printf("arc button clicked!\n");
}

gboolean draw_canvas(GtkWidget *widget, GdkEventExpose *event, gpointer data) 
{
	gui_t *gui = (gui_t *)data;
	printf("drawing canvas...\n");
   draw_ptr dp = gui->drawer;
   draw_start(dp);
   int i;

   float width, height;
   draw_get_canvas_dims(dp, &width, &height);

   // first, fill with background color
   draw_set_color(dp, 1,1,1);
   draw_set_line_width(dp, 1);
   draw_rectangle_filled(dp, 0, 0, width, height);

	// draw sketch objects
	for(i = 0; i < app_data.sketch_count; i++) {
		printf("drawing sketch object %d of %d...\n", i+1, app_data.sketch_count);
	}

   draw_finish(dp);
   return TRUE;
}

int gui_init(gui_t *self, int *argc, char ***argv)
{
	struct _button_bar *bb = &(self->button_bar);
	struct _status_bar *sb = &(self->status_bar);
	gtk_init(argc, argv);

   self->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	self->top_level_vbox = gtk_vbox_new(FALSE, 1);
   gtk_container_add(GTK_CONTAINER (self->window), self->top_level_vbox);

	bb->hbox = gtk_hbox_new(FALSE, 1);
	gtk_box_pack_start(GTK_BOX(self->top_level_vbox), bb->hbox, TRUE, TRUE, 0);

	/* Select Button */
	bb->select_btn = gtk_button_new_with_label("Select");
	gtk_box_pack_start(GTK_BOX(bb->hbox), bb->select_btn, FALSE, FALSE, 0);
	g_signal_connect(bb->select_btn, "clicked", G_CALLBACK(select_cb), NULL);

	/* Line Button */
	bb->line_btn = gtk_button_new_with_label("Line");
	gtk_box_pack_start(GTK_BOX(bb->hbox), bb->line_btn, FALSE, FALSE, 0);
	g_signal_connect(bb->line_btn, "clicked", G_CALLBACK(line_cb), NULL);

	/* Arc Button */
	bb->arc_btn = gtk_button_new_with_label("Arc");
	gtk_box_pack_start(GTK_BOX(bb->hbox), bb->arc_btn, FALSE, FALSE, 0);
	g_signal_connect(bb->arc_btn, "clicked", G_CALLBACK(arc_cb), NULL);

	/* Canvas */
   self->canvas = gtk_drawing_area_new();
   gtk_widget_set_size_request(self->canvas, 500,400);
   g_signal_connect(self->canvas, "expose_event", G_CALLBACK(draw_canvas), 
	                 self);
	gtk_box_pack_start(GTK_BOX(self->top_level_vbox), self->canvas, 
	                   TRUE, TRUE, 0);

	/* drawing "context" */
	self->drawer = draw_create(self->canvas);

	/* Status Bar */
	sb->hbox = gtk_hbox_new(FALSE, 10);
   gtk_box_pack_start(GTK_BOX(self->top_level_vbox), sb->hbox, FALSE, FALSE, 0);
   sb->left_label = gtk_label_new("Status...");
   gtk_box_pack_start(GTK_BOX(sb->hbox), sb->left_label, FALSE, FALSE, 0);
   gtk_box_pack_start(GTK_BOX(sb->hbox), gtk_vseparator_new(), FALSE, FALSE, 0);
   sb->right_label = gtk_label_new("Hello World");
   gtk_box_pack_start(GTK_BOX(sb->hbox), sb->right_label, FALSE, FALSE, 0);

	/* Finalize the GUI setup */
	g_signal_connect(self->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
   gtk_widget_show_all(self->window);

	return 0;
}

void gui_run(gui_t *self)
{
	gtk_main();
}
