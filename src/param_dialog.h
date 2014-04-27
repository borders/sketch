#if !defined(__PARAM_DIALOG_H__)
#define __PARAM_DIALOG_H__

#include <gtk/gtk.h>

typedef enum
{
  PARAM_TYPE_FLOAT,
  PARAM_TYPE_INT,
  PARAM_TYPE_STRING,

  NUM_PARAM_TYPES
} param_type_t;

int param_dialog(char *param_name, param_type_t type, void *param_val);

#endif
