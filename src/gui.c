#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "gui.h"
#include "main.h"
#include "solver.h"
#include "sketch_types.h"
#include "param_dialog.h"

#define SNAP_DIST_PX (8.0)

// private function prototypes
static void add_constraint(constraint_t *c);
static void update_constraints(void);

static int delete_constraint(constraint_t *c)
{
  int i;
  for(i = 0; i < app_data.constraint_count; i++)
  {
    if(c == app_data.constraints[i])
    {
      int j;
      for(j = i+1; j < app_data.constraint_count; j++)
      {
        app_data.constraints[j-1] = app_data.constraints[j];
      }
      app_data.constraint_count--;
    }
  }
  return 0;
}

static int delete_sketch_object(gui_t *gui, sketch_base_t *object)
{
  int i;
  int found = 0;

  // first, delete all constraints related to this object
  for(i=0; i < app_data.constraint_count; i++)
  {
    constraint_t *c = app_data.constraints[i];
    switch(object->type)
    {
      case SHAPE_TYPE_LINE:
        if((sketch_base_t *)c->line1 == object || 
            (sketch_base_t *)c->line2 == object)
        {
          delete_constraint(c);
        }
        break;
      default:
        printf("don't know how to delete constraints for this type of object!\n");
    }
  }

  // then delete the sketch object itself
  for(i=0; i < app_data.sketch_count; i++)
  {
    if(app_data.sketch[i] == object)
    {
      int j;

      switch(object->type)
      {
        case SHAPE_TYPE_LINE:
          sketch_line_fini((sketch_line_t *)object);
          sketch_line_free((sketch_line_t *)object);
          break;
        default:
          printf("don't know how to delete this type of object!\n");
      }

      for(j=i+1; j < app_data.sketch_count; j++)
        app_data.sketch[j-1] = app_data.sketch[j];
      app_data.sketch_count--;
      found = 1;
      break;
    }
  }

  // cleanup (ugly)
  if((void *)gui->state.highlight_sel.object == (void *)object)
  {
    gui->state.highlight_sel.type = SELECT_TYPE_NONE;
    gtk_widget_queue_draw(gui->canvas);
  }

  return found ? 0 : -1;
}


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
double px_to_user_dx(gui_t *self, double dx)
{
  return dx / self->x_m;
}

static inline
double px_to_user_y(gui_t *self, double y)
{
  return (y - self->y_b) / self->y_m;
}

static inline
double px_to_user_dy(gui_t *self, double dy)
{
  return dy / self->y_m;
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

static void set_active_tool(gui_t *gui, tool_t tool)
{
  switch(tool)
  {
    case TOOL_NONE:
      gui->state.active_tool = tool;
      gtk_label_set_text((GtkLabel *)(gui->status_bar.left_label), "tool: select");
      break;
    case TOOL_LINE:
      gui->state.active_tool = tool;
      gtk_label_set_text((GtkLabel *)(gui->status_bar.left_label), "tool: line");
      break;
    case TOOL_POLYLINE:
      gui->state.active_tool = tool;
      gtk_label_set_text((GtkLabel *)(gui->status_bar.left_label), "tool: ployline");
      break;
    case TOOL_ARC:
      gui->state.active_tool = tool;
      gtk_label_set_text((GtkLabel *)(gui->status_bar.left_label), "tool: arc");
      break;
    default:
      printf("invalid tool to activate impossible?\n");
  }
}

static void cancel_active_draw(gui_t *gui)
{
  if(!gui->state.draw_active)
    return;

  gui->state.draw_active = 0;
  switch(gui->state.active_tool)
  {
    case TOOL_POLYLINE:
    case TOOL_LINE:
      {
        sketch_line_t *line = 
          (sketch_line_t *)app_data.sketch[app_data.sketch_count - 1];
        delete_sketch_object(gui, (sketch_base_t *)line);
      }
      break;
    default:
      printf("don't know how to cancel active draw for this tool\n");
  }
}

static void select_cb(GtkButton *b, gpointer data)
{
  gui_t *gui = (gui_t *)data;
  //printf("select button clicked!\n");
  cancel_active_draw(gui);
  set_active_tool(gui, TOOL_NONE);
}

static void line_cb(GtkButton *b, gpointer data)
{
  gui_t *gui = (gui_t *)data;
  //printf("line button clicked!\n");
  cancel_active_draw(gui);
  set_active_tool(gui, TOOL_LINE);
}

static void polyline_cb(GtkButton *b, gpointer data)
{
  gui_t *gui = (gui_t *)data;
  //printf("polyline button clicked!\n");
  cancel_active_draw(gui);
  set_active_tool(gui, TOOL_POLYLINE);
}

static void arc_cb(GtkButton *b, gpointer data)
{
  gui_t *gui = (gui_t *)data;
  //printf("arc button clicked!\n");
  cancel_active_draw(gui);
  set_active_tool(gui, TOOL_ARC);
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

static 
int line_is_near(sketch_line_t *line, double x_u, double y_u, double tol_u)
{
}

static 
int point_is_near(sketch_point_t *pt, double x_u, double y_u, double tol_u)
{
}

static 
selection_t get_object_at_location(gui_t *gui, double x_u, double y_u, double tol_u)
{
  int i;
  selection_t sel = {SELECT_TYPE_NONE, NULL};

  int max_iter = app_data.sketch_count;
  if(gui->state.draw_active)
    max_iter--;

  for(i=0; i < max_iter; i++) 
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
  return sel;
}

static void start_drag(gui_t *gui, double x, double y)
{
  printf("starting drag\n");
  gui->dragging = 1;
  gui->drag_start_x = x;
  gui->drag_start_y = y;
  gui->state.end_x = x;
  gui->state.end_y = y;
}

static void end_drag(gui_t *gui)
{
  printf("ending drag\n");
  gui->dragging = 0;
  app_data.constraints_dirty = 1;
  update_constraints();
  gtk_widget_queue_draw(gui->canvas);
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
          case TOOL_POLYLINE: 
          case TOOL_LINE: 
          {
            double end_xp, end_yp;
            end_xp = event->x;
            end_yp = event->y;
            double end_xu, end_yu;
            end_xu = px_to_user_x(gui, end_xp);
            end_yu = px_to_user_y(gui, end_yp);

            char horiz = 0;
            if(event->state & GDK_SHIFT_MASK)
            {
              double dx = end_xu - gui->state.start_x;
              double dy = end_yu - gui->state.start_y;
              if(fabs(dx) > fabs(dy))
              {
                horiz = 1;
                end_yu = gui->state.start_y;
              }
              else
              {
                end_xu = gui->state.start_x;
              }
            }

            selection_t sel = 
              get_object_at_location(gui, end_xu, end_yu, SNAP_DIST_PX / fabs(gui->x_m) );
            if(sel.type == SELECT_TYPE_POINT) 
            {
              end_xu = ((sketch_point_t *)(sel.object))->x;
              end_yu = ((sketch_point_t *)(sel.object))->y;
            }
            printf("  px::   start: (%g,%g)  end: (%g,%g)\n", 
                gui->state.start_x, gui->state.start_y,
                end_xp, end_yp);
            printf("  user:: start: (%g,%g)  end: (%g,%g)\n",
                gui->state.start_x, gui->state.start_y,
                end_xu, end_yu);

            sketch_line_t *line = (sketch_line_t *)app_data.sketch[app_data.sketch_count - 1];
            line->v2->x = end_xu;
            line->v2->y = end_yu;

            if(event->state & GDK_SHIFT_MASK)
            {
              constraint_t *c = constraint_alloc();
              assert(c != NULL);
              if(horiz)
                constraint_init_line_horiz(c, line);
              else
                constraint_init_line_vert(c, line);
              add_constraint(c);
              update_constraints();
            }

            if(sel.type == SELECT_TYPE_POINT) 
            {
              constraint_t *c = constraint_alloc();
              assert(c != NULL);
              constraint_init_p_p_coinc(c, line->v2, (sketch_point_t *)(sel.object) );
              add_constraint(c);
              update_constraints();
            }

            if(gui->state.active_tool == TOOL_POLYLINE)
            {
              sketch_line_t *next_line = sketch_line_alloc();
              app_data.sketch[app_data.sketch_count++] = 
                (sketch_base_t *)next_line;

              gui->state.start_x = end_xu;
              gui->state.start_y = end_yu;

              coord_2D_t start, end;
              start.x = end_xu;
              start.y = end_yu;
              end.x = start.x;
              end.y = start.y;
              sketch_line_init(next_line, &start, &end);

              constraint_t *c = constraint_alloc();
              assert(c != NULL);
              constraint_init_p_p_coinc(c, line->v2, next_line->v1 );
              add_constraint(c);
              update_constraints();
            }
            else
            {
              gui->state.draw_active = false;
            }

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
                gui,
                px_to_user_x(gui, event->x), 
                px_to_user_y(gui, event->y), 
                SNAP_DIST_PX  / fabs(gui->x_m) );

            if(sel.type == SELECT_TYPE_NONE) 
            {
              if(event->state & GDK_SHIFT_MASK)
                printf("no change to selection\n");
              else
                selection_clear(gui);
            }
            else
            {
              if(event->state & GDK_SHIFT_MASK)
                selection_add_or_remove(gui, &sel);
              else
                selection_set(gui, &sel);
            }
            if(gui->state.selection_count > 0)
            {
              start_drag(gui, event->x, event->y);
            }
            gtk_widget_queue_draw(gui->canvas);
            break;
          }
          case TOOL_POLYLINE: 
          case TOOL_LINE:
            // Start of a line
            gui->state.start_x = px_to_user_x(gui, event->x);
            gui->state.start_y = px_to_user_y(gui, event->y);

            selection_t sel = 
              get_object_at_location(gui, gui->state.start_x, gui->state.start_y, 
              SNAP_DIST_PX  / fabs(gui->x_m) );
            if(sel.type == SELECT_TYPE_POINT) 
            {
              gui->state.start_x = ((sketch_point_t *)(sel.object))->x;
              gui->state.start_y = ((sketch_point_t *)(sel.object))->y;
            }

            sketch_line_t *line = sketch_line_alloc();
            app_data.sketch[app_data.sketch_count++] = (sketch_base_t *)line;

            coord_2D_t start, end;
            start.x = gui->state.start_x;
            start.y = gui->state.start_y;
            end.x = start.x;
            end.y = start.y;
            sketch_line_init(line, &start, &end);

            if(sel.type == SELECT_TYPE_POINT) 
            {
              constraint_t *c = constraint_alloc();
              assert(c != NULL);
              constraint_init_p_p_coinc(c, line->v1, (sketch_point_t *)(sel.object) );
              add_constraint(c);
              update_constraints();
            }

            gui->state.draw_active = true;
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
    else if(event->button == 1) 
    {
      if(gui->dragging) 
      {
        double dx_px = event->x - gui->drag_start_x;
        double dy_px = event->y - gui->drag_start_y;
        printf("dx_px=%g, dy_px=%g\n", dx_px, dy_px);
        double dx_user = px_to_user_dx(gui, dx_px);
        double dy_user = px_to_user_dy(gui, dy_px);
        printf("dx_user=%g, dy_user=%g\n", dx_user, dy_user);
        int i;
        for(i=0; i < gui->state.selection_count; i++)
        {
          if(gui->state.selections[i].type == SELECT_TYPE_POINT)
          {
            sketch_point_t *p = (sketch_point_t *)(gui->state.selections[i].object);
            p->x += dx_user;
            p->y += dy_user;
          }
          else if(gui->state.selections[i].type == SELECT_TYPE_LINE)
          {
            sketch_line_t *l = (sketch_line_t *)(gui->state.selections[i].object);
            l->v1->x += dx_user;
            l->v1->y += dy_user;
            l->v2->x += dx_user;
            l->v2->y += dy_user;
          }
        }
        end_drag(gui);
      }
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
    /*
    printf("got key release: %c = 0x%02X (%d)\n", event->keyval, event->keyval, 
        event->state);
    if(event->state & GDK_SHIFT_MASK)
      printf(" shift\n");
    if(event->state & GDK_CONTROL_MASK)
      printf(" control\n");
    if(event->state & GDK_META_MASK)
      printf(" meta\n");
    */
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
      cancel_active_draw(gui);
      gtk_widget_queue_draw(gui->canvas);
    }
    else if(gui->panning)
    {
      end_pan(gui);
    }
    else if(gui->dragging)
    {
      end_drag(gui);
    }
    else if(gui->state.active_tool != TOOL_NONE)
    {
      printf("switching tool to select (due to Escape key)\n");
      set_active_tool(gui, TOOL_NONE);
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
  else if(event->keyval == 'c')
  {
    printf("toggling draw_constraints\n");
    gui->draw_constraints = !gui->draw_constraints;
    gtk_widget_queue_draw(gui->canvas);
  }
  else if(event->keyval == 'u')
  {
    printf("updating constraints\n");
    update_constraints();
    gtk_widget_queue_draw(gui->canvas);
  }
  else if(event->keyval == 'U')
  {
    printf("updating constraints (with re-init)\n");
    app_data.constraints_dirty = 1;
    update_constraints();
    gtk_widget_queue_draw(gui->canvas);
  }
  else if(event->keyval == GDK_KEY_Delete)
  {
    //printf("got delete key press\n");
    if(gui->state.active_tool == TOOL_NONE)
    {
      int i;
      for(i=0; i < gui->state.selection_count; i++)
      {
        if(gui->state.selections[i].type == SELECT_TYPE_POINT ||
           gui->state.selections[i].type == SELECT_TYPE_LINE)
        {
          printf("deleting selected sketch object...\n");
          if(delete_sketch_object(gui, (sketch_base_t *)(gui->state.selections[i].object)))
          {
            printf("error deleting sketch object\n");
          }
          else 
          {
            gtk_widget_queue_draw(gui->canvas);
          }
        }
      }
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

  double xu, yu;
  xu = px_to_user_x(gui, event->x);
  yu = px_to_user_y(gui, event->y);

  selection_t sel = 
    get_object_at_location(gui, xu, yu, SNAP_DIST_PX / fabs(gui->x_m) );

  gui->state.highlight_sel = sel;
  if(gui->state.highlight_sel.type != gui->state.last_highlight_sel.type ||
      gui->state.highlight_sel.object != gui->state.last_highlight_sel.object)
  {
    gtk_widget_queue_draw(gui->canvas);
  }

  if(gui->state.draw_active) 
  {
    gui->state.end_x = event->x;
    gui->state.end_y = event->y;
    if(event->state & GDK_SHIFT_MASK)
    {
      double dx = xu - gui->state.start_x;
      double dy = yu - gui->state.start_y;
      if(fabs(dx) > fabs(dy))
        gui->state.end_y = user_to_px_y(gui, gui->state.start_y);
      else
        gui->state.end_x = user_to_px_x(gui, gui->state.start_x);
    }

    switch(gui->state.active_tool)
    {
      case TOOL_POLYLINE:
      case TOOL_LINE:
        {
          sketch_line_t *line = (sketch_line_t *)(app_data.sketch[app_data.sketch_count-1]);
          line->v2->x = px_to_user_x(gui, gui->state.end_x);
          line->v2->y = px_to_user_y(gui, gui->state.end_y);
        }
        break;
      default:
        printf("don't know what to do!\n");
    }

    gtk_widget_queue_draw(gui->canvas);
  } 
  else if(gui->state.active_tool == TOOL_NONE) 
  {
    if(event->state & GDK_BUTTON1_MASK)
    {
      if(gui->dragging)
      {
        gui->state.end_x = event->x;
        gui->state.end_y = event->y;
        gtk_widget_queue_draw(gui->canvas);
      }
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

  gui->state.last_highlight_sel = gui->state.highlight_sel;
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

  // draw the horizontal ruler bar(s)
  draw_set_color(dp, 0, 0, 0);
  for(i=0; i < ax->num_actual_major_tics; i++) 
  {
    double val = ax->major_tic_values[i];
    double x_px = user_to_px_x(self, val);

    // top
    draw_text(dp, ax->major_tic_labels[i], 8, x_px, 5, ANCHOR_TOP_MIDDLE);
    draw_line(dp, x_px, 15, x_px, 25);

    // bottom
    draw_text(dp, ax->major_tic_labels[i], 8, x_px, height-5, ANCHOR_BOTTOM_MIDDLE);
    draw_line(dp, x_px, height - 15, x_px, height - 25);
  }

  // draw the vertical ruler bar(s)
  draw_set_color(dp, 0, 0, 0);
  for(i=0; i < ay->num_actual_major_tics; i++) 
  {
    double val = ay->major_tic_values[i];
    double y_px = user_to_px_y(self, val);
    char *s = ay->major_tic_labels[i];
    double w = draw_get_text_width(dp, s, 8);

    // left
    draw_text(dp, s, 8, 5, y_px, ANCHOR_MIDDLE_LEFT);
    draw_line(dp, 5+w+5, y_px, 30, y_px);

    // right
    draw_text(dp, s, 8, width - 5, y_px, ANCHOR_MIDDLE_RIGHT);
    draw_line(dp, width - (5+w+5), y_px, width - 30, y_px);
  }

}


static void draw_sketch_point(sketch_base_t *obj, gui_t *gui)
{
  draw_ptr dp = gui->drawer;
  sketch_point_t *pt = (sketch_point_t *)obj;
  
  draw_set_color(dp, 0,0,1);
  double radius = 1.0;

  if(obj->is_selected) 
  {
    draw_set_color(dp, 1,0,0);
    radius = 3.0;
  }
  else if(obj->is_highlighted) 
  {
    draw_set_color(dp, 0,1,0);
    //radius = radius * 2;
    radius = 3.0;
  }

  double x_offset_px = 0;
  double y_offset_px = 0;
  if(gui->dragging && obj->is_selected)
  {
    x_offset_px = gui->state.end_x - gui->drag_start_x;
    y_offset_px = gui->state.end_y - gui->drag_start_y;
  }

  draw_circle_filled(dp, 
      user_to_px_x(gui, pt->x) + x_offset_px, 
      user_to_px_y(gui, pt->y) + y_offset_px, 
      radius);
}

static void draw_sketch_line(sketch_base_t *obj, gui_t *gui)
{
  draw_ptr dp = gui->drawer;

  draw_set_line_width(dp, obj->line_width);

  if(obj->is_selected) 
  {
    draw_set_color(dp, 1,0,0);
  }
  if(obj->is_highlighted) 
  {
    draw_set_line_width(dp, obj->line_width * 1.5);
  }
  sketch_line_t *line = (sketch_line_t *)obj;

  double x_offset_px = 0;
  double y_offset_px = 0;
  if(gui->dragging && obj->is_selected)
  {
    x_offset_px = gui->state.end_x - gui->drag_start_x;
    y_offset_px = gui->state.end_y - gui->drag_start_y;
  }

  draw_line(dp, 
      user_to_px_x(gui, line->v1->x) + x_offset_px, 
      user_to_px_y(gui, line->v1->y) + y_offset_px,
      user_to_px_x(gui, line->v2->x) + x_offset_px, 
      user_to_px_y(gui, line->v2->y) + y_offset_px);

  draw_sketch_point( (sketch_base_t *)line->v1, gui);
  draw_sketch_point( (sketch_base_t *)line->v2, gui);
}

#define CSIZE (10.0)


static void get_line_constraint_location(gui_t *gui, sketch_line_t *line,
    constraint_type_t type, double *xp, double *yp)
{
  double xm, ym;
  xm = 0.5 * (line->v1->x + line->v2->x);
  ym = 0.5 * (line->v1->y + line->v2->y);

  double x_px = user_to_px_x(gui, xm);
  double y_px = user_to_px_y(gui, ym);
  switch(type)
  {
    case CT_LINE_LINE_EQUAL:
      x_px += 0.0;
      y_px += 0.0;
      break;
    case CT_LINE_HORIZ:
      x_px += +CSIZE;
      y_px += 0.0;
      break;
    case CT_LINE_VERT:
      x_px += 0.0;
      y_px += +CSIZE;
      break;
    case CT_LINE_LENGTH:
      x_px += -CSIZE;
      y_px += 0.0;
      break;
    case CT_LINE_LINE_PARALLEL:
      x_px += +CSIZE;
      y_px += +CSIZE;
      break;
    case CT_LINE_LINE_ORTHOG:
      x_px += -CSIZE;
      y_px += +CSIZE;
      break;
    default:
      ;
  }

  *xp = x_px;
  *yp = y_px;
  return;
}

void draw_line_line_connector(gui_t *gui, sketch_line_t *l1, sketch_line_t *l2,
    constraint_type_t type)
{
  draw_ptr dp = gui->drawer;
  double x1, y1, x2, y2;

  get_line_constraint_location(gui, l1, type, &x1, &y1);
  get_line_constraint_location(gui, l2, type, &x2, &y2);

  draw_set_color(dp, 0.7, 0.7, 0.7);
  draw_set_line_width(dp, 1);
  draw_line(dp, x1, y1, x2, y2);
}

void draw_line_constraint(gui_t *gui, sketch_line_t *line, 
    constraint_type_t type)
{
  draw_ptr dp = gui->drawer;
  draw_set_color(dp, 0.7, 0.7, 0.7);

  double x_px, y_px;
  get_line_constraint_location(gui, line, type, &x_px, &y_px);

  // draw the square background
  draw_rectangle_filled(dp, 
      x_px - 0.5*CSIZE, y_px - 0.5*CSIZE, 
      x_px + 0.5*CSIZE, y_px + 0.5*CSIZE );

  draw_set_line_width(dp, 1);
  draw_set_color(dp, 0, 0, 0);

  switch(type)
  {
    case CT_LINE_LINE_EQUAL:
      draw_line(dp, 
          x_px - 0.4*CSIZE, y_px - 0.2*CSIZE, 
          x_px + 0.4*CSIZE, y_px - 0.2*CSIZE);
      draw_line(dp, 
          x_px - 0.4*CSIZE, y_px + 0.2*CSIZE, 
          x_px + 0.4*CSIZE, y_px + 0.2*CSIZE);
      break;
    case CT_LINE_HORIZ:
      draw_line(dp, x_px - 0.4*CSIZE, y_px, x_px + 0.4*CSIZE, y_px);
      break;
    case CT_LINE_VERT:
      draw_line(dp, x_px, y_px - 0.4*CSIZE, x_px, y_px + 0.4*CSIZE);
      break;
    case CT_LINE_LINE_PARALLEL:
      draw_line(dp, 
          x_px - 0.4*CSIZE, y_px + 0.4*CSIZE, 
          x_px + 0.1*CSIZE, y_px - 0.4*CSIZE);
      draw_line(dp, 
          x_px - 0.1*CSIZE, y_px + 0.4*CSIZE, 
          x_px + 0.4*CSIZE, y_px - 0.4*CSIZE);
      break;
    case CT_LINE_LINE_ORTHOG:
      draw_line(dp, 
          x_px - 0.4*CSIZE, y_px + 0.4*CSIZE, 
          x_px + 0.4*CSIZE, y_px + 0.4*CSIZE);
      draw_line(dp, 
          x_px, y_px + 0.4*CSIZE, 
          x_px, y_px - 0.4*CSIZE);
      break;
    case CT_LINE_LENGTH:
      draw_line(dp, 
          x_px - 0.25*CSIZE, y_px - 0.4*CSIZE, 
          x_px - 0.25*CSIZE, y_px + 0.4*CSIZE);
      draw_line(dp, 
          x_px - 0.25*CSIZE, y_px + 0.4*CSIZE, 
          x_px + 0.25*CSIZE, y_px + 0.4*CSIZE);
      break;
    default:
      ;
  }

}

void draw_equal_constraint(gui_t *gui, double x, double y)
{
  draw_ptr dp = gui->drawer;
  draw_set_color(dp, 0.7, 0.7, 0.7);
  double x_px = user_to_px_x(gui, x);
  double y_px = user_to_px_y(gui, y);
  draw_rectangle_filled(dp, 
      x_px - 0.5*CSIZE, y_px - 0.5*CSIZE, 
      x_px + 0.5*CSIZE, y_px + 0.5*CSIZE );
  draw_set_line_width(dp, 1);
  draw_set_color(dp, 0, 0, 0);
  draw_line(dp, 
      x_px - 0.4*CSIZE, y_px - 0.2*CSIZE, 
      x_px + 0.4*CSIZE, y_px - 0.2*CSIZE);
  draw_line(dp, 
      x_px - 0.4*CSIZE, y_px + 0.2*CSIZE, 
      x_px + 0.4*CSIZE, y_px + 0.2*CSIZE);
}

void draw_constraint(gui_t *gui, constraint_t *c)
{
  draw_ptr dp = gui->drawer;
  switch(c->type)
  {
    case CT_LINE_LINE_EQUAL:
      //printf("drawing equal constraint\n");
      draw_line_line_connector(gui, c->line1, c->line2, c->type);
      draw_line_constraint(gui, c->line1, c->type);
      draw_line_constraint(gui, c->line2, c->type);
      break;
    case CT_LINE_LINE_PARALLEL:
      //printf("drawing parallel constraint\n");
      draw_line_line_connector(gui, c->line1, c->line2, c->type);
      draw_line_constraint(gui, c->line1, c->type);
      draw_line_constraint(gui, c->line2, c->type);
      break;
    case CT_LINE_LINE_ORTHOG:
      //printf("drawing orthogonal constraint\n");
      draw_line_line_connector(gui, c->line1, c->line2, c->type);
      draw_line_constraint(gui, c->line1, c->type);
      draw_line_constraint(gui, c->line2, c->type);
      break;
    case CT_LINE_HORIZ:
      //printf("drawing horizontal constraint\n");
      draw_line_constraint(gui, c->line1, c->type);
      break;
    case CT_LINE_VERT:
      //printf("drawing vertical constraint\n");
      draw_line_constraint(gui, c->line1, c->type);
      break;
    case CT_LINE_LENGTH:
      //printf("drawing line length constraint\n");
      draw_line_constraint(gui, c->line1, c->type);
      break;
    default:
      ;
  }
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

  // draw constraints (if desired)
  if(gui->draw_constraints)
  {
    for(i=0; i < app_data.constraint_count; i++)
    {
      draw_constraint(gui, app_data.constraints[i]);
    }
  }

  // draw the currently highlighted object (if there is one)
  switch(gui->state.highlight_sel.type)
  {
    case SELECT_TYPE_NONE:
      break;
    case SELECT_TYPE_LINE:
      {
        sketch_base_t *line = (sketch_base_t *)gui->state.highlight_sel.object;
        line->is_highlighted = 1;
        draw_sketch_line(line, gui);
        line->is_highlighted = 0;
      }
      break;
    case SELECT_TYPE_POINT:
      {
        sketch_base_t *pt = (sketch_base_t *)gui->state.highlight_sel.object;
        pt->is_highlighted = 1;
        draw_sketch_point(pt, gui);
        pt->is_highlighted = 0;
      }
      break;
    default:
      printf("don't know how to draw this type of highlight object (%d)\n", 
          gui->state.highlight_sel.type);
  }

  draw_finish(dp);
  return TRUE;
}

static void state_init(struct _state *s)
{
  s->draw_active = false;
  s->active_tool = TOOL_NONE;
  s->selection_count = 0;
  s->highlight_sel.type = SELECT_TYPE_NONE;
  s->last_highlight_sel.type = SELECT_TYPE_NONE;
}

static GtkWidget *toolbar_button_new(const char *image_path, int size, 
    const char *label, int toggle, 
    gboolean (*cb)(GtkWidget *, gpointer),
    gpointer user_data)
{
  GtkToolItem *b;
  if(toggle)
    b = gtk_toggle_tool_button_new();
  else
    b = gtk_tool_button_new(NULL, label);

  GdkPixbuf *pb = NULL;
  if(image_path != NULL)
  {
    pb = gdk_pixbuf_new_from_file(image_path, NULL);
    if(pb != NULL)
    {
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
      fprintf(stderr,
          "Error loading button image file. Falling back to text label...\n");
    }
  }

  if(pb == NULL)
  {
    gtk_tool_item_set_is_important((GtkToolItem *)b, TRUE);
    gtk_tool_button_set_label_widget((GtkToolButton *)b, NULL);
    gtk_tool_button_set_label((GtkToolButton *)b, label);
  }

  if(cb != NULL)
  {
    g_signal_connect((GtkWidget *)b, toggle ? "toggled" : "clicked", G_CALLBACK(cb), user_data);
  }
  return (GtkWidget *)b;
}

gboolean tools_cb(GtkWidget *w, gpointer data)
{
  printf("in tools callback!\n");
  return TRUE;
}

void make_tools_toolbar(gui_t *self)
{
  struct _tools_tb *p = &(self->tools_tb);
  p->tb = gtk_toolbar_new();

  p->select_btn = toolbar_button_new(NULL, 30, "Select", 1, tools_cb, self);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->select_btn, -1);

  p->line_btn = toolbar_button_new("button_icon.svg", 30, "line", 1, tools_cb, self);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->line_btn, -1);

  p->arc_btn = toolbar_button_new("dummy_filename.svg", 30, "Arc", 1, tools_cb, self);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->arc_btn, -1);

  gtk_box_pack_start(GTK_BOX(self->top_level_vbox), p->tb, FALSE, FALSE, 0);
}

static void add_constraint(constraint_t *c)
{
  app_data.constraints[app_data.constraint_count++] = c;
  app_data.constraints_dirty = 1;
}

static void update_constraints(void)
{
  if(app_data.constraint_count < 1)
    return;

  if(app_data.constraints_dirty)
  {
    solver_fini(app_data.solver);
    solver_init(app_data.solver, app_data.constraints, 
        app_data.constraint_count);
    solver_set_iterate_cb(app_data.solver, NULL, (void*)(app_data.solver));
    app_data.constraints_dirty = 0;
  }
  //solver_set_initial(app_data.solver);
  solver_solve(app_data.solver);
}

gboolean constraint_cb(GtkWidget *w, gpointer data)
{
  gui_t  *gui = (gui_t *)data;
  printf("in constraint callback!\n");

  // don't bother if nothing's selected
  if(gui->state.selection_count == 0)
  {
    printf("no objects selected!\n");
    return TRUE;
  }

  if(w == gui->constraint_tb.coinc_btn)
  {
    printf("coinc btn\n");
    if(gui->state.selection_count != 2)
    {
      printf("coincident constraint requires 2 objects selected\n");
      return TRUE;
    }
    if(gui->state.selections[0].type == SELECT_TYPE_POINT &&
       gui->state.selections[1].type == SELECT_TYPE_POINT)
    {
      constraint_t *c = constraint_alloc();
      assert(c != NULL);
      constraint_init_p_p_coinc(c,
          (sketch_point_t *)(gui->state.selections[0].object),
          (sketch_point_t *)(gui->state.selections[1].object) );

      add_constraint(c);
      update_constraints();
      gtk_widget_queue_draw(gui->canvas);
    }
    if( (gui->state.selections[0].type == SELECT_TYPE_LINE &&
         gui->state.selections[1].type == SELECT_TYPE_POINT) ||
        (gui->state.selections[1].type == SELECT_TYPE_LINE &&
         gui->state.selections[0].type == SELECT_TYPE_POINT) )
    {
      constraint_t *c = constraint_alloc();
      assert(c != NULL);

      sketch_line_t *l;
      sketch_point_t *p;
      if(gui->state.selections[0].type == SELECT_TYPE_LINE)
      {
        l = (sketch_line_t *)gui->state.selections[0].object;
        p = (sketch_point_t *)gui->state.selections[1].object;
      }
      else
      {
        l = (sketch_line_t *)gui->state.selections[1].object;
        p = (sketch_point_t *)gui->state.selections[0].object;
      }

      constraint_init_l_p_coinc(c, l, p);
      add_constraint(c);
      update_constraints();
      gtk_widget_queue_draw(gui->canvas);
    }
    else
    {
      printf("unsupported object types for coincident constraint\n");
      return TRUE;
    }
  }
  else if(w == gui->constraint_tb.horiz_btn)
  {
    printf("horiz btn\n");
    int i;
    int all_lines = 1;
    for(i=0; i < gui->state.selection_count; i++)
    {
      if(gui->state.selections[i].type != SELECT_TYPE_LINE)
      {
        all_lines = 0;
        break;
      }
    }
    if(!all_lines)
    {
      printf("horizontal constraint requires only lines selected\n");
      return TRUE;
    }
    for(i=0; i < gui->state.selection_count; i++)
    {
      constraint_t *c = constraint_alloc();
      assert(c != NULL);
      constraint_init_line_horiz(c,
          (sketch_line_t *)(gui->state.selections[i].object) );
      add_constraint(c);
    }

    update_constraints();
    gtk_widget_queue_draw(gui->canvas);

  }
  else if(w == gui->constraint_tb.vert_btn)
  {
    printf("vert btn\n");
    int i;
    int all_lines = 1;
    for(i=0; i < gui->state.selection_count; i++)
    {
      if(gui->state.selections[i].type != SELECT_TYPE_LINE)
      {
        all_lines = 0;
        break;
      }
    }
    if(!all_lines)
    {
      printf("vertical constraint requires only lines selected\n");
      return TRUE;
    }
    for(i=0; i < gui->state.selection_count; i++)
    {
      constraint_t *c = constraint_alloc();
      assert(c != NULL);
      constraint_init_line_vert(c,
          (sketch_line_t *)(gui->state.selections[i].object) );
      add_constraint(c);
    }

    update_constraints();
    gtk_widget_queue_draw(gui->canvas);
  }
  else if(w == gui->constraint_tb.distance_btn)
  {
    printf("distance btn\n");
    int i;
    int all_lines = 1;
    for(i=0; i < gui->state.selection_count; i++)
    {
      if(gui->state.selections[i].type != SELECT_TYPE_LINE)
      {
        all_lines = 0;
        break;
      }
    }
    if(!all_lines)
    {
      printf("distance constraint requires only lines selected\n");
      return TRUE;
    }
    for(i=0; i < gui->state.selection_count; i++)
    {
      sketch_line_t *l = (sketch_line_t *)gui->state.selections[i].object;

      int ret;
      double dist = sketch_line_get_length(l);
      ret = param_dialog("distance", PARAM_TYPE_FLOAT, &dist);

      if(ret == 0)
      {
        constraint_t *c = constraint_alloc();
        assert(c != NULL);
        constraint_init_line_length(c, l, dist);
        add_constraint(c);
      }
    }

    update_constraints();
    gtk_widget_queue_draw(gui->canvas);

  }
  else if(w == gui->constraint_tb.parallel_btn)
  {
    printf("parallel btn\n");
    if(gui->state.selection_count != 2)
    {
      printf("parallel constraint requires 2 objects selected\n");
      return TRUE;
    }
    if(gui->state.selections[0].type == SELECT_TYPE_LINE &&
       gui->state.selections[1].type == SELECT_TYPE_LINE)
    {
      constraint_t *c = constraint_alloc();
      assert(c != NULL);
      constraint_init_l_l_parallel(c,
          (sketch_line_t *)(gui->state.selections[0].object),
          (sketch_line_t *)(gui->state.selections[1].object) );

      add_constraint(c);
      update_constraints();
      gtk_widget_queue_draw(gui->canvas);
    }
    else
    {
      printf("unsupported object types for parallel constraint\n");
      return TRUE;
    }
  }
  else if(w == gui->constraint_tb.perp_btn)
  {
    printf("perp btn\n");
    if(gui->state.selection_count != 2)
    {
      printf("perp constraint requires 2 objects selected\n");
      return TRUE;
    }
    if(gui->state.selections[0].type == SELECT_TYPE_LINE &&
       gui->state.selections[1].type == SELECT_TYPE_LINE)
    {
      constraint_t *c = constraint_alloc();
      assert(c != NULL);
      constraint_init_l_l_perp(c,
          (sketch_line_t *)(gui->state.selections[0].object),
          (sketch_line_t *)(gui->state.selections[1].object) );

      add_constraint(c);
      update_constraints();
      gtk_widget_queue_draw(gui->canvas);
    }
    else
    {
      printf("unsupported object types for perp constraint\n");
      return TRUE;
    }
  }
  else if(w == gui->constraint_tb.equal_btn)
  {
    printf("equal btn\n");
    if(gui->state.selection_count != 2)
    {
      printf("equal constraint requires 2 objects selected\n");
      return TRUE;
    }
    if(gui->state.selections[0].type == SELECT_TYPE_LINE &&
       gui->state.selections[1].type == SELECT_TYPE_LINE)
    {
      constraint_t *c = constraint_alloc();
      assert(c != NULL);
      constraint_init_l_l_equal(c,
          (sketch_line_t *)(gui->state.selections[0].object),
          (sketch_line_t *)(gui->state.selections[1].object) );

      add_constraint(c);
      update_constraints();
      gtk_widget_queue_draw(gui->canvas);
    }
    else
    {
      printf("unsupported object types for line constraint\n");
      return TRUE;
    }
  }
  else
  {
    printf("unknown btn\n");
  }
  return TRUE;
}

void make_constraint_toolbar(gui_t *self)
{
  struct _constraint_tb *p = &(self->constraint_tb);
  p->tb = gtk_toolbar_new();

  p->coinc_btn = toolbar_button_new(NULL, 30, "Coinc.", 0, constraint_cb, self);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->coinc_btn, -1);

  p->horiz_btn = toolbar_button_new(NULL, 30, "--", 0, constraint_cb, self);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->horiz_btn, -1);

  p->vert_btn = toolbar_button_new(NULL, 30, "|", 0, constraint_cb, self);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->vert_btn, -1);

  p->parallel_btn = toolbar_button_new(NULL, 30, "//", 0, constraint_cb, self);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->parallel_btn, -1);

  p->perp_btn = toolbar_button_new(NULL, 30, "-|", 0, constraint_cb, self);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->perp_btn, -1);

  p->equal_btn = toolbar_button_new(NULL, 30, "=", 0, constraint_cb, self);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->equal_btn, -1);

  p->distance_btn = toolbar_button_new(NULL, 30, "Dist.", 0, constraint_cb, self);
  gtk_toolbar_insert((GtkToolbar *)p->tb, (GtkToolItem *)p->distance_btn, -1);

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

  //make_tools_toolbar(self);
  make_constraint_toolbar(self);

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

  /* Polyline Button */
  bb->polyline_btn = gtk_button_new_with_label("Polyline");
  gtk_box_pack_start(GTK_BOX(bb->hbox), bb->polyline_btn, FALSE, FALSE, 0);
  g_signal_connect(bb->polyline_btn, "clicked", G_CALLBACK(polyline_cb), self);

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
  sb->left_label = gtk_label_new("tool: select");
  gtk_box_pack_start(GTK_BOX(sb->hbox), sb->left_label, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(sb->hbox), gtk_vseparator_new(), FALSE, FALSE, 0);
  sb->right_label = gtk_label_new("Hello World");
  gtk_box_pack_start(GTK_BOX(sb->hbox), sb->right_label, FALSE, FALSE, 0);

  /* Finalize the GUI setup */
  g_signal_connect(self->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_widget_show_all(self->window);

  self->xmin = -10;
  self->xmax = +10;
  self->ymin = -10;
  self->ymax = +10;

  self->panning = 0;
  self->dragging = 0;

  self->draw_constraints = 1;

  return 0;
}

void gui_run(gui_t *self)
{
  gtk_main();
}
