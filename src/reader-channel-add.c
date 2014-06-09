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
	GtkStack *stack;
	GtkEntry *urlentry;
	GtkTreeView *entrieslist;
	GtkButton *entriessave;
	gint entriescount;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderChannelAdd, reader_channel_add, GTK_TYPE_BOX);

static void
on_channel_fetched (GObject *source,
                    GAsyncResult *res,
                    ReaderChannelAdd *add)
{
	const gchar *url;
	GtkListStore *model;
	GtkTreeIter iter;
	GrssFeedChannel *channel;
	ReaderChannelAddPrivate *priv;
	GError *error = NULL;

	priv = reader_channel_add_get_instance_private (add);
	model = GTK_LIST_STORE (gtk_tree_view_get_model (priv->entrieslist));

	channel = GRSS_FEED_CHANNEL (source);

	if (grss_feed_channel_fetch_finish (channel, res, &error) == FALSE) {
		g_warning ("Unable to fetch feed: %s", error->message);
		g_error_free (error);
		g_object_unref (channel);
	}
	else {
		url = grss_feed_channel_get_source (channel);

		if (reader_engine_has_channel (reader_app_window_get_engine (mainWin), url) == TRUE) {
			g_message ("Channel already existing");
			g_object_unref (channel);
		}
		else {
			gtk_list_store_append (model, &iter);
			gtk_list_store_set (model, &iter,
				            0, TRUE,
				            1, grss_feed_channel_get_title (channel),
				            2, channel, -1);
		}
	}

	priv->entriescount--;

	if (priv->entriescount <= 0) {
		if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter) == FALSE)
			gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "empty");
		else
			gtk_widget_set_sensitive (GTK_WIDGET (priv->entriessave), TRUE);
	}

}

static void
on_save_by_url (GtkButton *button,
                ReaderChannelAdd *add)
{
	const gchar *url;
	GrssFeedChannel *channel;
	ReaderChannelAddPrivate *priv;

	priv = reader_channel_add_get_instance_private (add);
	url = gtk_entry_get_text (GTK_ENTRY (priv->urlentry));
	if (url == '\0')
		return;

	channel = grss_feed_channel_new_with_source ((gchar*) url);
	if (channel == NULL) {
		g_warning ("Invalid feed channel");
		return;
	}

	grss_feed_channel_fetch_async (channel, (GAsyncReadyCallback) on_channel_fetched, add);
	reader_app_window_change_state (mainWin, READER_STATE_FRONT);
}

static void
on_save_by_file (GtkButton *button,
                 ReaderChannelAdd *add)
{
	gboolean activated;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GrssFeedChannel *channel;
	ReaderChannelAddPrivate *priv;

	priv = reader_channel_add_get_instance_private (add);
	model = gtk_tree_view_get_model (priv->entrieslist);

	if (gtk_tree_model_get_iter_first (model, &iter) == TRUE) {
		do {
			gtk_tree_model_get (model, &iter, 0, &activated, 2, &channel, -1);

			if (activated == TRUE)
				reader_engine_push_channel (reader_app_window_get_engine (mainWin), channel);

			g_object_unref (channel);

		} while (gtk_tree_model_iter_next (model, &iter));

		gtk_list_store_clear (GTK_LIST_STORE (model));
	}

	reader_app_window_change_state (mainWin, READER_STATE_FRONT);
}

static void
on_new_by_url_btn_clicked (GtkButton *button,
                           ReaderChannelAdd *add)
{
	ReaderChannelAddPrivate *priv;

	priv = reader_channel_add_get_instance_private (add);
	gtk_entry_set_text (priv->urlentry, "");
	gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "byurl");
}

static void
on_new_by_file_btn_clicked (GtkButton *button,
                            ReaderChannelAdd *add)
{
	char *filename;
	gint res;
	GList *iter;
	GList *channels;
	GtkWidget *dialog;
	GtkTreeModel *model;
	GrssFeedsGroup *group;
	ReaderChannelAddPrivate *priv;
	GError *error = NULL;

	priv = reader_channel_add_get_instance_private (add);

	dialog = gtk_file_chooser_dialog_new (_("Open File"),
	                                      NULL,
	                                      GTK_FILE_CHOOSER_ACTION_OPEN, _("Cancel"),
	                                      GTK_RESPONSE_CANCEL, _("Open"), GTK_RESPONSE_ACCEPT,
	                                      NULL);

	res = gtk_dialog_run (GTK_DIALOG (dialog));

	if (res == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		group = grss_feeds_group_new ();
		channels = grss_feeds_group_parse_file (group, filename, &error);
		g_free (filename);
		g_object_unref (group);

		if (error != NULL) {
			g_warning ("Unable to read file: %s", error->message);
			g_error_free (error);
			gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "empty");
		}
		else {
			model = gtk_tree_view_get_model (priv->entrieslist);
			if (model == NULL) {
				model = GTK_TREE_MODEL (gtk_list_store_new (3,
					                                    G_TYPE_BOOLEAN,
					                                    G_TYPE_STRING,
					                                    G_TYPE_POINTER));
				gtk_tree_view_set_model (priv->entrieslist, model);
			}

			priv->entriescount = g_list_length (channels);

			if (priv->entriescount > 0) {
				gtk_widget_set_sensitive (GTK_WIDGET (priv->entriessave), FALSE);

				for (iter = channels; iter; iter = iter->next)
					grss_feed_channel_fetch_async (iter->data, (GAsyncReadyCallback) on_channel_fetched, add);

				g_list_free (channels);

				gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "selectchannels");
			}
			else {
				gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "empty");
			}
		}
	}
	else {
		reader_app_window_change_state (mainWin, READER_STATE_FRONT);
	}

	gtk_widget_destroy (dialog);
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
on_switch_channel_include (GtkCellRendererToggle *cell,
                           gchar *path,
                           ReaderChannelAdd *add)
{
	GtkTreePath *p;
	GtkTreeIter iter;
	GtkTreeModel *model;
	ReaderChannelAddPrivate *priv;

	priv = reader_channel_add_get_instance_private (add);
	model = gtk_tree_view_get_model (priv->entrieslist);
	p = gtk_tree_path_new_from_string (path);
	gtk_tree_model_get_iter (model, &iter, p);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, !gtk_cell_renderer_toggle_get_active (cell), -1);
	gtk_tree_path_free (p);
}

static void
reader_channel_add_class_init (ReaderChannelAddClass *class)
{
	G_OBJECT_CLASS (class)->dispose = reader_channel_add_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/gnome/reader/ui/add.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderChannelAdd, stack);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderChannelAdd, urlentry);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderChannelAdd, entrieslist);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderChannelAdd, entriessave);

	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_new_by_url_btn_clicked);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_new_by_file_btn_clicked);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_save_by_url);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_switch_channel_include);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_save_by_file);
}

ReaderChannelAdd*
reader_channel_add_new ()
{
	return g_object_new (READER_CHANNEL_ADD_TYPE, NULL);
}

void
reader_channel_add_reset (ReaderChannelAdd *add)
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	GrssFeedChannel *channel;
	ReaderChannelAddPrivate *priv;

	priv = reader_channel_add_get_instance_private (add);

	model = gtk_tree_view_get_model (priv->entrieslist);

	if (model != NULL && gtk_tree_model_get_iter_first (model, &iter) == TRUE) {
		do {
			gtk_tree_model_get (model, &iter, 2, &channel, -1);
			g_object_unref (channel);
		} while (gtk_tree_model_iter_next (model, &iter));

		gtk_list_store_clear (GTK_LIST_STORE (model));
	}

	gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "start");
}

