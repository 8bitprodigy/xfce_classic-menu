/* system-menu.c - System menu implementation */

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

/* System action callback */
static void
on_system_action(GtkMenuItem *item, gpointer user_data)
{
    const gchar *command = (const gchar *)user_data;
    GError *error = NULL;

    if (!g_spawn_command_line_async(command, &error)) {
        g_warning("Failed to execute command: %s", error ? error->message : "Unknown error");
        if (error) {
            g_error_free(error);
        }
    }
}

/* Build the System menu */
GtkWidget *
build_system_menu(void)
{
    GtkWidget *submenu = gtk_menu_new();
    GtkWidget *item;

    /* Settings Manager */
    item = create_menu_item_with_icon("Settings Manager", "preferences-system");
    g_signal_connect(G_OBJECT(item), "activate",
                    G_CALLBACK(on_system_action), "xfce4-settings-manager");
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

    /* Separator */
    item = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

    /* Lock Screen */
    item = create_menu_item_with_icon("Lock Screen", "system-lock-screen");
    g_signal_connect(G_OBJECT(item), "activate",
                    G_CALLBACK(on_system_action), "xflock4");
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

    /* Log Out */
    item = create_menu_item_with_icon("Log Out", "system-log-out");
    g_signal_connect(G_OBJECT(item), "activate",
                    G_CALLBACK(on_system_action), "xfce4-session-logout");
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), item);

    gtk_widget_show_all(submenu);
    return submenu;
}
