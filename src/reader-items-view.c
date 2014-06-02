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

#include <webkit2/webkit2.h>

#include "common.h"
#include "reader-items-view.h"
#include "reader-engine.h"

struct _ReaderItemsView
{
	GtkBox parent;
};

struct _ReaderItemsViewClass
{
	GtkBoxClass parent_class;
};

typedef struct _ReaderItemsViewPrivate ReaderItemsViewPrivate;

struct _ReaderItemsViewPrivate
{
	GtkStack *stack;
	GtkTreeView *items;
	GtkTreeSelection *selection;

	GtkStack *webstack;
	GtkLinkButton *title;
	GtkLabel *date;
	WebKitWebView *webview;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderItemsView, reader_items_view, GTK_TYPE_BOX);

static void
on_item_selection_changed (GtkTreeSelection *selection,
                           ReaderItemsView *view)
{
	int read;
	gchar *url;
	gchar *title;
	gchar *contents;
	gchar *str_date;
	GtkTreeIter iter;
	GtkTreePath *path;
	GtkTreeModel *model;
	GDateTime *date;
	ReaderItemsViewPrivate *priv;

	priv = reader_items_view_get_instance_private (view);

	if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
		path = gtk_tree_model_get_path (model, &iter);

		if (gtk_tree_path_get_depth (path) != 1) {
			gtk_tree_model_get (model, &iter,
						ITEM_COLUMN_CONTENTS, &contents,
						ITEM_COLUMN_TITLE, &title,
						ITEM_COLUMN_TIME, &date,
						ITEM_COLUMN_URL, &url,
						ITEM_COLUMN_READ, &read, -1);

			gtk_stack_set_visible_child_name (GTK_STACK (priv->webstack), "webcontents");

			gtk_button_set_label (GTK_BUTTON (priv->title), title);
			gtk_link_button_set_uri (priv->title, url);

			str_date = g_date_time_format (date, "%c");
			gtk_label_set_text (priv->date, str_date);
			g_free (str_date);

			webkit_web_view_load_html (priv->webview, contents, NULL);

			g_free (title);
			g_free (url);
			g_free (contents);

			if (read == UNREAD_FONT_WEIGHT)
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, ITEM_COLUMN_READ, READ_FONT_WEIGHT, -1);
		}

		gtk_tree_path_free (path);
	}
}

static void
reader_items_view_init (ReaderItemsView *view)
{
	ReaderItemsViewPrivate *priv;

	priv = reader_items_view_get_instance_private (view);
	gtk_widget_init_template (GTK_WIDGET (view));

	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (priv->stack)), "stack");
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (priv->stack)), "webstack");
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (priv->title)), "title");
	gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (priv->date)), "date");
}

static void
reader_items_view_dispose (GObject *object)
{
	G_OBJECT_CLASS (reader_items_view_parent_class)->dispose (object);
}

static void
reader_items_view_class_init (ReaderItemsViewClass *class)
{
	G_OBJECT_CLASS (class)->dispose = reader_items_view_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/gnome/reader/ui/items.ui");

	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderItemsView, stack);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderItemsView, webstack);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderItemsView, items);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderItemsView, selection);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderItemsView, title);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderItemsView, date);
	gtk_widget_class_bind_template_child_private (GTK_WIDGET_CLASS (class), ReaderItemsView, webview);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_item_selection_changed);
}

ReaderItemsView*
reader_items_view_new ()
{
	return g_object_new (READER_ITEMS_VIEW_TYPE, NULL);
}

void
reader_items_view_set_model (ReaderItemsView *view,
                             GtkTreeModel *model)
{
	GtkTreeIter useless;
	ReaderItemsViewPrivate *priv;

	priv = reader_items_view_get_instance_private (view);

	if (gtk_tree_model_get_iter_first (model, &useless) == FALSE) {
		gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "empty");
	}
	else {
		gtk_stack_set_visible_child_name (GTK_STACK (priv->stack), "contents");
		gtk_stack_set_visible_child_name (GTK_STACK (priv->webstack), "webempty");

		gtk_tree_view_set_model (priv->items, model);
		gtk_tree_view_expand_all (priv->items);
		gtk_tree_selection_unselect_all (priv->selection);
	}
}

