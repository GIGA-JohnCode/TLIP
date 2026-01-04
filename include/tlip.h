#pragma once

#include "jpeg.h"
#include "process.h"
#include "types.h"
#include "util.h"

#include <gtk/gtk.h>
#include <stdbool.h>

extern bool cli_mode;
extern bool individual_input;
extern bool suppress_view;
extern GtkWindow *main_window;

int cli_main(int argc, char *argv[]);
int gui_main(void);
