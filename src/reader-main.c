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

#include "reader-application.h"

#include <gtk/gtk.h>

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

int
main (int argc, char *argv[])
{
	gint retval;
	ReaderApplication *application;
	
	/*
		TODO
	*/

	/* Initialize gettext support */
	// bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	// bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	// textdomain (GETTEXT_PACKAGE);

	g_set_prgname ("reader");
	g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL);

	application = reader_application_new ();

	retval = g_application_run (G_APPLICATION (application), argc, argv);

	g_object_unref (application);

	return retval;
}
