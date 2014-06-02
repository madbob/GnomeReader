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

#include "common.h"
#include "reader-channels-view.h"
#include "reader-engine.h"

struct _ReaderChannelsView
{
	GdMainView parent;
};

struct _ReaderChannelsViewClass
{
	GdMainViewClass parent_class;
};

typedef struct _ReaderChannelsViewPrivate ReaderChannelsViewPrivate;

struct _ReaderChannelsViewPrivate
{
	ReaderEngine *engine;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderChannelsView, reader_channels_view, GD_TYPE_MAIN_VIEW);

static void
reader_channels_view_init (ReaderChannelsView *win)
{
	gd_main_view_set_view_type (GD_MAIN_VIEW (win), GD_MAIN_VIEW_ICON);
}

static void
reader_channels_view_class_init (ReaderChannelsViewClass *class)
{
}

ReaderChannelsView*
reader_channels_view_new ()
{
	return g_object_new (READER_CHANNELS_VIEW_TYPE, NULL);
}

void
reader_channels_view_set_engine (ReaderChannelsView *view, ReaderEngine *engine)
{
	ReaderChannelsViewPrivate *priv;

	priv = reader_channels_view_get_instance_private (view);
	priv->engine = engine;
	gd_main_view_set_model (GD_MAIN_VIEW (view), reader_engine_get_channels_model (priv->engine));
}

gboolean
reader_channels_view_has_channels (ReaderChannelsView *view)
{
	GtkTreeModel *model;
	GtkTreeIter useless;

	model = gd_main_view_get_model (GD_MAIN_VIEW (view));
	return (gtk_tree_model_get_iter_first (model, &useless) == FALSE);
}

