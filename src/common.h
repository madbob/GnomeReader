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

#ifndef __READERCOMMON_H
#define __READERCOMMON_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libgd/gd.h>

#define TITLE_FONT_WEIGHT		800
#define UNREAD_FONT_WEIGHT		600
#define READ_FONT_WEIGHT		400

#define ICON_SIZE			50

#include "reader-appwin.h"
ReaderAppWindow *mainWin;

#endif /* __READERCOMMON_H */

