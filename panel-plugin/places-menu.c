/* places-menu.c - Places menu implementation */

#include "classic-menu.h"

/* Create a menu item with icon and label */
static GtkWidget *
create_menu_item_with_icon(const gchar *label, const gchar *icon_name)
{
    GtkWidget *menu_item;
    GtkWidget *box;
    GtkWidget *icon;
    GtkWidget *label_widget;

    menu_item = gtk_menu_item_new();
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);

    if (icon_name != NULL) {
        icon = gtk_image_new_from_icon_name(icon_name, GTK_ICON_SIZE_MENU);
        gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
    }

    label_widget = gtk_label_new(label);
    gtk_label_set_xalign(GTK_LABEL(label_widget), 0.0);
    gtk_box_pack_start(GTK_BOX(box), label_widget, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(menu_item), box);
    gtk_widget_show_all(menu_item);

    return menu_item;
}

/* Open a location in the file manager */
static void
on_place_activate(GtkMenuItem *item, gpointer user_data)
{
    const gchar *uri = (const gchar *)user_data;
    GError *error = NULL;
    gchar *command;

    /* Use exo-open to launch file manager */
    command = g_strdup_printf("exo-open --launch FileManager '%s'", uri);

    if (!g_spawn_command_line_async(command, &error)) {
        g_warning("Failed to open location: %s", error ? error->message : "Unknown error");
        if (error) {
            g_error_free(error);
        }
    }

    g_free(command);
}

/* Build the Places menu */
GtkWidget *
build_places_menu(void)
{
    GtkWidget *submenu = gtk_menu_new();
    GtkWidget *item;
    const gchar *home_dir = g_get_home_dir();
    gchar *path;

    /* Home folder */
    item = create_menu_item_with_icon("Home Folder", "user-home");
    g_signal_connect(G_OBJECT(item), "activate",
                    G_CALLBACK(on_place_activate), (gpointer)home_dir);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

    /* Desktop */
    path = g_build_filename(home_dir, "Desktop", NULL);
    item = create_menu_item_with_icon("Desktop", "user-desktop");
    g_signal_connect(G_OBJECT(item), "activate",
                    G_CALLBACK(on_place_activate), (gpointer)path);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

    /* Documents */
    path = g_build_filename(home_dir, "Documents", NULL);
    item = create_menu_item_with_icon("Documents", "folder-documents");
    g_signal_connect(G_OBJECT(item), "activate",
                    G_CALLBACK(on_place_activate), (gpointer)path);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

    /* Downloads */
    path = g_build_filename(home_dir, "Downloads", NULL);
    item = create_menu_item_with_icon("Downloads", "folder-download");
    g_signal_connect(G_OBJECT(item), "activate",
                    G_CALLBACK(on_place_activate), (gpointer)path);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

    /* Separator */
    item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

    /* File Manager */
    item = create_menu_item_with_icon("Browse Files", "system-file-manager");
    g_signal_connect(G_OBJECT(item), "activate",
                    G_CALLBACK(on_place_activate), (gpointer)home_dir);
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

    gtk_widget_show_all(submenu);
    return submenu;
}
