/* classic-menu.c - Main plugin file */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>
#include <garcon/garcon.h>

#include "classic-menu.h"

/* Plugin structure */
typedef struct {
    XfcePanelPlugin *plugin;

    GtkWidget *menubar;
    GtkWidget *applications_item;
    GtkWidget *places_item;
    GtkWidget *system_item;

    GarconMenu *garcon_menu;
} 
ClassicMenuPlugin;

/* Forward declarations */
static void     classic_menu_construct(          XfcePanelPlugin *plugin);
static void     classic_menu_free(               XfcePanelPlugin *plugin, ClassicMenuPlugin *menu);
static gboolean classic_menu_size_changed(       XfcePanelPlugin *plugin, gint               size,        ClassicMenuPlugin *menu);
static void     classic_menu_orientation_changed(XfcePanelPlugin *plugin, GtkOrientation     orientation, ClassicMenuPlugin *menu);

/* Plugin construction */
static void
classic_menu_construct(XfcePanelPlugin *plugin)
{
    ClassicMenuPlugin *menu;
    GtkWidget 
                      *applications_menu, 
                      *places_menu, 
                      *system_menu;

    /* Allocate plugin structure */
    menu = g_new0(ClassicMenuPlugin, 1);
    menu->plugin = plugin;

    /* Create menubar */
    menu->menubar = gtk_menu_bar_new();
    gtk_container_add(GTK_CONTAINER(plugin), menu->menubar);
    xfce_panel_plugin_add_action_widget(plugin, menu->menubar);

    /* Create Applications menu item */
    menu->applications_item = gtk_menu_item_new_with_label("Applications");
    applications_menu       = build_applications_menu(&menu->garcon_menu);
    gtk_menu_item_set_submenu(
            GTK_MENU_ITEM(menu->applications_item), 
            applications_menu
        );
    gtk_menu_shell_append(
            GTK_MENU_SHELL(menu->menubar), 
            menu->applications_item
        );

    /* Create Places menu item */
    menu->places_item = gtk_menu_item_new_with_label("Places");
    places_menu       = build_places_menu();
    gtk_menu_item_set_submenu(
            GTK_MENU_ITEM(menu->places_item), 
            places_menu
        );
    gtk_menu_shell_append(
            GTK_MENU_SHELL(menu->menubar), 
            menu->places_item
        );

    /* Create System menu item */
    menu->system_item = gtk_menu_item_new_with_label("System");
    system_menu       = build_system_menu(&menu->garcon_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu->system_item), system_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu->menubar), menu->system_item);

    /* Show all widgets */
    gtk_widget_show_all(menu->menubar);

    /* Connect plugin signals */
    g_signal_connect(
            G_OBJECT(plugin), 
            "free-data",
            G_CALLBACK(classic_menu_free), 
            menu
        );
    g_signal_connect(
            G_OBJECT(plugin), 
            "size-changed",
            G_CALLBACK(classic_menu_size_changed), 
            menu
        );
    g_signal_connect(
            G_OBJECT(plugin), 
            "orientation-changed",
            G_CALLBACK(classic_menu_orientation_changed), 
            menu
        );
}

/* Free plugin resources */
static void
classic_menu_free(XfcePanelPlugin *plugin, ClassicMenuPlugin *menu)
{
    if (menu->garcon_menu != NULL) {
        g_object_unref(menu->garcon_menu);
    }

    g_free(menu);
}

/* Handle size changes */
static gboolean
classic_menu_size_changed(
    XfcePanelPlugin   *plugin, 
    gint               size, 
    ClassicMenuPlugin *menu
)
{
    gtk_widget_set_size_request(GTK_WIDGET(plugin), -1, -1);
    return TRUE;
}

/* Handle orientation changes */
static void
classic_menu_orientation_changed(
    XfcePanelPlugin   *plugin, 
    GtkOrientation     orientation, 
    ClassicMenuPlugin *menu
)
{
    /* Menubar automatically handles orientation */
}

/* Register the plugin */
XFCE_PANEL_PLUGIN_REGISTER(classic_menu_construct);
