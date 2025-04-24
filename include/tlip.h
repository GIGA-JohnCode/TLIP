#pragma once

#include "jpeg.h"
#include "process.h"
#include "types.h"
#include "util.h"

#include <gtk/gtk.h>
#include <stdbool.h>

extern bool cli_mode;
extern GtkWindow *main_window;

int cli_main(void);
int gui_main(void);
