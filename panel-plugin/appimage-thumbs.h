/* appimage-thumbs.h - Async icon loading for Type 2 AppImages */

#ifndef __APPIMAGE_THUMBS_H__
#define __APPIMAGE_THUMBS_H__

#include <gtk/gtk.h>

/* Asynchronously load the best available icon for the AppImage at PATH
 * into IMAGE.
 *
 * The call returns immediately.  It first checks the freedesktop thumbnail
 * cache (~/.cache/thumbnails/normal/).  If a valid cached thumbnail exists
 * it is applied on the next main-loop iteration.  Otherwise the AppImage's
 * embedded .DirIcon is extracted on a background thread, scaled to 16 px,
 * written into the cache, and then applied — also on the main thread via
 * g_idle_add so GTK is never touched from a worker thread.
 *
 * If IMAGE is destroyed before the background work finishes the update is
 * silently discarded. */
void appimage_load_icon_async(GtkImage    *image,
                              const gchar *path);

#endif /* __APPIMAGE_THUMBS_H__ */
