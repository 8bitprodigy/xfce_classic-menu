/* applications-menu.c - Applications menu implementation */

#include "classic-menu.h"
#include <garcon/garcon.h>

/* Launch application callback */
static void
on_application_activate(GtkMenuItem *item, gpointer user_data)
{
    GarconMenuItem *garcon_item = GARCON_MENU_ITEM(user_data);
    const gchar *command;
    GError *error = NULL;

    command = garcon_menu_item_get_command(garcon_item);

    if (command == NULL) {
        g_warning("No command found for menu item");
        return;
    }

    if (!g_spawn_command_line_async(command, &error)) {
        g_warning("Failed to launch application: %s", error ? error->message : "Unknown error");
        if (error) {
            g_error_free(error);
        }
    }
}

/* Search/Run dialog callback */
static void
on_search_activate(GtkMenuItem *item, gpointer user_data)
{
    GError *error = NULL;

    if (!g_spawn_command_line_async("xfce4-appfinder", &error)) {
        g_warning("Failed to launch application finder: %s", error ? error->message : "Unknown error");
        if (error) {
            g_error_free(error);
        }
    }
}

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

/* Recursively build menu from elements */
static void
populate_menu_from_elements(GtkWidget *menu, GList *elements)
{
    GList *li;

    for (li = elements; li != NULL; li = li->next) {
        GarconMenuElement *element = GARCON_MENU_ELEMENT(li->data);

        if (GARCON_IS_MENU_SEPARATOR(element)) {
            GtkWidget *separator = gtk_separator_menu_item_new();
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
        }
        else if (GARCON_IS_MENU(element)) {
            /* Submenu (category) */
            GarconMenu *submenu_data = GARCON_MENU(element);
            GarconMenuDirectory *directory;
            GtkWidget *category_item;
            GtkWidget *category_submenu;
            const gchar *category_name;
            const gchar *category_icon;
            GList *submenu_elements;

            directory = garcon_menu_get_directory(submenu_data);
            if (directory == NULL) {
                continue;
            }

            /* Check if directory is visible */
            if (!garcon_menu_element_get_visible(GARCON_MENU_ELEMENT(directory))) {
                continue;
            }

            category_name = garcon_menu_element_get_name(GARCON_MENU_ELEMENT(directory));
            category_icon = garcon_menu_element_get_icon_name(GARCON_MENU_ELEMENT(directory));

            /* Get items in this submenu */
            submenu_elements = garcon_menu_get_elements(submenu_data);
            if (submenu_elements == NULL) {
                continue;
            }

            /* Create category menu item */
            category_item = create_menu_item_with_icon(category_name, category_icon);

            /* Build submenu recursively */
            category_submenu = gtk_menu_new();
            populate_menu_from_elements(category_submenu, submenu_elements);

            gtk_menu_item_set_submenu(GTK_MENU_ITEM(category_item), category_submenu);
            gtk_menu_shell_append(GTK_MENU_SHELL(menu), category_item);
        }
        else if (GARCON_IS_MENU_ITEM(element)) {
            /* Menu item (application) */
            GarconMenuItem *garcon_item = GARCON_MENU_ITEM(element);
            GtkWidget *menu_item;
            const gchar *name;
            const gchar *icon_name;

            /* Check if item is visible */
            if (!garcon_menu_element_get_visible(element)) {
                continue;
            }

            name = garcon_menu_element_get_name(element);
            icon_name = garcon_menu_element_get_icon_name(element);

            menu_item = create_menu_item_with_icon(name, icon_name);

            g_signal_connect(G_OBJECT(menu_item), "activate",
                            G_CALLBACK(on_application_activate), garcon_item);

            gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
        }
    }
}

/* Build the Applications menu */
GtkWidget *
build_applications_menu(GarconMenu **garcon_menu_ptr)
{
    GtkWidget *main_menu;
    GtkWidget *search_item;
    GtkWidget *separator;
    GList *elements;

    main_menu = gtk_menu_new();

    /* Add search item at the top */
    search_item = create_menu_item_with_icon("Search for applications...", "system-search");
    g_signal_connect(G_OBJECT(search_item), "activate",
                    G_CALLBACK(on_search_activate), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), search_item);

    /* Add separator */
    separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(main_menu), separator);

    /* Load the applications menu */
    if (*garcon_menu_ptr == NULL) {
        *garcon_menu_ptr = garcon_menu_new_applications();
    }

    if (!garcon_menu_load(*garcon_menu_ptr, NULL, NULL)) {
        g_warning("Failed to load applications menu");
        gtk_widget_show_all(main_menu);
        return main_menu;
    }

    /* Get all menu elements */
    elements = garcon_menu_get_elements(*garcon_menu_ptr);

    /* Recursively populate the menu */
    populate_menu_from_elements(main_menu, elements);

    gtk_widget_show_all(main_menu);
    return main_menu;
}
