#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gui.h"
#include "main.h"
#include "sketch_types.h"

void gui_update_draw_scale(gui_t *self, 
                        double xmin, double xmax, 
                        double ymin, double ymax,
                        double w, double h)
{
  self->x_m = w / (xmax - xmin);
  self->y_m = h / (ymin - ymax);
  if(fabs(self->x_m) < fabs(self->y_m)) 
  {
    self->y_m = -self->x_m;
    self->x_b = -self->x_m * xmin;
    self->y_b = h/2.0 - self->y_m * (ymin+ymax)/2.0;
  } 
  else 
  {
    self->x_m = -self->y_m;
    self->y_b = -self->y_m * ymax;
    self->x_b = w/2.0 - self->x_m * (xmin+xmax)/2.0;
  }
}

static inline
double px_to_user_x(gui_t *self, double x)
{
  return (x - self->x_b) / self->x_m;
}

static inline
double px_to_user_y(gui_t *self, double y)
{
  return (y - self->y_b) / self->y_m;
}

static inline
double user_to_px_x(gui_t *self, double x)
{
  return self->x_m * x + self->x_b;
}

static inline
double user_to_px_y(gui_t *self, double y)
{
  return self->y_m * y + self->y_b;
}

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

static 
void point_rotate(double x, double y, double theta, double *xp, double *yp)
{
  double cq = cos(theta);
  double sq = sin(theta);
  *xp =  x * cq + y * sq;
  *yp = -x * sq + y * cq;
}

static int sketch_point_is_pt_near(sketch_point_t *s, double x, double y, double tol)
{
  double dx, dy, dist;
  dx = s->x - x;
  dy = s->y - y;
  dist = sqrt(dx*dx + dy*dy);
  if(dist <= tol)
  {
    return 1;
  }
  return 0;
}

static int sketch_line_is_pt_near(sketch_line_t *s, double x, double y, double tol)
{
  coord_2D_t point;
  double theta, len;
  sketch_line_get_point_angle_len(s, &point, &theta, &len);
  double xp1, xp2, yp;
  point_rotate(point.x, point.y, theta, &xp1, &yp);
  xp2 = xp1 + len;
  
  double x_m_p, y_m_p;
  point_rotate(x, y, theta, &x_m_p, &y_m_p);

  if(x_m_p >= xp1 && x_m_p <= xp2 && y_m_p < (yp+tol) && y_m_p > (yp-tol)) 
  {
    return 1;
  }
  return 0;
}

static int highlight_sketch_line(sketch_line_t *s, double x, double y, double tol)
{
  int changed = 0;

  if( sketch_line_is_pt_near(s, x, y, tol)) 
  {
    if(!s->base.is_highlighted) 
    {
      s->base.is_highlighted = 1;
      changed = 1;
    }
    s->v1->base.is_highlighted = 1;
    s->v2->base.is_highlighted = 1;
  } 
  else 
  {
    if(s->base.is_highlighted) 
    {
      s->base.is_highlighted = 0;
      changed = 1;
    }
    s->v1->base.is_highlighted = 0;
    s->v2->base.is_highlighted = 0;
  }
  return changed;
}

static int highlight_sketch_objects(double x, double y, double tol)
{
  int i;
  int something_changed = 0;
  for(i=0; i<app_data.sketch_count; i++) 
  {
    sketch_base_t *s = app_data.sketch[i];
    switch(s->type) 
    {
      case SHAPE_TYPE_LINE:
        if(highlight_sketch_line((sketch_line_t *)s, x, y, tol)) 
        {
          something_changed = 1;
        }
        break;
      case SHAPE_TYPE_ARC:
        printf("Arc not yet supported as selections\n");
        break;
      default:
        printf("Unsupported shape type!\n");
    }
  }
  return something_changed;
}

static 
int line_is_near(sketch_line_t *line, double x_u, double y_u, double tol_u)
{
}

static 
int point_is_near(sketch_point_t *pt, double x_u, double y_u, double tol_u)
{
}

static 
selection_t get_object_at_location(double x_u, double y_u, double tol_u)
{
  int i;
  selection_t sel = {SELECT_TYPE_NONE, NULL};

  for(i=0; i<app_data.sketch_count; i++) 
  {
    sketch_base_t *s = app_data.sketch[i];
    switch(s->type) 
    {
      sketch_base_t *o;
      case SHAPE_TYPE_LINE:
        // first check endpoints
        if(sketch_point_is_pt_near(((sketch_line_t *)s)->v1, x_u, y_u, tol_u))
        {
          sel.type = SELECT_TYPE_POINT;
          sel.object = ((sketch_line_t *)s)->v1;
          return sel;
        }
        if(sketch_point_is_pt_near(((sketch_line_t *)s)->v2, x_u, y_u, tol_u))
        {
          sel.type = SELECT_TYPE_POINT;
          sel.object = ((sketch_line_t *)s)->v2;
          return sel;
        }

        // then the line itself
        if(sketch_line_is_pt_near((sketch_line_t *)s, x_u, y_u, tol_u))
        {
          sel.type = SELECT_TYPE_LINE;
          sel.object = (sketch_line_t *)s;
          return sel;
        }
        break;
      case SHAPE_TYPE_ARC:
        printf("Arc not yet supported as selections\n");
        break;
      default:
        printf("Unsupported shape type!\n");
    }
  }
}

static void start_pan(gui_t *gui, double x, double y)
{
  printf("starting pan\n");
  gui->panning = 1;
  gui->pan_start_x = x;
  gui->pan_start_y = y;
  gui->pan_start_xmin = gui->xmin;
  gui->pan_start_xmax = gui->xmax;
  gui->pan_start_ymin = gui->ymin;
  gui->pan_start_ymax = gui->ymax;
}

static void end_pan(gui_t *gui)
{
  printf("ending pan\n");
  gui->panning = 0;
  gtk_widget_queue_draw(gui->canvas);
}

static void clear_sketch_select(sketch_base_t *s)
{
  // first clear its own flag
  //printf("clearing sketch object's select flag\n");
  s->is_selected = 0;

  // then clear all it's children's flags
  int i;
  for(i=0; i < s->child_count; i++)
  {
    //printf("clearing sketch object's child select flag\n");
    clear_sketch_select(s->children[i]);
  }
}


static void clear_all_selection_flags(void)
{
  int i;
  for(i=0; i<app_data.sketch_count; i++) 
  {
    clear_sketch_select(app_data.sketch[i]);
  }
}


static void update_selection_flags(gui_t *gui)
{
  // first, clear all flags recursively
  clear_all_selection_flags();

  int j;
  for(j=0; j < gui->state.selection_count; j++)
  {
    switch(gui->state.selections[j].type)
    {
      case SELECT_TYPE_LINE:
      case SELECT_TYPE_POINT:
        ((sketch_base_t *)(gui->state.selections[j].object))->is_selected = 1;
        printf("setting selected flag\n");
        break;
      default:
        printf("unhandled selection type\n");
    }
  }

}

static void selection_set(gui_t *gui, selection_t *sel)
{
  printf("setting selection\n");
  gui->state.selections[0] = *sel;
  gui->state.selection_count = 1;
  update_selection_flags(gui);
}

static void selection_add_or_remove(gui_t *gui, selection_t *sel)
{
  assert(sel->type != SELECT_TYPE_NONE);

  // first check if this object is already selected. If it is,
  // remove it and return
  int i;
  for(i=0; i < gui->state.selection_count; i++)
  {
    if(sel->object == gui->state.selections[i].object)
    {
      printf("object already selected - removing it\n");
      int j;
      for(j = i+1; j < gui->state.selection_count; j++)
      {
        gui->state.selections[j-1] = gui->state.selections[j];
      }
      gui->state.selection_count--;
      update_selection_flags(gui);
      return;
    }
  }

  // make sure there's room to add it
  if(gui->state.selection_count >= MAX_SELECTIONS)
  {
    fprintf(stderr, "no room to add selection!\n");
    return;
  }
  printf("adding selection\n");
  gui->state.selections[gui->state.selection_count++] = *sel;
  update_selection_flags(gui);
}

static void selection_clear(gui_t *gui)
{
  printf("clearing selection\n");
  gui->state.selections[0].type = SELECT_TYPE_NONE;
  gui->state.selections[0].object = NULL;
  gui->state.selection_count = 0;
  update_selection_flags(gui);
}

gboolean mouse_button_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  gui_t *gui = (gui_t *)data;
  switch(event->type) 
  {
  case GDK_BUTTON_PRESS:

    // right-click
    if(event->button == 3) 
    {
      start_pan(gui, event->x, event->y);
    }
    // left-click
    else if(event->button == 1) 
    {
      if(gui->state.draw_active) 
      {
        switch(gui->state.active_tool) 
        {
          case TOOL_NONE:
            printf("Shouldn't be here!\n");
            break;
          case TOOL_LINE: 
          {
            double end_xp, end_yp;
            end_xp = event->x;
            end_yp = event->y;
            double end_xu, end_yu;
            end_xu = px_to_user_x(gui, end_xp);
            end_yu = px_to_user_y(gui, end_yp);
            printf("  px::   start: (%g,%g)  end: (%g,%g)\n", 
                gui->state.start_x, gui->state.start_y,
                end_xp, end_yp);
            printf("  user:: start: (%g,%g)  end: (%g,%g)\n",
                gui->state.start_x, gui->state.start_y,
                end_xu, end_yu);

            sketch_line_t *line = sketch_line_alloc();
            app_data.sketch[app_data.sketch_count++] = (sketch_base_t *)line;
            coord_2D_t start, end;
            start.x = gui->state.start_x;
            start.y = gui->state.start_y;
            end.x = end_xu;
            end.y = end_yu;
            sketch_line_init(line, &start, &end);
            gui->state.draw_active = false;
            gtk_widget_queue_draw(gui->canvas);
            break;
          }
          default:  
            printf("Unsupported tool!\n");
        }
      } 
      else 
      {
        switch(gui->state.active_tool) 
        {
          case TOOL_NONE:
          {
            // Select a sketch object
            selection_t sel = get_object_at_location(
                px_to_user_x(gui, event->x), 
                px_to_user_y(gui, event->y), 
                5.0 / fabs(gui->x_m) );

            if(sel.type == SELECT_TYPE_NONE) 
            {
              if(event->state & GDK_SHIFT_MASK)
              {
                printf("no change to selection\n");
              }
              else
              {
                selection_clear(gui);
              }
            }
            else
            {
              if(event->state & GDK_SHIFT_MASK)
              {
                selection_add_or_remove(gui, &sel);
              }
              else
              {
                selection_set(gui, &sel);
              }
            }
            gtk_widget_queue_draw(gui->canvas);
            break;
          }
          case TOOL_LINE:
            // Start of a line
            gui->state.draw_active = true;
            gui->state.start_x = px_to_user_x(gui, event->x);
            gui->state.start_y = px_to_user_y(gui, event->y);
            break;
          default:  
            printf("Unsupported tool!\n");
        }
      }
    }

    break;
  case GDK_BUTTON_RELEASE:
    if(event->button == 3) 
    {
      if(gui->panning) 
        end_pan(gui);
    }
    break;
  default:
    //printf("Unexpected event type!\n");
    ;
  }
  
  return TRUE;
}

gboolean key_release_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  gui_t *gui = (gui_t *)data;

  if(event->keyval == GDK_KEY_space)
  { // space bar
    if(gui->panning)
    {
      end_pan(gui);
    }
  }
  else
  {
    printf("got key release: %c = 0x%02X (%d)\n", event->keyval, event->keyval, 
        event->state);
    if(event->state & GDK_SHIFT_MASK)
      printf(" shift\n");
    if(event->state & GDK_CONTROL_MASK)
      printf(" control\n");
    if(event->state & GDK_META_MASK)
      printf(" meta\n");
  }
  return TRUE;
}

gboolean key_press_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  gui_t *gui = (gui_t *)data;

  if(event->keyval == 'z') 
  { // zoom in
    double alpha = 0.75;
    gint x_s_px, y_s_px;
    gtk_widget_get_pointer(gui->canvas, &x_s_px, &y_s_px);
    double x_s, y_s;
    x_s = px_to_user_x(gui, x_s_px);
    y_s = px_to_user_y(gui, y_s_px);
    double dx = gui->xmax - gui->xmin;
    double dy = gui->ymax - gui->ymin;
    gui->xmin = x_s - alpha/2.0 * dx;
    gui->xmax = x_s + alpha/2.0 * dx;
    gui->ymin = y_s - alpha/2.0 * dy;
    gui->ymax = y_s + alpha/2.0 * dy;
    gtk_widget_queue_draw(gui->canvas);

  } 
  else if(event->keyval == 'Z') 
  { // zoom out
    double alpha = 4.0/3.0;
    gint x_s_px, y_s_px;
    gtk_widget_get_pointer(gui->canvas, &x_s_px, &y_s_px);
    double x_s, y_s;
    x_s = px_to_user_x(gui, x_s_px);
    y_s = px_to_user_y(gui, y_s_px);
    double dx = gui->xmax - gui->xmin;
    double dy = gui->ymax - gui->ymin;
    gui->xmin = x_s - alpha/2.0 * dx;
    gui->xmax = x_s + alpha/2.0 * dx;
    gui->ymin = y_s - alpha/2.0 * dy;
    gui->ymax = y_s + alpha/2.0 * dy;
    gtk_widget_queue_draw(gui->canvas);

  }
  else if(event->keyval == GDK_KEY_Escape)
  { // escape should cancel any active operation
    if(gui->state.draw_active)
    {
      gui->state.draw_active = 0;
      gtk_widget_queue_draw(gui->canvas);
    }
  }
  else if(event->keyval == GDK_KEY_space)
  { // space bar
    if(!gui->panning)
    {
      gint x_px, y_px;
      gtk_widget_get_pointer(gui->canvas, &x_px, &y_px);
      start_pan(gui, x_px, y_px);
    }
  }
  else
  {
    printf("got key press: %c = 0x%02X (%d)\n", event->keyval, event->keyval, 
        event->state);
    if(event->state & GDK_SHIFT_MASK)
      printf(" shift\n");
    if(event->state & GDK_CONTROL_MASK)
      printf(" control\n");
    if(event->state & GDK_META_MASK)
      printf(" meta\n");
  }

  return TRUE;
}

gboolean mouse_motion_cb(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
  gui_t *gui = (gui_t *)data;
  //printf("mouse motion callback!\n");

  if(gui->state.draw_active) 
  {
    gui->state.end_x = event->x;
    gui->state.end_y = event->y;
    gtk_widget_queue_draw(gui->canvas);
  } 
  else if(gui->state.active_tool == TOOL_NONE) 
  {
    if( highlight_sketch_objects(px_to_user_x(gui, event->x), 
          px_to_user_y(gui, event->y), 5.0/fabs(gui->x_m) ) ) 
    {
      //printf("redrawing due to highlight state change\n");
      gtk_widget_queue_draw(gui->canvas);
    }
  }

  if(gui->panning) 
  {
    gui->xmin = gui->pan_start_xmin - (event->x - gui->pan_start_x) / gui->x_m;
    gui->xmax = gui->pan_start_xmax - (event->x - gui->pan_start_x) / gui->x_m;
    gui->ymin = gui->pan_start_ymin - (event->y - gui->pan_start_y) / gui->y_m;
    gui->ymax = gui->pan_start_ymax - (event->y - gui->pan_start_y) / gui->y_m;
    gtk_widget_queue_draw(gui->canvas);
  }

  return TRUE;
}

/*************************************************/

static double round_up_to_nearest(double num, double nearest) 
{
  double a = num / nearest;
  return (ceil(a) * nearest);
}

static double round_down_to_nearest(double num, double nearest) 
{
  double a = num / nearest;
  return (floor(a) * nearest);
}

static void get_double_parts(double f, double *mantissa, int *exponent) 
{
  int neg = 0;
  if(f == 0.0) 
  {  
    *mantissa = 0.0;
    *exponent = 0;
    return;
  }
  if(f < 0) 
    neg = 1;
  *exponent = floor(log10(fabs(f)));
  *mantissa = f / pow(10, *exponent);
  return;
}

static int set_linear_tic_values(axis_t *a, double min, double max) 
{
  double raw_range = max - min;
  double raw_tic_delta = raw_range / (NUM_REQ_TICS - 1);
  double mantissa;
  int exponent;
  get_double_parts(raw_tic_delta, &mantissa, &exponent);
  double actual_tic_delta;

  if(mantissa <= 1.0) 
  {
    actual_tic_delta = 1.0 * pow(10., exponent);
  }
  else if(mantissa <= 2.0) 
  {
    actual_tic_delta = 2.0 * pow(10., exponent);
  }
  else if(mantissa <= 2.5) 
  {
    actual_tic_delta = 2.5 * pow(10., exponent);
  }
  else if(mantissa <= 5.0) 
  {
    actual_tic_delta = 5.0 * pow(10., exponent);
  }
  else {
    actual_tic_delta = 1.0 * pow(10., exponent+1);
  }

  double min_tic_val;
  min_tic_val = round_up_to_nearest(min, actual_tic_delta);
    
  double tic_val;
  int i = 0;
  for(tic_val = min_tic_val; 
      tic_val < max && i < MAX_NUM_MAJOR_TICS; 
      tic_val += actual_tic_delta) 
  {
    /* perform check to see if it should be equal to zero */
    if(fabs(tic_val / actual_tic_delta) < 0.5) 
    {
      tic_val = 0.0;
    }
    a->major_tic_values[i] = tic_val;
    i++;
  }
  if(i >= MAX_NUM_MAJOR_TICS) 
  {
    printf("Too many major tics!!!\n");
    return -1;
  }
  
  a->num_actual_major_tics = i;   
  a->major_tic_delta = actual_tic_delta;

  return 0;
}


static int set_major_tic_labels(axis_t *a) 
{
  int i, ret;
  int err = 0;
  double dd = log10( fabs(a->major_tic_delta) );
  double d1 = log10(fabs(a->major_tic_values[0]));
  double d2 = log10(fabs(a->major_tic_values[a->num_actual_major_tics-1]));
  double d = (d2>d1) ? d2 : d1;
  int sigs = ceil(d) - floor(dd) + 1.5;
  if(sigs < 4) 
    sigs = 4;
  sprintf(a->tic_label_format_string, "%%.%dg", sigs);
  for(i=0; i<a->num_actual_major_tics; i++) 
  { 
    ret = snprintf(a->major_tic_labels[i], MAJOR_TIC_LABEL_SIZE, 
        a->tic_label_format_string, a->major_tic_values[i]);
  }
  return err;
}


/*************************************************/


static void draw_ruler(gui_t *self)
{
  int i;
  draw_ptr dp = self->drawer;

  axis_t *ax = &(self->x_axis);
  axis_t *ay = &(self->y_axis);

  set_linear_tic_values(ax, self->xmin, self->xmax);
  set_linear_tic_values(ay, self->ymin, self->ymax);
  set_major_tic_labels(ax);
  set_major_tic_labels(ay);

  float width, height;
   draw_get_canvas_dims(dp, &width, &height);
  
  // draw some gridlines for now...
  draw_set_line_width(dp, 1);
  draw_set_color(dp, 0.9, 0.9, 0.9);
  for(i=0; i < ax->num_actual_major_tics; i++) 
  {
    double val = ax->major_tic_values[i];
    double x_px = user_to_px_x(self, val);
    draw_line(dp, x_px, 0, x_px, height);
  }
  for(i=0; i < ay->num_actual_major_tics; i++) 
  {
    double val = ay->major_tic_values[i];
    double y_px = user_to_px_y(self, val);
    draw_line(dp, 0, y_px, width, y_px);
  }

  // draw the top (horizontal) ruler bar
  draw_set_color(dp, 0, 0, 0);
  for(i=0; i < ax->num_actual_major_tics; i++) 
  {
    double val = ax->major_tic_values[i];
    double x_px = user_to_px_x(self, val);
    draw_text(dp, ax->major_tic_labels[i], 8, x_px, 5, ANCHOR_TOP_MIDDLE);
    draw_line(dp, x_px, 15, x_px, 25);
  }

  // draw the left (vertical) ruler bar
  draw_set_color(dp, 0, 0, 0);
  for(i=0; i < ay->num_actual_major_tics; i++) 
  {
    double val = ay->major_tic_values[i];
    double y_px = user_to_px_y(self, val);
    char *s = ay->major_tic_labels[i];
    double w = draw_get_text_width(dp, s, 8);
    draw_text(dp, s, 8, 5, y_px, ANCHOR_MIDDLE_LEFT);
    draw_line(dp, 5+w+5, y_px, 30, y_px);
  }

}


static void draw_sketch_point(sketch_base_t *obj, gui_t *gui)
{
  draw_ptr dp = gui->drawer;
  sketch_point_t *pt = (sketch_point_t *)obj;
  
  draw_set_color(dp, 0,0,1);
  double radius = 2.0;

  if(obj->is_selected) 
  {
    draw_set_color(dp, 1,0,0);
    radius = 4.0;
  }
  else if(obj->is_highlighted) 
  {
    draw_set_color(dp, 0,1,0);
    radius = 3.0;
  }

  draw_circle_filled(dp, user_to_px_x(gui, pt->x), user_to_px_y(gui, pt->y), radius);
}

static void draw_sketch_line(sketch_base_t *obj, gui_t *gui)
{
  draw_ptr dp = gui->drawer;

  if(obj->is_selected) 
  {
    draw_set_color(dp, 1,0,0);
  }
  if(obj->is_highlighted) 
  {
    draw_set_line_width(dp, 3);
  }
  sketch_line_t *line = (sketch_line_t *)obj;
  draw_line(dp, 
      user_to_px_x(gui, line->v1->x), user_to_px_y(gui, line->v1->y),
      user_to_px_x(gui, line->v2->x), user_to_px_y(gui, line->v2->y) );

  draw_sketch_point( (sketch_base_t *)line->v1, gui);
  draw_sketch_point( (sketch_base_t *)line->v2, gui);
}

gboolean draw_canvas(GtkWidget *widget, GdkEventExpose *event, gpointer data) 
{
  gui_t *gui = (gui_t *)data;
  draw_ptr dp = gui->drawer;
  draw_start(dp);
  int i;

  // grab keyboard focus
  gtk_widget_grab_focus(gui->canvas);

  float width, height;
  draw_get_canvas_dims(dp, &width, &height);


  /* Set the scaling conversion coefficients.
   * This really should only need to be done when:
   *   - the view is changed (pan or zoom)
   *   - the window is resized (configure event)
   */
  gui_update_draw_scale(gui, gui->xmin, gui->xmax, gui->ymin, gui->ymax, 
      width, height);

   // first, fill with background color
   draw_set_color(dp, 1,1,1);
   draw_rectangle_filled(dp, 0, 0, width, height);

  // draw the ruler
  draw_ruler(gui);

  // draw the active line/arc, if there is one
  if(gui->state.draw_active) 
  {
    draw_set_line_width(dp, 1);
    draw_set_color(dp, 0,0,1);
    switch(gui->state.active_tool) 
    {
      case TOOL_LINE:
        draw_line(dp, 
            user_to_px_x(gui, gui->state.start_x), user_to_px_y(gui, gui->state.start_y), 
            gui->state.end_x, gui->state.end_y);
        break;
    }
  }

  // draw sketch objects
  for(i = 0; i < app_data.sketch_count; i++) 
  {
    draw_set_line_width(dp, 2);
    draw_set_color(dp, 0,0,1);
    //printf("drawing sketch object %d of %d...\n", i+1, app_data.sketch_count);
    sketch_base_t *obj = app_data.sketch[i];
    switch(obj->type) 
    {
      case SHAPE_TYPE_LINE: 
      {
        draw_sketch_line(obj, gui);
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
  s->selection_count = 0;
}

static GtkWidget *toolbar_button_new(const char *image_path, int size, const char *label)
{
  GtkToolItem *b = gtk_toggle_tool_button_new();

  if(image_path != NULL)
  {
    GdkPixbuf *pb = gdk_pixbuf_new_from_file(image_path, NULL);
    assert(pb);

    gint pbw = gdk_pixbuf_get_width(pb);
    gint pbh = gdk_pixbuf_get_height(pb);
    if(pbw > pbh)
    {
      pbh = (1.0 * size / pbw) * pbh;
      pbw = size;
    }
    else
    {
      pbw = (1.0 * size / pbh) * pbw;
      pbh = size;
    }
    GdkPixbuf *pb2 = gdk_pixbuf_scale_simple(pb, pbw, pbh, GDK_INTERP_HYPER);
    g_object_unref(pb);

    GtkWidget *button_image = gtk_image_new_from_pixbuf(pb2);
    assert(button_image);
    gtk_tool_button_set_icon_widget((GtkToolButton *)b, button_image);
  }
  else
  {
    gtk_tool_item_set_is_important((GtkToolItem *)b, TRUE);
    gtk_tool_button_set_label_widget((GtkToolButton *)b, NULL);
    gtk_tool_button_set_label((GtkToolButton *)b, label);
  }
  return (GtkWidget *)b;
}

void make_tools_toolbar(gui_t *self)
{
  struct _tools_tb *p = &(self->tools_tb);
  p->tb = gtk_toolbar_new();

  p->select_btn = toolbar_button_new(NULL, 30, "Select");
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->select_btn, -1);

  p->line_btn = toolbar_button_new("button_icon.svg", 30, NULL);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->line_btn, -1);

  p->arc_btn = toolbar_button_new(NULL, 30, "Arc");
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->arc_btn, -1);

  gtk_box_pack_start(GTK_BOX(self->top_level_vbox), p->tb, FALSE, FALSE, 0);
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

  make_tools_toolbar(self);

  bb->hbox = gtk_hbox_new(FALSE, 1);
  gtk_box_pack_start(GTK_BOX(self->top_level_vbox), bb->hbox, FALSE, FALSE, 0);

  /* Select Button */
  //bb->select_btn = gtk_button_new_with_label("Select");
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
  gtk_widget_add_events(self->canvas, 
      (GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK ) );

  gtk_widget_set_can_focus(self->canvas, TRUE);

  g_signal_connect(self->canvas, "button_press_event", 
      G_CALLBACK(mouse_button_cb), self);

  g_signal_connect(self->canvas, "button_release_event", 
      G_CALLBACK(mouse_button_cb), self);

  g_signal_connect(self->canvas, "motion_notify_event", 
      G_CALLBACK(mouse_motion_cb), self);

  g_signal_connect(self->canvas, "key_press_event", 
      G_CALLBACK(key_press_cb), self);

  g_signal_connect(self->canvas, "key_release_event", 
      G_CALLBACK(key_release_cb), self);

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

  // for now, just set user coords equal to pixel coords
  self->xmin = -10;
  self->xmax = +10;
  self->ymin = -10;
  self->ymax = +10;

  self->panning = 0;

  return 0;
}

void gui_run(gui_t *self)
{
  gtk_main();
}
