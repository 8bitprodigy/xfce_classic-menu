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
        gtk_image_set_pixel_size(GTK_IMAGE(icon), 16);
        gtk_box_pack_start(GTK_BOX(box), icon, FALSE, FALSE, 0);
    }

    label_widget = gtk_label_new(label);
    gtk_label_set_xalign(GTK_LABEL(label_widget), 0.0);
    gtk_box_pack_start(GTK_BOX(box), label_widget, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(menu_item), box);
    gtk_widget_show_all(menu_item);

    return menu_item;
}

/* Open a URI in the file manager */
static void
on_place_activate(GtkMenuItem *item, gpointer user_data)
{
    const gchar *uri = (const gchar *)user_data;
    GError *error = NULL;
    gchar *command;

    command = g_strdup_printf("exo-open --launch FileManager '%s'", uri);

    if (!g_spawn_command_line_async(command, &error)) {
        g_warning(
                "Failed to open location: %s",
                error ? error->message : "Unknown error"
            );
        if (error) {
            g_error_free(error);
        }
    }

    g_free(command);
}

/* Append an item that owns its own copy of the URI string */
static void
append_uri_item(GtkWidget   *menu,
                const gchar *label,
                const gchar *icon_name,
                const gchar *uri)
{
    GtkWidget *item = create_menu_item_with_icon(label, icon_name);
    g_signal_connect_data(
            G_OBJECT(item), "activate",
            G_CALLBACK(on_place_activate),
            g_strdup(uri),
            (GClosureNotify) g_free,
            0
        );
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
}

/* Read ~/.config/gtk-3.0/bookmarks (falling back to ~/.gtk-bookmarks) and
 * append each entry to the menu.  This is the exact same file Thunar reads
 * for its sidebar, so the Places menu will mirror the Thunar sidebar
 * automatically — personal bookmarks and all.
 *
 * Format: one entry per line:  <URI> [optional label]
 * Lines beginning with '#' are comments. */
static gboolean
append_gtk_bookmarks(GtkWidget *menu)
{
    gchar  *path;
    gchar  *contents = NULL;
    gchar **lines;
    guint   i;
    gboolean any = FALSE;

    /* GTK3 location, fall back to legacy */
    path = g_build_filename(
            g_get_user_config_dir(), "gtk-3.0", "bookmarks", NULL
        );
    if (!g_file_get_contents(path, &contents, NULL, NULL)) {
        g_free(path);
        path = g_build_filename(g_get_home_dir(), ".gtk-bookmarks", NULL);
        if (!g_file_get_contents(path, &contents, NULL, NULL)) {
            g_free(path);
            return FALSE;
        }
    }
    g_free(path);

    lines = g_strsplit(contents, "\n", -1);
    g_free(contents);

    for (i = 0; lines[i] != NULL; i++) {
        gchar       *line  = g_strstrip(lines[i]);
        gchar       *uri;
        gchar       *space;
        const gchar *label;
        const gchar *icon_name;
        gchar       *derived_label = NULL;

        if (*line == '\0' || *line == '#') {
            continue;
        }

        /* Split on the first space to separate URI from optional label */
        space = strchr(line, ' ');
        if (space != NULL) {
            *space = '\0';
            label  = (*(space + 1) != '\0') ? space + 1 : NULL;
        } else {
            label = NULL;
        }
        uri = line;

        /* Fall back to basename of the URI as the label */
        if (label == NULL) {
            GFile *file   = g_file_new_for_uri(uri);
            derived_label = g_file_get_basename(file);
            g_object_unref(file);
            label = derived_label;
        }

        icon_name = g_str_has_prefix(uri, "file://") ? "folder"
                                                      : "folder-remote";

        append_uri_item(menu, label, icon_name, uri);
        g_free(derived_label);
        any = TRUE;
    }

    g_strfreev(lines);
    return any;
}

/* Build the Places menu */
GtkWidget *
build_places_menu(void)
{
    GtkWidget   *submenu = gtk_menu_new();
    GtkWidget   *sep;
    const gchar *home_dir = g_get_home_dir();
    gchar       *home_uri;

    gtk_menu_set_reserve_toggle_size(GTK_MENU(submenu), FALSE);

    /* ── Home ─────────────────────────────────────────────────────────── */

    home_uri = g_filename_to_uri(home_dir, NULL, NULL);
    append_uri_item(submenu, "Home Folder", "user-home", home_uri);
    g_free(home_uri);

    /* ── GTK bookmarks — mirrors the Thunar sidebar exactly ───────────── */

    /* The bookmarks file contains the XDG folders (Desktop, Documents,
     * Music, Pictures, Videos, Downloads) as well as any personal
     * bookmarks the user has added in Thunar, in the order they appear
     * in the sidebar.  We read it wholesale so the menu stays in sync
     * with Thunar without any extra configuration. */
    sep = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), sep);

    if (!append_gtk_bookmarks(submenu)) {
        /* Bookmarks file absent or empty — fall back to the common XDG
         * dirs so the menu is never completely bare. */
        gchar *path;
        gchar *uri;

        path = g_build_filename(home_dir, "Desktop",   NULL);
        uri  = g_filename_to_uri(path, NULL, NULL);
        append_uri_item(submenu, "Desktop",   "user-desktop",     uri);
        g_free(uri); g_free(path);

        path = g_build_filename(home_dir, "Documents", NULL);
        uri  = g_filename_to_uri(path, NULL, NULL);
        append_uri_item(submenu, "Documents", "folder-documents",  uri);
        g_free(uri); g_free(path);

        path = g_build_filename(home_dir, "Music",     NULL);
        uri  = g_filename_to_uri(path, NULL, NULL);
        append_uri_item(submenu, "Music",     "folder-music",      uri);
        g_free(uri); g_free(path);

        path = g_build_filename(home_dir, "Pictures",  NULL);
        uri  = g_filename_to_uri(path, NULL, NULL);
        append_uri_item(submenu, "Pictures",  "folder-pictures",   uri);
        g_free(uri); g_free(path);

        path = g_build_filename(home_dir, "Videos",    NULL);
        uri  = g_filename_to_uri(path, NULL, NULL);
        append_uri_item(submenu, "Videos",    "folder-videos",     uri);
        g_free(uri); g_free(path);

        path = g_build_filename(home_dir, "Downloads", NULL);
        uri  = g_filename_to_uri(path, NULL, NULL);
        append_uri_item(submenu, "Downloads", "folder-download",   uri);
        g_free(uri); g_free(path);

        sep = gtk_separator_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(submenu), sep);
    }

    /* ── Computer / Network ───────────────────────────────────────────── */

    append_uri_item(submenu, "Computer", "computer",          "computer://");
    append_uri_item(submenu, "Network",  "network-workgroup", "network://");

    sep = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(submenu), sep);

    append_uri_item(submenu, "Connect to Server...", "network-server", "network://");

    gtk_widget_show_all(submenu);
    return submenu;
}
