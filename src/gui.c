#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <gtk/gtk.h>

#include "gui.h"
#include "main.h"
#include "sketch_types.h"

static void select_cb(GtkButton *b, gpointer data)
{
	gui_t *gui = (gui_t *)data;
	printf("select button clicked!\n");
	gui->state.draw_active = false;
	gui->state.active_tool = TOOL_NONE;
}

static void line_cb(GtkButton *b, gpointer data)
{
	gui_t *gui = (gui_t *)data;
	printf("line button clicked!\n");
	gui->state.draw_active = false;
	gui->state.active_tool = TOOL_LINE;
}

static void arc_cb(GtkButton *b, gpointer data)
{
	gui_t *gui = (gui_t *)data;
	printf("arc button clicked!\n");
	gui->state.draw_active = false;
	gui->state.active_tool = TOOL_ARC;
}

gboolean mouse_button_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	gui_t *gui = (gui_t *)data;
	//printf("mouse button callback!\n");
	switch(event->type) {
	case GDK_BUTTON_PRESS:
		//printf("  button pressed\n");

		if(gui->state.draw_active) {
			switch(gui->state.active_tool) {
			case TOOL_NONE:
				printf("Shouldn't be here!\n");
				break;
			case TOOL_LINE:
				printf("TODO: finish a line (create a line object)\n");
				double end_x, end_y;
				end_x = event->x;
				end_y = event->y;
				printf("  start: (%g,%g)  end: (%g,%g)\n",
				       gui->state.start_x, gui->state.start_y,
				       end_x, end_y);

				sketch_line_t *line = sketch_line_alloc();
				app_data.sketch[app_data.sketch_count++] = (sketch_base_t *)line;
				coord_2D_t start, end;
				start.x = gui->state.start_x;
				start.y = gui->state.start_y;
				end.x = end_x;
				end.y = end_y;
				sketch_line_init(line, &start, &end);

				gui->state.draw_active = false;
				break;
			default:	
				printf("Unsupported tool!\n");
			}
		} else {
			switch(gui->state.active_tool) {
			case TOOL_NONE:
				printf("TODO: select something...\n");
				break;
			case TOOL_LINE:
				printf("TODO: start a line...\n");
				gui->state.draw_active = true;
				gui->state.start_x = event->x;
				gui->state.start_y = event->y;
				break;
			default:	
				printf("Unsupported tool!\n");
			}
		}

		break;
	case GDK_BUTTON_RELEASE:
		//printf("  button released\n");
		break;
	default:
		//printf("Unexpected event type!\n");
		;
	}
	
	return TRUE;
}

gboolean mouse_motion_cb(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	gui_t *gui = (gui_t *)data;
	//printf("mouse motion callback!\n");

	if(gui->state.draw_active) {
		gui->state.end_x = event->x;
		gui->state.end_y = event->y;
		gtk_widget_queue_draw(gui->canvas);
	}

	return TRUE;
}

gboolean draw_canvas(GtkWidget *widget, GdkEventExpose *event, gpointer data) 
{
	gui_t *gui = (gui_t *)data;
   draw_ptr dp = gui->drawer;
   draw_start(dp);
   int i;

   float width, height;
   draw_get_canvas_dims(dp, &width, &height);

   // first, fill with background color
   draw_set_color(dp, 1,1,1);
   draw_set_line_width(dp, 1);
   draw_rectangle_filled(dp, 0, 0, width, height);

	// draw the active line/arc, if there is one
	if(gui->state.draw_active) {
		switch(gui->state.active_tool) {
		case TOOL_LINE:
			draw_set_color(dp, 0,0,1);
			draw_line(dp, 
			          gui->state.start_x, gui->state.start_y, 
			          gui->state.end_x, gui->state.end_y
			);
			break;
		}
	}

	// draw sketch objects
	for(i = 0; i < app_data.sketch_count; i++) {
		//printf("drawing sketch object %d of %d...\n", i+1, app_data.sketch_count);
		sketch_base_t *obj = app_data.sketch[i];
		switch(obj->type) {
		case SHAPE_TYPE_LINE: {
			sketch_line_t *line = (sketch_line_t *)obj;
			draw_line(dp, 
			          line->v1.x, line->v1.y, 
			          line->v2.x, line->v2.y
			);
			break;
		}
		case SHAPE_TYPE_ARC:
			break;
		}
	}

   draw_finish(dp);
   return TRUE;
}

static void state_init(struct _state *s)
{
	s->draw_active = false;
	s->active_tool = TOOL_NONE;
}

int gui_init(gui_t *self, int *argc, char ***argv)
{
	struct _button_bar *bb = &(self->button_bar);
	struct _status_bar *sb = &(self->status_bar);
	gtk_init(argc, argv);

	state_init( &(self->state) );

   self->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	self->top_level_vbox = gtk_vbox_new(FALSE, 1);
   gtk_container_add(GTK_CONTAINER (self->window), self->top_level_vbox);

	bb->hbox = gtk_hbox_new(FALSE, 1);
	gtk_box_pack_start(GTK_BOX(self->top_level_vbox), bb->hbox, FALSE, FALSE, 0);

	/* Select Button */
	bb->select_btn = gtk_button_new_with_label("Select");
	gtk_box_pack_start(GTK_BOX(bb->hbox), bb->select_btn, FALSE, FALSE, 0);
	g_signal_connect(bb->select_btn, "clicked", G_CALLBACK(select_cb), self);

	/* Line Button */
	bb->line_btn = gtk_button_new_with_label("Line");
	gtk_box_pack_start(GTK_BOX(bb->hbox), bb->line_btn, FALSE, FALSE, 0);
	g_signal_connect(bb->line_btn, "clicked", G_CALLBACK(line_cb), self);

	/* Arc Button */
	bb->arc_btn = gtk_button_new_with_label("Arc");
	gtk_box_pack_start(GTK_BOX(bb->hbox), bb->arc_btn, FALSE, FALSE, 0);
	g_signal_connect(bb->arc_btn, "clicked", G_CALLBACK(arc_cb), self);

	/* Canvas */
   self->canvas = gtk_drawing_area_new();
   gtk_widget_set_size_request(self->canvas, 500,400);
   g_signal_connect(self->canvas, "expose_event", G_CALLBACK(draw_canvas), 
	                 self);
	gtk_box_pack_start(GTK_BOX(self->top_level_vbox), self->canvas, 
	                   TRUE, TRUE, 0);
	gtk_widget_add_events(self->canvas, (
	                                     GDK_BUTTON_PRESS_MASK | 
	                                     GDK_BUTTON_RELEASE_MASK | 
	                                     GDK_POINTER_MOTION_MASK
	                                    )
	                      );
   g_signal_connect(self->canvas, "button_press_event", 
	                 G_CALLBACK(mouse_button_cb), self);
   g_signal_connect(self->canvas, "button_release_event", 
	                 G_CALLBACK(mouse_button_cb), self);
   g_signal_connect(self->canvas, "motion_notify_event", 
	                 G_CALLBACK(mouse_motion_cb), self);

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
