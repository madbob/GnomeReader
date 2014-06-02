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
#include "reader-application.h"
#include "reader-appwin.h"

struct _ReaderApplication
{
	GtkApplication parent;
};

struct _ReaderApplicationClass
{
	GtkApplicationClass parent_class;
};

G_DEFINE_TYPE(ReaderApplication, reader_application, GTK_TYPE_APPLICATION);

static void
reader_application_init (ReaderApplication *app)
{
}

static void
quit_activated (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       app)
{
	g_application_quit (G_APPLICATION (app));
}

static GActionEntry app_entries[] =
{
	{ "quit", quit_activated, NULL, NULL, NULL }
};

static void
reader_application_startup (GApplication *app)
{
	const gchar *css;
	GBytes *bytes;
	GtkCssProvider *provider;
	const gchar *quit_accels[2] = { "<Ctrl>Q", NULL };

	G_APPLICATION_CLASS (reader_application_parent_class)->startup (app);

	g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app);
	gtk_application_set_accels_for_action (GTK_APPLICATION (app), "app.quit", quit_accels);

	bytes = g_resources_lookup_data ("/org/gnome/reader/ui/reader.css", 0, NULL);
	css = g_bytes_get_data (bytes, NULL);
	provider = gtk_css_provider_new ();
	gtk_style_context_add_provider_for_screen (gdk_screen_get_default (), GTK_STYLE_PROVIDER (provider), G_MAXUINT);
	gtk_css_provider_load_from_data (provider, css, -1, NULL);
	g_object_unref (provider);
	g_bytes_unref (bytes);
}

static void
reader_application_activate (GApplication *app)
{
	ReaderAppWindow *win;

	win = reader_app_window_new (READER_APPLICATION (app));
	gtk_window_present (GTK_WINDOW (win));
}

static void
reader_application_class_init (ReaderApplicationClass *class)
{
	G_APPLICATION_CLASS (class)->startup = reader_application_startup;
	G_APPLICATION_CLASS (class)->activate = reader_application_activate;
}

ReaderApplication *
reader_application_new (void)
{
	return g_object_new (READER_TYPE_APPLICATION,
			       "application-id", "org.madbob.gnome",
			       NULL);
}

