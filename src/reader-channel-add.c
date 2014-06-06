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
#include "reader-channel-add.h"
#include "reader-appwin.h"

struct _ReaderChannelAdd
{
	GtkBox parent;
};

struct _ReaderChannelAddClass
{
	GtkBoxClass parent_class;
};

typedef struct _ReaderChannelAddPrivate ReaderChannelAddPrivate;

struct _ReaderChannelAddPrivate
{
	GtkWidget *addnewentry;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderChannelAdd, reader_channel_add, GTK_TYPE_BOX);

static void
on_channel_fetched (GObject *source,
                    GAsyncResult *res,
                    ReaderChannelAdd *add)
{
	GrssFeedChannel *channel;
	GError *error = NULL;

	channel = GRSS_FEED_CHANNEL (source);

	if (grss_feed_channel_fetch_finish (channel, res, &error) == FALSE) {
		g_warning ("Unable to fetch feed: %s", error->message);
		g_error_free (error);
	}
	else {
		reader_engine_push_channel (reader_app_window_get_engine (mainWin), channel);
	}

	g_object_unref (channel);
}

static void
on_save_new_feed (GtkButton *button,
                  ReaderChannelAdd *add)
{
	const gchar *url;
	GrssFeedChannel *channel;
	ReaderChannelAddPrivate *priv;

	priv = reader_channel_add_get_instance_private (add);
	url = gtk_entry_get_text (GTK_ENTRY (priv->addnewentry));
	if (url == '\0')
		return;

	/*
		TODO	Validate the URL
	*/

	channel = grss_feed_channel_new_with_source ((gchar*) url);
	grss_feed_channel_fetch_async (channel, (GAsyncReadyCallback) on_channel_fetched, add);

	reader_app_window_change_state (mainWin, READER_STATE_FRONT);
}

static void
on_save_new_opml (GtkFileChooserButton *widget,
                  ReaderChannelAdd *add)
{
	GList *iter;
	GList *channels;
	GrssFeedsGroup *group;
	GError *error = NULL;

	group = grss_feeds_group_new ();
	channels = grss_feeds_group_parse_file (group, gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (widget)), &error);
	g_object_unref (group);

	if (error != NULL) {
		g_warning ("Unable to read file: %s", error->message);
		g_error_free (error);
		return;
	}

	for (iter = channels; iter; iter = iter->next)
		grss_feed_channel_fetch_async (iter->data, (GAsyncReadyCallback) on_channel_fetched, add);

	g_list_free (channels);

	reader_app_window_change_state (mainWin, READER_STATE_FRONT);
}

static void
on_cancel_new_feed (GtkButton *button,
                    ReaderChannelAdd *add)
{
	reader_app_window_change_state (mainWin, READER_STATE_FRONT);
}

static void
reader_channel_add_init (ReaderChannelAdd *add)
{
	gtk_widget_init_template (GTK_WIDGET (add));
}

static void
reader_channel_add_dispose (GObject *object)
{
	G_OBJECT_CLASS (reader_channel_add_parent_class)->dispose (object);
}

static void
reader_channel_add_class_init (ReaderChannelAddClass *class)
{
	G_OBJECT_CLASS (class)->dispose = reader_channel_add_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/gnome/reader/ui/add.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderChannelAdd, addnewentry);

	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_save_new_feed);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_save_new_opml);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_cancel_new_feed);
}

ReaderChannelAdd*
reader_channel_add_new ()
{
	return g_object_new (READER_CHANNEL_ADD_TYPE, NULL);
}

