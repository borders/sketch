#include "param_dialog.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

static char *value_to_string(void *value, param_type_t type)
{
  static char str[255];
  switch(type)
  {
    case PARAM_TYPE_FLOAT:
      snprintf(str, sizeof(str), "%lg", *((double *)value));
      break;
    case PARAM_TYPE_INT:
      snprintf(str, sizeof(str), "%d", *((int *)value));
      break;
    case PARAM_TYPE_STRING:
      strcpy(str, (char *)value);
      break;
  }
  printf("value string: %s\n", str);
  return str;
}

static int parse_value(const char *str, param_type_t type, void *param_val)
{
  switch(type)
  {
    case PARAM_TYPE_FLOAT:
      {
        errno = 0;
        char *endptr;
        double d = strtod(str, &endptr);
        if(errno || (endptr == str) || (*endptr != '\0'))
        {
          printf("error parsing float from string: %s\n", str);
          return -1;
        }
        *((double *)param_val) = d;
        printf("parsed float value: %lg\n", d);
      }
      break;
    case PARAM_TYPE_INT:
      {
        errno = 0;
        char *endptr;
        long int l = strtol(str, &endptr, 0);
        if(errno || (endptr == str) || (*endptr != '\0'))
        {
          printf("error parsing int from string: %s\n", str);
          return -1;
        }
        *((int *)param_val) = (int)l;
        printf("parsed int value: %d\n", (int)l);
      }
      break;
    case PARAM_TYPE_STRING:
      strncpy((char *)param_val, str, 100);
        printf("parsed string value: %s\n", str);
      break;
    default:
      printf("unknown type to parse from string!\n");
      return -1;
  }
  return 0;
}

int param_dialog(char *param_name, param_type_t type, void *param_val)
{
  GtkWidget *dialog, *content_area, *entry;
  char title[255];
  int ret;

  snprintf(title, sizeof(title), "%s=...", param_name);

  dialog = gtk_dialog_new_with_buttons (title, NULL,
            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
            GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
            NULL);

  content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  entry = gtk_entry_new();

  gtk_container_add (GTK_CONTAINER (content_area), entry);

  gtk_widget_show_all(dialog);

  int done = 0;
  while(!done)
  {
    gtk_entry_set_text((GtkEntry *)entry, 
        value_to_string(param_val, type));
    gtk_widget_grab_focus(entry);
    gint result = gtk_dialog_run(GTK_DIALOG (dialog));
    switch (result)
    {
      case GTK_RESPONSE_ACCEPT:
        ret = parse_value(gtk_entry_get_text((GtkEntry *)entry), type, param_val);
        if(ret == 0)
          done = 1;
        break;
      case GTK_RESPONSE_REJECT:
        printf("cancelled\n");
        done = 1;
        break;
      default:
        printf("unknown response code: %d\n", result);
        done = 1;
        break;
    }
  }
  gtk_widget_destroy(dialog);

  return 0;
}

