/* classic-menu.h - Common header for classic menu plugin */

#ifndef __CLASSIC_MENU_H__
#define __CLASSIC_MENU_H__

#include <gtk/gtk.h>
#include <garcon/garcon.h>

/* applications-menu.c */
GtkWidget *build_applications_menu(GarconMenu **garcon_menu_ptr);

/* places-menu.c */
GtkWidget *build_places_menu(void);

/* system-menu.c */
GtkWidget *build_system_menu(void);

#endif /* __CLASSIC_MENU_H__ */
