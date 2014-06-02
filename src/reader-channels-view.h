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

#ifndef __READERCHANNELSVIEW_H
#define __READERCHANNELSVIEW_H

#include "reader-engine.h"

#define READER_CHANNELS_VIEW_TYPE (reader_channels_view_get_type ())
#define READER_CHANNELS_VIEW(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), READER_CHANNELS_VIEW_TYPE, ReaderChannelsView))

typedef struct _ReaderChannelsView         ReaderChannelsView;
typedef struct _ReaderChannelsViewClass    ReaderChannelsViewClass;

GType reader_channels_view_get_type (void);
ReaderChannelsView* reader_channels_view_new ();
void reader_channels_view_set_engine (ReaderChannelsView *view, ReaderEngine *engine);
gboolean reader_channels_view_has_channels (ReaderChannelsView *view);

#endif /* __READERCHANNELSVIEW_H */

