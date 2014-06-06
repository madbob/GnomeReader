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
#include "reader-topbar.h"
#include "reader-appwin.h"

struct _ReaderTopbar
{
	GtkStack parent;
};

struct _ReaderTopbarClass
{
	GtkStackClass parent_class;
};

G_DEFINE_TYPE(ReaderTopbar, reader_topbar, GTK_TYPE_STACK);

static void
on_new_btn_clicked (GtkButton *button,
                    ReaderTopbar *bar)
{
	reader_app_window_change_state (mainWin, READER_STATE_ADD);
}

static void
on_select_btn_clicked (GtkButton *button,
                       ReaderTopbar *bar)
{
	reader_app_window_change_state (mainWin, READER_STATE_SELECT);
}

static void
on_delete_btn_clicked (GtkButton *button,
                       ReaderTopbar *bar)
{
	reader_app_window_change_state (mainWin, READER_STATE_DELETE);
}

static void
on_back_btn_clicked (GtkButton *button,
                     ReaderTopbar *bar)
{
	reader_app_window_change_state (mainWin, READER_STATE_FRONT);
}

static void
reader_topbar_init (ReaderTopbar *bar)
{
	gtk_widget_init_template (GTK_WIDGET (bar));
}

static void
reader_topbar_dispose (GObject *object)
{
	G_OBJECT_CLASS (reader_topbar_parent_class)->dispose (object);
}

static void
reader_topbar_class_init (ReaderTopbarClass *class)
{
	G_OBJECT_CLASS (class)->dispose = reader_topbar_dispose;

	gtk_widget_class_set_template_from_resource (GTK_WIDGET_CLASS (class), "/org/gnome/reader/ui/topbar.ui");

	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_new_btn_clicked);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_select_btn_clicked);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_delete_btn_clicked);
	gtk_widget_class_bind_template_callback (GTK_WIDGET_CLASS (class), on_back_btn_clicked);
}

ReaderTopbar*
reader_topbar_new ()
{
	return g_object_new (READER_TOPBAR_TYPE, NULL);
}

