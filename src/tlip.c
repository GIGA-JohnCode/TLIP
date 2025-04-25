#include "tlip.h"

#include <gtk/gtk.h>
#include <stdbool.h>
#include <strings.h>

bool cli_mode == false;
GtkWindow *main_window = NULL;

int main(int argc, char* argv[])
{
    if (argc == 1)
        return gui_main();
    else if (argc == 2 && !strcasecmp(argv[1], "--cli")) // strcasecmp() returns 0 on match
        return cli_main();
    else
    {
        printf("Usage: ./tlip --cli");
        return -1;
    }
}

