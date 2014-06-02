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

#ifndef __READERTOPBAR_H
#define __READERTOPBAR_H

#include <gtk/gtk.h>

#define READER_TOPBAR_TYPE (reader_topbar_get_type ())
#define READER_TOPBAR(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), READER_TOPBAR_TYPE, ReaderTopbar))

typedef struct _ReaderTopbar         ReaderTopbar;
typedef struct _ReaderTopbarClass    ReaderTopbarClass;

GType reader_topbar_get_type (void);
ReaderTopbar* reader_topbar_new ();

#endif /* __READERTOPBAR_H */

