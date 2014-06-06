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

#include <gtk/gtk.h>
#include <libgrss/libgrss.h>

#include "common.h"
#include "reader-application.h"
#include "reader-appwin.h"
#include "reader-engine.h"
#include "reader-topbar.h"
#include "reader-channels-view.h"
#include "reader-items-view.h"
#include "reader-channel-add.h"

struct _ReaderAppWindow
{
	GtkApplicationWindow parent;
};

struct _ReaderAppWindowClass
{
	GtkApplicationWindowClass parent_class;
};

typedef struct _ReaderAppWindowPrivate ReaderAppWindowPrivate;

struct _ReaderAppWindowPrivate
{
	ReaderEngine *engine;
	GtkWidget *stack;
	ReaderTopbar *topbar;
	ReaderChannelsView *channels;
	ReaderItemsView *items;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderAppWindow, reader_app_window, GTK_TYPE_APPLICATION_WINDOW);

static void
test_empty_event (ReaderAppWindow *win)
{
	reader_app_window_change_state (win, READER_STATE_FRONT);
}

/*
	Remember: GdMainView's "item-activated" signal has different signature
	than GtkIconView's namesake signal
*/
static void
on_channel_selection_changed (ReaderChannelsView *channels,
                              gchar *id,
                              GtkTreePath *path,
                              ReaderAppWindow *win)
{
	GtkTreeModel *model;
	ReaderAppWindowPrivate *priv;

	priv = reader_app_window_get_instance_private (win);
	model = reader_engine_get_items_model (priv->engine, id);
	reader_items_view_set_model (priv->items, model);
	reader_app_window_change_state (win, READER_STATE_ITEMVIEW);
}

static void
reader_app_window_finalize (GObject *object)
{
	ReaderAppWindowPrivate *priv;

	priv = reader_app_window_get_instance_private (READER_APP_WINDOW (object));
	g_object_unref (priv->engine);
}

static void
reader_app_window_init (ReaderAppWindow *win)
{
	GtkTreeModel *model;
	ReaderAppWindowPrivate *priv;

	mainWin = win;

	priv = reader_app_window_get_instance_private (win);
	gtk_widget_init_template (GTK_WIDGET (win));
	g_object_set (gtk_settings_get_default (), "gtk-shell-shows-app-menu", FALSE, NULL);
	gtk_application_window_set_show_menubar (GTK_APPLICATION_WINDOW (win), TRUE);

	priv->engine = reader_engine_new ();
	reader_channels_view_set_engine (priv->channels, priv->engine);

	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (priv->stack)), "stack");
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (priv->topbar)), "topbar");
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (priv->channels)), "channels");

	model = reader_engine_get_channels_model (priv->engine);
	g_signal_connect_swapped (model, "row-inserted", G_CALLBACK (test_empty_event), win);
	g_signal_connect_swapped (model, "row-deleted", G_CALLBACK (test_empty_event), win);
}

static void
reader_app_window_dispose (GObject *object)
{
	G_OBJECT_CLASS (reader_app_window_parent_class)->dispose (object);
}

static void
reader_app_window_class_init (ReaderAppWindowClass *class)
{
	G_OBJECT_CLASS (class)->dispose = reader_app_window_dispose;
	G_OBJECT_CLASS (class)->finalize = reader_app_window_finalize;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/gnome/reader/ui/window.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderAppWindow, stack);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderAppWindow, topbar);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderAppWindow, channels);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderAppWindow, items);

	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_channel_selection_changed);
}

ReaderAppWindow*
reader_app_window_new (ReaderApplication *app)
{
	return g_object_new (READER_APP_WINDOW_TYPE, "application", app, NULL);
}

ReaderEngine*
reader_app_window_get_engine (ReaderAppWindow *win)
{
	ReaderAppWindowPrivate *priv;

	priv = reader_app_window_get_instance_private (win);
	return priv->engine;
}

void
reader_app_window_change_state (ReaderAppWindow *win, READER_APP_STATE state)
{
	ReaderAppWindowPrivate *priv;

	priv = reader_app_window_get_instance_private (win);
	switch (state) {
		case READER_STATE_FRONT:
			if (reader_channels_view_has_channels (priv->channels))
				gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "empty");
			else
				gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "channels");

			gd_main_view_set_selection_mode (GD_MAIN_VIEW (priv->channels), FALSE);
			gtk_stack_set_visible_child_name (GTK_STACK (priv->topbar), "front");
			break;

		case READER_STATE_ADD:
			gtk_stack_set_visible_child_name (GTK_STACK (priv->topbar), "addnew");
			gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "addnew");
			break;

		case READER_STATE_SELECT:
			gd_main_view_set_selection_mode (GD_MAIN_VIEW (priv->channels), TRUE);
			gtk_stack_set_visible_child_name (GTK_STACK (priv->topbar), "edit");
			break;

		case READER_STATE_DELETE:
			reader_engine_delete_channels (priv->engine, gd_main_view_get_selection (GD_MAIN_VIEW (priv->channels)));
			reader_app_window_change_state (win, READER_STATE_FRONT);
			break;

		case READER_STATE_ITEMVIEW:
			gtk_stack_set_visible_child_name (GTK_STACK (priv->topbar), "items");
			gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "items");
			break;
	}
}

