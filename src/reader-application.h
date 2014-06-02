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

#ifndef __READER_APPLICATION_H__
#define __READER_APPLICATION_H__

#include <gdk/gdk.h>
#include <gio/gio.h>
#include <gtk/gtk.h>

#define READER_DESKTOP_ICON_VIEW_IID	"OAFIID:Reader_File_Manager_Desktop_Canvas_View"

#define READER_TYPE_APPLICATION reader_application_get_type()
#define READER_APPLICATION(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), READER_TYPE_APPLICATION, ReaderApplication))
#define READER_APPLICATION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), READER_TYPE_APPLICATION, ReaderApplicationClass))
#define READER_IS_APPLICATION(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), READER_TYPE_APPLICATION))
#define READER_IS_APPLICATION_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), READER_TYPE_APPLICATION))
#define READER_APPLICATION_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), READER_TYPE_APPLICATION, ReaderApplicationClass))

typedef struct _ReaderApplication ReaderApplication;
typedef struct _ReaderApplicationClass ReaderApplicationClass;
typedef struct _ReaderApplicationPriv ReaderApplicationPriv;

GType reader_application_get_type (void);

ReaderApplication* reader_application_new (void);

#endif /* __READER_APPLICATION_H__ */

