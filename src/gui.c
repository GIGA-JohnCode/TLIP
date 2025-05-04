#include "tlip.h"

#include <gtk/gtk.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static GtkWidget *input_file_entry;
static GtkWidget *width_info_label;
static GtkWidget *width_entry;
static GtkWidget *height_info_label;
static GtkWidget *height_entry;
static GtkWidget *size_info_label;
static GtkWidget *size_entry;
static GtkWidget *output_file_entry;

static void on_activate(GtkApplication *app);
static void on_input_browse_clicked(GtkButton *button, gpointer user_data);
static void input_file_selected(GObject *source, GAsyncResult *res, gpointer user_data);
static void update_image_info(int width, int height, size_t file_size);
static void on_output_browse_clicked(GtkButton *button, gpointer user_data);
static void output_file_selected(GObject *source, GAsyncResult *res, gpointer user_data);
static void on_process_clicked(GtkButton *button, gpointer user_data);

int gui_main(void)
{
    GtkApplication * app = gtk_application_new("com.example.tlip", G_APPLICATION_DEFAULT_FLAGS);
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

            //GtkWidget *dimension_info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
            GtkWidget *dimension_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 55);
                GtkWidget *width_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
                    GtkWidget *width_info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                        GtkWidget *width_info_label_static = gtk_label_new("Current Width:");
                        /*Global*/ width_info_label = gtk_label_new("-");
                    GtkWidget *width_entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                        GtkWidget *width_label = gtk_label_new("Width:");
                        /*Global*/ width_entry = gtk_entry_new();
                GtkWidget *height_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
                    GtkWidget *height_info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                        GtkWidget *height_info_label_static = gtk_label_new("Current Height:");
                        /*Global*/ height_info_label = gtk_label_new("-");
                    GtkWidget *height_entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                        GtkWidget *height_label = gtk_label_new("Height:");
                        /*Global*/ height_entry = gtk_entry_new();

            GtkWidget *size_info_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
                GtkWidget *size_info_label_static = gtk_label_new("Current Size:");
                /*Global*/ size_info_label = gtk_label_new("-");

            GtkWidget *size_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                GtkWidget *size_label = gtk_label_new("Max Size (KB):");
                /*Global*/ size_entry = gtk_entry_new();

            GtkWidget *output_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
                GtkWidget *output_label = gtk_label_new("Save to:");
                /*Global*/ output_file_entry = gtk_entry_new();
                    gtk_widget_set_hexpand(output_file_entry, TRUE);
                GtkWidget *output_browse_button = gtk_button_new_with_label("Browse");
                    g_signal_connect(output_browse_button, "clicked", G_CALLBACK(on_output_browse_clicked), NULL);

            GtkWidget *hints_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
                GtkWidget *hint_0 = gtk_label_new("HINTS");
                GtkWidget *hint_1 = gtk_label_new("1. Leaving Width/Height/Size field/s empty means current value/s will be used.");
                GtkWidget *hint_2 = gtk_label_new("2. Input 0(zero) in size field to remove size limit during encoding.");
                GtkWidget *hint_3 = gtk_label_new("3. Leaving the 'save to' field empty will make a duplicate.");

            GtkWidget *process_button = gtk_button_new_with_label("Process");
                g_signal_connect(process_button, "clicked", G_CALLBACK(on_process_clicked), NULL);


                        gtk_box_append(GTK_BOX(width_info_box), width_info_label_static);
                        gtk_box_append(GTK_BOX(width_info_box), width_info_label);

                        gtk_box_append(GTK_BOX(width_entry_box), width_label);
                        gtk_box_append(GTK_BOX(width_entry_box), width_entry);

                        gtk_box_append(GTK_BOX(height_info_box), height_info_label_static);
                        gtk_box_append(GTK_BOX(height_info_box), height_info_label);

                        gtk_box_append(GTK_BOX(height_entry_box), height_label);
                        gtk_box_append(GTK_BOX(height_entry_box), height_entry);

                    gtk_box_append(GTK_BOX(height_box), height_info_box);
                    gtk_box_append(GTK_BOX(height_box), height_entry_box);

                    gtk_box_append(GTK_BOX(width_box), width_info_box);
                    gtk_box_append(GTK_BOX(width_box), width_entry_box);

                gtk_box_append(GTK_BOX(hints_box), hint_0);
                gtk_box_append(GTK_BOX(hints_box), hint_1);
                gtk_box_append(GTK_BOX(hints_box), hint_2);
                gtk_box_append(GTK_BOX(hints_box), hint_3);

                gtk_box_append(GTK_BOX(output_box), output_label);
                gtk_box_append(GTK_BOX(output_box), output_file_entry);
                gtk_box_append(GTK_BOX(output_box), output_browse_button);

                gtk_box_append(GTK_BOX(size_box), size_label);
                gtk_box_append(GTK_BOX(size_box), size_entry);

                gtk_box_append(GTK_BOX(size_info_box), size_info_label_static);
                gtk_box_append(GTK_BOX(size_info_box), size_info_label);

                gtk_box_append(GTK_BOX(dimension_box), width_box);
                gtk_box_append(GTK_BOX(dimension_box), height_box);

                gtk_box_append(GTK_BOX(input_box), input_label);
                gtk_box_append(GTK_BOX(input_box), input_file_entry);
                gtk_box_append(GTK_BOX(input_box), input_browse_button);

            gtk_box_append(GTK_BOX(main_box), input_box);
            gtk_box_append(GTK_BOX(main_box), dimension_box);
            gtk_box_append(GTK_BOX(main_box), size_info_box);
            gtk_box_append(GTK_BOX(main_box), size_box);
            gtk_box_append(GTK_BOX(main_box), output_box);
            gtk_box_append(GTK_BOX(main_box), hints_box);
            gtk_box_append(GTK_BOX(main_box), process_button);

        gtk_window_set_child(GTK_WINDOW(window), main_box);

    gtk_window_present(GTK_WINDOW(window));
}

static void on_input_browse_clicked(GtkButton *button, gpointer user_data)
{
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Choose Image");
    gtk_file_dialog_open(dialog, GTK_WINDOW(main_window), NULL, input_file_selected, NULL);
    g_object_unref(dialog);
}

static void input_file_selected(GObject *source, GAsyncResult *res, gpointer user_data)
{
    GFile *file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source), res, NULL);
    if (file)
    {
        char *path = g_file_get_path(file);
        gtk_editable_set_text(GTK_EDITABLE(input_file_entry), path);

        rgb *palette = load_jpeg(path);
        if (palette)
        {
            update_image_info(palette->width, palette->height, palette->original_size);
            free(palette->buffer);
            free(palette);
        }
        g_free(path);
        g_object_unref(file);
    }
}

static void update_image_info(int width, int height, size_t file_size)
{
    char width_text[30], height_text[32], size_text[32];

    snprintf(width_text, sizeof(width_text), "%i px", width);
    snprintf(height_text, sizeof(height_text), "%i px", height);
    snprintf(size_text, sizeof(size_text), "%.1f KB", file_size / 1024.0);

    gtk_label_set_text(GTK_LABEL(width_info_label), width_text);
    gtk_label_set_text(GTK_LABEL(height_info_label), height_text);
    gtk_label_set_text(GTK_LABEL(size_info_label), size_text);
}

static void on_output_browse_clicked(GtkButton *button, gpointer user_data)
{
    GtkFileDialog *dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, "Save Image");
    gtk_file_dialog_save(dialog, GTK_WINDOW(main_window), NULL, output_file_selected, NULL);
    g_object_unref(dialog);
}

static void output_file_selected(GObject *source, GAsyncResult *res, gpointer user_data)
{
    GFile *file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(source), res, NULL);
    if (file)
    {
        char *path = g_file_get_path(file);
        gtk_editable_set_text(GTK_EDITABLE(output_file_entry), path);
        g_free(path);
        g_object_unref(file);
    }
}

static void on_process_clicked(GtkButton *button, gpointer user_data)
{
    const char *input_path = gtk_editable_get_text(GTK_EDITABLE(input_file_entry));
    const char *width_str = gtk_editable_get_text(GTK_EDITABLE(width_entry));
    const char *height_str = gtk_editable_get_text(GTK_EDITABLE(height_entry));
    const char *size_str = gtk_editable_get_text(GTK_EDITABLE(size_entry));
    const char *temp_output_path = gtk_editable_get_text(GTK_EDITABLE(output_file_entry));
    char output_path[PATH_MAX];
    strcpy(output_path, temp_output_path);

    if (!input_path || input_path[0] == '\0')
    {
        show_error("Please select an input file");
        return;
    }

    if ((!width_str || width_str[0] == '\0') &&
        (!height_str || height_str[0] == '\0') &&
        (!size_str || size_str[0] == '\0'))
    {
        alert("ERROR", "Aborted\nNothing to process");
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

    store_jpeg(palette, target_size, (char*)output_path, (char*)input_path);

    free(palette->buffer);
    free(palette);
}

