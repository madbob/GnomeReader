/*
 * Reader
 *
 * Copyright (C) 2014 Roberto Guido <rguido@src.gnome.org>
 *
 * Reader is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * Reader is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __READERAPPWIN_H
#define __READERAPPWIN_H

#include <gtk/gtk.h>
#include "reader-application.h"
#include "reader-engine.h"

#define READER_APP_WINDOW_TYPE (reader_app_window_get_type ())
#define READER_APP_WINDOW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), READER_APP_WINDOW_TYPE, ReaderAppWindow))

typedef struct _ReaderAppWindow         ReaderAppWindow;
typedef struct _ReaderAppWindowClass    ReaderAppWindowClass;

typedef enum {
	READER_STATE_FRONT,
	READER_STATE_ADD,
	READER_STATE_SELECT,
	READER_STATE_DELETE,
	READER_STATE_ITEMVIEW
} READER_APP_STATE;

GType reader_app_window_get_type (void);
ReaderAppWindow* reader_app_window_new (ReaderApplication *app);

ReaderEngine* reader_app_window_get_engine (ReaderAppWindow *win);
void reader_app_window_change_state (ReaderAppWindow *win, READER_APP_STATE state);

#endif /* __READERAPPWIN_H */

