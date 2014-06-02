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

#ifndef __READERITEMSVIEW_H
#define __READERITEMSVIEW_H

#define READER_ITEMS_VIEW_TYPE (reader_items_view_get_type ())
#define READER_ITEMS_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), READER_ITEMS_VIEW_TYPE, ReaderItemsView))

typedef struct _ReaderItemsView         ReaderItemsView;
typedef struct _ReaderItemsViewClass    ReaderItemsViewClass;

GType reader_items_view_get_type (void);
ReaderItemsView* reader_items_view_new ();

void reader_items_view_set_model (ReaderItemsView *view, GtkTreeModel *model);

#endif /* __READERITEMSVIEW_H */

