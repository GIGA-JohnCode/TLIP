#include "tlip.h"

#include <gtk/gtk.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

static GtkWidget *input_file_entry;
static GtkWidget *width_entry;
static GtkWidget *height_entry;
static GtkWidget *size_entry;
static GtkWidget *output_file_entry;

static void on_activate(GtkApplication *app);
static void on_input_browse_clicked(GtkButton *button, gpointer user_data);
static void on_output_browse_clicked(GtkButton *button, gpointer user_data);
static void on_process_clicked(GtkButton *button, gpointer user_data);

int gui_main(void)
{
    GtkApplication * app = gtk_application_new("com.example.tlip", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);

    int status = g_application_run(G_APPLICATION(app), 0, NULL);

    g_object_unref(app);
    return status;
}

static void on_activate(GtkApplication *app)
{
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "TLIP");
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 250);

    main_window = GTK_WINDOW(window);

        GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_widget_set_margin_start(main_box, 10);
        gtk_widget_set_margin_end(main_box, 10);
        gtk_widget_set_margin_top(main_box, 10);
        gtk_widget_set_margin_bottom(main_box, 10);

            GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                GtkWidget *input_label = gtk_label_new("File:");
                /*Global*/ input_file_entry = gtk_entry_new();
                    gtk_widget_set_hexpand(input_file_entry, TRUE);
                GtkWidget *input_browse_button = gtk_button_new_with_label("Browse");
                    g_signal_connect(input_browse_button, "clicked", G_CALLBACK(on_input_browse_clicked), NULL);

            GtkWidget *dimension_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                GtkWidget *width_label = gtk_label_new("Width:");
                /*Global*/ width_entry = gtk_entry_new();
                GtkWidget *height_label = gtk_label_new("Height:");
                /*Global*/ height_entry = gtk_entry_new();

            GtkWidget *size_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                GtkWidget *size_label = gtk_label_new("Max Size (KB):");
                /*Global*/ size_entry = gtk_entry_new();

            GtkWidget *output_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                GtkWidget *output_label = gtk_label_new("Save to:");
                /*Global*/ output_file_entry = gtk_entry_new();
                    gtk_widget_set_hexpand(output_file_entry, TRUE);
                GtkWidget *output_browse_button = gtk_button_new_with_label("Browse");
                    g_signal_connect(output_browse_button, "clicked", G_CALLBACK(on_output_browse_clicked), NULL);

            GtkWidget *process_button = gtk_button_new_with_label("Process");
                g_signal_connect(process_button, "clicked", G_CALLBACK(on_process_clicked), NULL);

                gtk_box_append(GTK_BOX(output_box), output_label);
                gtk_box_append(GTK_BOX(output_box), output_file_entry);
                gtk_box_append(GTK_BOX(output_box), output_browse_button);

                gtk_box_append(GTK_BOX(size_box), size_label);
                gtk_box_append(GTK_BOX(size_box), size_entry);

                gtk_box_append(GTK_BOX(dimension_box), width_label);
                gtk_box_append(GTK_BOX(dimension_box), width_entry);
                gtk_box_append(GTK_BOX(dimension_box), height_label);
                gtk_box_append(GTK_BOX(dimension_box), height_entry);

                gtk_box_append(GTK_BOX(input_box), input_label);
                gtk_box_append(GTK_BOX(input_box), input_file_entry);
                gtk_box_append(GTK_BOX(input_box), input_browse_button);

            gtk_box_append(GTK_BOX(main_box), input_box);
            gtk_box_append(GTK_BOX(main_box), dimension_box);
            gtk_box_append(GTK_BOX(main_box), size_box);
            gtk_box_append(GTK_BOX(main_box), output_box);
            gtk_box_append(GTK_BOX(main_box), process_button);

        gtk_window_set_child(GTK_WINDOW(window), main_box);

    gtk_widget_show(window);
}

static void on_input_browse_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Choose Image", GTK_WINDOW(main_window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Open", GTK_RESPONSE_ACCEPT,
        NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(input_file_entry), filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static void on_output_browse_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
        "Save Image", GTK_WINDOW(main_window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "Cancel", GTK_RESPONSE_CANCEL,
        "Save", GTK_RESPONSE_ACCEPT,
        NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        gtk_entry_set_text(GTK_ENTRY(output_file_entry), filename);
        g_free(filename);
    }

    gtk_widget_destroy(dialog);
}

static void on_process_clicked(GtkButton *button, gpointer user_data)
{
    const char *input_path = gtk_entry_get_text(GTK_ENTRY(input_file_entry));
    const char *width_str = gtk_entry_get_text(GTK_ENTRY(width_entry));
    const char *height_str = gtk_entry_get_text(GTK_ENTRY(height_entry));
    const char *size_str = gtk_entry_get_text(GTK_ENTRY(size_entry));
    const char *output_path = gtk_entry_get_text(GTK_ENTRY(output_file_entry));

    if (!input_path || input_path[0] == '\0')
    {
        show_error("Please select an input file");
        return;
    }

    if ((!width_str || width_str[0] == '\0') &&
        (!height_str || height_str[0] == '\0') &&
        (!size_str || size_str[0] == '\0'))
    {
        show_error("Please specify either dimension or max size");
        return;
    }

    rgb* palette = load_jpeg(input_path);
    if (!palette)
        return;

    int new_width, new_height;
    if (width_str && width_str[0] != '\0')
        new_width = atoi(width_str);
    else
        new_width = palette->width;
    if (height_str && height_str[0] != '\0')
        new_height = atoi(height_str);
    else
        new_height = palette->height;

    if (!resize(palette, new_width, new_height))
    {
        free(palette->buffer);
        free(palette);
        return;
    }

    size_t target_size;
    if (size_str && size_str[0] != '\0')
        target_size = 1024 * atoi(size_str);
    else
        target_size = SIZE_MAX;

    if (output_path && output_path[0] != '\0')
    {
        store_jpeg(palette, target_size, (char*)output_path);
    }
    else
        store_jpeg(palette, target_size, (char*)input_path);

    free(palette->buffer);
    free(palette);
}

