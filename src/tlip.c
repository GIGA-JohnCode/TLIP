#include "tlip.h"

#include <gtk/gtk.h>
#include <stdbool.h>

bool cli_mode == false;
GtkWindow *main_window = NULL;

int main(int argc, char* argv[])
{
    // condition to be added
    gui_main();
}

