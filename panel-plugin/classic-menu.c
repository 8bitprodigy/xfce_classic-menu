/* classic-menu.c - Main plugin file */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4util/libxfce4util.h>
#include <garcon/garcon.h>

#include "classic-menu.h"

/* Plugin structure */
typedef struct {
    XfcePanelPlugin  *plugin;

    GtkWidget        *menubar;
    GtkWidget        *applications_item;
    GtkWidget        *places_item;
    GtkWidget        *system_item;

    GarconMenu       *garcon_menu;
    ClassicMenuConfig config;
}
ClassicMenuPlugin;

/* Config key names */
#define CONFIG_GROUP        "classic-menu"
#define KEY_DRILLDOWN_MODE  "drilldown-mode"

/* Forward declarations */
static void     classic_menu_construct(          XfcePanelPlugin *plugin);
static void     classic_menu_free(               XfcePanelPlugin *plugin, ClassicMenuPlugin *menu);
static gboolean classic_menu_size_changed(       XfcePanelPlugin *plugin, gint               size,        ClassicMenuPlugin *menu);
static void     classic_menu_orientation_changed(XfcePanelPlugin *plugin, GtkOrientation     orientation, ClassicMenuPlugin *menu);
static void     classic_menu_configure(          XfcePanelPlugin *plugin,                                 ClassicMenuPlugin *menu);

/* ── Config ─────────────────────────────────────────────────────────────── */

static void
classic_menu_config_load(ClassicMenuPlugin *menu)
{
    XfceRc *rc;
    gchar  *path;

    /* Sensible defaults */
    menu->config.drilldown_mode = DRILLDOWN_FOLDERS_ONLY;

    path = xfce_panel_plugin_lookup_rc_file(menu->plugin);
    if (path == NULL) {
        return;
    }

    rc = xfce_rc_simple_open(path, TRUE);
    g_free(path);
    if (rc == NULL) {
        return;
    }

    xfce_rc_set_group(rc, CONFIG_GROUP);
    menu->config.drilldown_mode = (DrillDownMode)
        xfce_rc_read_int_entry(rc, KEY_DRILLDOWN_MODE, DRILLDOWN_FOLDERS_ONLY);

    xfce_rc_close(rc);
}

static void
classic_menu_config_save(ClassicMenuPlugin *menu)
{
    XfceRc *rc;
    gchar  *path;

    path = xfce_panel_plugin_save_location(menu->plugin, TRUE);
    if (path == NULL) {
        return;
    }

    rc = xfce_rc_simple_open(path, FALSE);
    g_free(path);
    if (rc == NULL) {
        return;
    }

    xfce_rc_set_group(rc, CONFIG_GROUP);
    xfce_rc_write_int_entry(rc, KEY_DRILLDOWN_MODE,
                            (gint) menu->config.drilldown_mode);

    xfce_rc_close(rc);
}

/* ── Properties dialog ──────────────────────────────────────────────────── */

static void
on_drilldown_changed(GtkComboBox *combo, gpointer user_data)
{
    ClassicMenuPlugin *menu = (ClassicMenuPlugin *)user_data;

    menu->config.drilldown_mode =
        (DrillDownMode) gtk_combo_box_get_active(combo);

    classic_menu_config_save(menu);

    /* Rebuild the Places menu so the change takes effect immediately */
    {
        GtkWidget *places_menu = build_places_menu(&menu->config);
        gtk_menu_item_set_submenu(
                GTK_MENU_ITEM(menu->places_item),
                places_menu
            );
    }
}

static void
classic_menu_configure(XfcePanelPlugin *plugin, ClassicMenuPlugin *menu)
{
    GtkWidget *dialog;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *combo;

    xfce_panel_plugin_block_menu(plugin);

    dialog = xfce_titled_dialog_new_with_mixed_buttons(
            "Classic Menu Preferences",
            NULL,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            "window-close-symbolic", "Close", GTK_RESPONSE_OK,
            NULL
        );

    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 12);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 6);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 12);
    gtk_box_pack_start(
            GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
            grid, TRUE, TRUE, 0
        );

    label = gtk_label_new("Folder drill-down shows:");
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(
            GTK_COMBO_BOX_TEXT(combo), "None"
        );
    gtk_combo_box_text_append_text(
            GTK_COMBO_BOX_TEXT(combo), "Folders only"
        );
    gtk_combo_box_text_append_text(
            GTK_COMBO_BOX_TEXT(combo), "Folders and AppImages"
        );
    gtk_combo_box_text_append_text(
            GTK_COMBO_BOX_TEXT(combo), "All contents"
        );
    gtk_combo_box_set_active(
            GTK_COMBO_BOX(combo), (gint) menu->config.drilldown_mode
        );
    gtk_grid_attach(GTK_GRID(grid), combo, 1, 0, 1, 1);

    g_signal_connect(
            G_OBJECT(combo), "changed",
            G_CALLBACK(on_drilldown_changed),
            menu
        );

    g_signal_connect_swapped(
            G_OBJECT(dialog), "response",
            G_CALLBACK(gtk_widget_destroy),
            dialog
        );
    g_signal_connect_swapped(
            G_OBJECT(dialog), "destroy",
            G_CALLBACK(xfce_panel_plugin_unblock_menu),
            plugin
        );

    gtk_widget_show_all(dialog);
}

/* ── Plugin construction ────────────────────────────────────────────────── */

static void
on_places_item_activate(GtkMenuItem *item, gpointer user_data)
{
    GtkWidget *places_menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(item));
    if (places_menu != NULL) {
        g_object_set_data(G_OBJECT(places_menu), "activated-time",
                          GUINT_TO_POINTER(gtk_get_current_event_time()));
    }
}

static void
classic_menu_construct(XfcePanelPlugin *plugin)
{
    ClassicMenuPlugin *menu;
    GtkWidget         *applications_menu;
    GtkWidget         *places_menu;
    GtkWidget         *system_menu;

    menu         = g_new0(ClassicMenuPlugin, 1);
    menu->plugin = plugin;

    classic_menu_config_load(menu);

    /* Create menubar */
    menu->menubar = gtk_menu_bar_new();
    gtk_container_add(GTK_CONTAINER(plugin), menu->menubar);
    xfce_panel_plugin_add_action_widget(plugin, menu->menubar);

    /* Applications */
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

    /* Places */
    menu->places_item = gtk_menu_item_new_with_label("Places");
    places_menu       = build_places_menu(&menu->config);
    gtk_menu_item_set_submenu(
            GTK_MENU_ITEM(menu->places_item),
            places_menu
        );
    g_signal_connect(G_OBJECT(menu->places_item), "activate",
                     G_CALLBACK(on_places_item_activate), NULL);
    gtk_menu_shell_append(
            GTK_MENU_SHELL(menu->menubar),
            menu->places_item
        );

    /* System */
    menu->system_item = gtk_menu_item_new_with_label("System");
    system_menu       = build_system_menu(&menu->garcon_menu);
    gtk_menu_item_set_submenu(
            GTK_MENU_ITEM(menu->system_item),
            system_menu
        );
    gtk_menu_shell_append(
            GTK_MENU_SHELL(menu->menubar),
            menu->system_item
        );

    gtk_widget_show_all(menu->menubar);

    /* Plugin signals */
    g_signal_connect(
            G_OBJECT(plugin), "free-data",
            G_CALLBACK(classic_menu_free), menu
        );
    g_signal_connect(
            G_OBJECT(plugin), "size-changed",
            G_CALLBACK(classic_menu_size_changed), menu
        );
    g_signal_connect(
            G_OBJECT(plugin), "orientation-changed",
            G_CALLBACK(classic_menu_orientation_changed), menu
        );

    /* Show the Configure entry in the right-click plugin menu */
    xfce_panel_plugin_menu_show_configure(plugin);
    g_signal_connect(
            G_OBJECT(plugin), "configure-plugin",
            G_CALLBACK(classic_menu_configure), menu
        );
}

/* ── Boilerplate ────────────────────────────────────────────────────────── */

static void
classic_menu_free(XfcePanelPlugin *plugin, ClassicMenuPlugin *menu)
{
    if (menu->garcon_menu != NULL) {
        g_object_unref(menu->garcon_menu);
    }

    g_free(menu);
}

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
