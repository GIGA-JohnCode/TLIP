#include "tlip.h"

#include <gtk/gtk.h>
#include <stdbool.h>
#include <strings.h>

bool cli_mode = false;
GtkWindow *main_window = NULL;

int main(int argc, char* argv[])
{
    if (argc == 1)
        return gui_main();
    else if (argc <= 6 && !strcasecmp(argv[1], "--cli")) // strcasecmp() returns 0 on match
    {
        cli_mode = true;
        return cli_main(argc, argv);
    }
    else
    {
        printf("Usage: ./tlip --cli path/to/image width(px) height(px) size_constraint(KB)");
        return -1;
    }
}

