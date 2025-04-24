#include "tlip.h"

int main(int argc, char* argv[])
{
    // condition to be added
    gui_main();
}


fprintf(stderr, "Error: Failed to open image at given path.\n");

// To this GTK dialog:
static void show_error_dialog(GtkWindow *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(parent,
                                             GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_ERROR,
                                             GTK_BUTTONS_OK,
                                             "Error");
    gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", message);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Then call it when an error occurs:
show_error_dialog(main_window, "Failed to open image at given path.");

void show_error(char* error_message)
{
    if (cli_mode)
        fprintf(stderr, error_message);
    else
    {
        
    }

}
