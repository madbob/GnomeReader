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

#ifndef __READERENGINE_H
#define __READERENGINE_H

#include <gtk/gtk.h>
#include <libgd/gd-main-view-generic.h>
#include <libgrss/libgrss.h>

#define READER_ENGINE_TYPE (reader_engine_get_type ())
#define READER_ENGINE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), READER_ENGINE_TYPE, ReaderEngine))

typedef struct _ReaderEngine         ReaderEngine;
typedef struct _ReaderEngineClass    ReaderEngineClass;

enum {
	EXTRA_COLUMN_UNREADS = GD_MAIN_COLUMN_LAST,
	EXTRA_COLUMN_FEEDS_MODEL,
	EXTRA_COLUMN_LAST
} CHANNELS_MODEL_COLUMNS;

enum {
	ITEM_COLUMN_ID,
	ITEM_COLUMN_TITLE,
	ITEM_COLUMN_URL,
	ITEM_COLUMN_CONTENTS,
	ITEM_COLUMN_TIME,
	ITEM_COLUMN_READ,
	ITEM_COLUMN_BG,
	ITEM_COLUMN_LAST
} ITEMS_MODEL_COLUMNS;

GType reader_engine_get_type (void);
ReaderEngine* reader_engine_new ();
GtkTreeModel* reader_engine_get_channels_model (ReaderEngine *engine);
GtkTreeModel* reader_engine_get_items_model (ReaderEngine *engine, const gchar *channel);
void reader_engine_push_channel (ReaderEngine *engine, GrssFeedChannel *channel);

#endif /* __READERENGINE_H */

