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

#include <libtracker-sparql/tracker-sparql.h>

#include "common.h"
#include "reader-engine.h"

#define FEED_CHANNEL_TRACKER_CLASS	"http://www.tracker-project.org/temp/mfo#FeedChannel"
#define FEED_MESSAGE_TRACKER_CLASS	"http://www.tracker-project.org/temp/mfo#FeedMessage"

struct _ReaderEngine
{
	GObject parent;
};

struct _ReaderEngineClass
{
	GObjectClass parent_class;
};

typedef struct _ReaderEnginePrivate ReaderEnginePrivate;

struct _ReaderEnginePrivate
{
	GDBusConnection *connection;
	TrackerSparqlConnection *tracker;
	GtkListStore *data;
};

G_DEFINE_TYPE_WITH_PRIVATE(ReaderEngine, reader_engine, G_TYPE_OBJECT);

static void
verify_tracker_update (GObject *source,
                       GAsyncResult *res,
                       gpointer unused)
{
	GError *error = NULL;

	tracker_sparql_connection_query_finish (TRACKER_SPARQL_CONNECTION (source), res, &error);

	if (error != NULL) {
		g_warning ("Unable to update Tracker: %s", error->message);
		g_error_free (error);
	}
}

static void
model_item_changed (GtkTreeModel *tree_model,
                    GtkTreePath *path,
                    GtkTreeIter *iter,
                    ReaderEngine *engine)
{
	int read;
	gboolean r;
	gchar *id;
	gchar *query;
	ReaderEnginePrivate *priv;

	if (gtk_tree_path_get_depth (path) == 1)
		return;

	gtk_tree_model_get (tree_model, iter, ITEM_COLUMN_ID, &id, ITEM_COLUMN_READ, &read, -1);
	r = (read == READ_FONT_WEIGHT ? TRUE : FALSE);

	priv = reader_engine_get_instance_private (engine);
	query = g_strdup_printf ("DELETE {<%s> nmo:isRead %s} "
	                         "WHERE {<%s> nmo:isRead %s} "
	                         "INSERT {<%s> nmo:isRead %s}",
	                         id, r ? "false" : "true", id, r ? "false" : "true", id, r ? "true" : "false");

	tracker_sparql_connection_update_async (priv->tracker, query, 0, NULL,
	                                        (GAsyncReadyCallback) verify_tracker_update, NULL);
	g_free (id);
	g_free (query);
}

static gboolean
find_in_model (GtkTreeModel *model,
               int column,
               const gchar *id,
               GtkTreeIter **target)
{
	gchar *value;
	gboolean done;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_first (model, &iter) == FALSE)
		return FALSE;

	done = FALSE;

	do {
		gtk_tree_model_get (model, &iter, column, &value, -1);

		if (strcmp (value, id) == 0)
			done = TRUE;

		g_free (value);

	} while (done == FALSE && gtk_tree_model_iter_next (model, &iter));

	if (done == TRUE && target != NULL)
		*target = gtk_tree_iter_copy (&iter);

	return done;
}

static GDateTime*
item_cursor_to_time (TrackerSparqlCursor *cursor, gint index)
{
	const gchar *date;
	GTimeVal val;

	date = tracker_sparql_cursor_get_string (cursor, index, NULL);
	g_time_val_from_iso8601 (date, &val);
	return g_date_time_new_from_timeval_local (&val);
}

static gint
date_time_day_difference (GDateTime *first,
                          GDateTime *second)
{
	gint dfirst;
	gint dsecond;

	dfirst = g_date_time_get_day_of_year (first);
	dsecond = g_date_time_get_day_of_year (second);

	if (dfirst == dsecond)
		return 0;
	else
		return g_date_time_compare (first, second);
}

static gboolean
dispose_item_in_model (GtkTreeModel *model,
                       const gchar *item_id,
                       GDateTime *date,
                       GtkTreeIter *iter)
{
	gint index;
	gint diff;
	gboolean found;
	gchar *date_str;
	gchar *test_id;
	GDateTime *test_date;
	GtkTreeStore *tstore;
	GtkTreeIter parent;

	found = FALSE;
	index = 0;
	tstore = GTK_TREE_STORE (model);

	if (gtk_tree_model_get_iter_first (model, &parent) == TRUE) {
		do {
			gtk_tree_model_get (model, &parent, ITEM_COLUMN_TIME, &test_date, -1);
			diff = date_time_day_difference (date, test_date);

			if (diff > 0) {
				break;
			}
			else if (diff == 0) {
				found = TRUE;
				break;
			}

			index++;

		} while (gtk_tree_model_iter_next (model, &parent));
	}

	if (found == FALSE) {
		date_str = g_date_time_format (date, "%x");
		date = g_date_time_ref (date);

		gtk_tree_store_insert (tstore, &parent, NULL, index);
		gtk_tree_store_set (tstore, &parent,
		                    ITEM_COLUMN_TITLE, date_str,
		                    ITEM_COLUMN_TIME, date,
		                    ITEM_COLUMN_READ, TITLE_FONT_WEIGHT,
		                    ITEM_COLUMN_BG, "gray", -1);
		gtk_tree_store_prepend (tstore, iter, &parent);

		g_free (date_str);
	}
	else {
		found = FALSE;

		if (gtk_tree_model_iter_children (model, iter, &parent) == TRUE) {
			do {
				gtk_tree_model_get (model, iter, ITEM_COLUMN_ID, &test_id, -1);

				if (strcmp (test_id, item_id) == 0) {
					found = TRUE;
					break;
				}

				g_free (test_id);

			} while (gtk_tree_model_iter_next (model, iter));
		}

		if (found == FALSE)
			gtk_tree_store_prepend (tstore, iter, &parent);
	}

	return !found;
}

static void
pass_items (ReaderEngine *engine,
            TrackerSparqlCursor *cursor)
{
	gboolean blocked;
	const gchar *channel_id;
	const gchar *item_id;
	GtkTreeIter iter;
	GtkTreeModel *model;
	GDateTime *date;

	if (cursor != NULL) {
		while (tracker_sparql_cursor_next (cursor, NULL, NULL)) {
			channel_id = tracker_sparql_cursor_get_string (cursor, 0, NULL);
			model = reader_engine_get_items_model (engine, channel_id);
			if (model == NULL)
				continue;

			item_id = tracker_sparql_cursor_get_string (cursor, 1, NULL);
			date = item_cursor_to_time (cursor, 6);

			if (dispose_item_in_model (model, item_id, date, &iter) == FALSE) {
				g_signal_handlers_block_by_func (model, G_CALLBACK (model_item_changed), engine);
				blocked = TRUE;
			}
			else {
				blocked = FALSE;
			}

			gtk_tree_store_set (GTK_TREE_STORE (model), &iter,
			                    ITEM_COLUMN_ID, item_id,
			                    ITEM_COLUMN_TITLE, tracker_sparql_cursor_get_string (cursor, 2, NULL),
			                    ITEM_COLUMN_URL, tracker_sparql_cursor_get_string (cursor, 3, NULL),
			                    ITEM_COLUMN_TIME, date,
			                    ITEM_COLUMN_CONTENTS, tracker_sparql_cursor_get_string (cursor, 5, NULL),
			                    ITEM_COLUMN_READ, tracker_sparql_cursor_get_boolean (cursor, 4) ? READ_FONT_WEIGHT : UNREAD_FONT_WEIGHT, -1);

			if (blocked == TRUE)
				g_signal_handlers_unblock_by_func (model, G_CALLBACK (model_item_changed), engine);
		}

		g_object_unref (cursor);
	}
}

static void
on_fetch_items (GObject *source,
                GAsyncResult *res,
                ReaderEngine *engine)
{
	TrackerSparqlCursor *cursor;
	GError *error = NULL;

	cursor = tracker_sparql_connection_query_finish (TRACKER_SPARQL_CONNECTION (source), res, &error);

	if (error != NULL) {
		g_warning ("Unable to fetch feeds from Tracker: %s", error->message);
		g_error_free (error);
	}
	else {
		pass_items (engine, cursor);
	}
}

static void
collect_items (ReaderEngine *engine,
               const gchar *channel_id,
               const gchar *item_id)
{
	gchar *query;
	gchar *cid;
	gchar *iid;
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);

	if (channel_id == NULL)
		cid = g_strdup ("?c");
	else
		cid = g_strdup_printf ("<%s>", channel_id);

	if (item_id == NULL)
		iid = g_strdup ("?s");
	else
		iid = g_strdup_printf ("<%s>", item_id);

	query = g_strdup_printf ("SELECT %s %s ?t ?l ?r ?c ?d "
		                 "WHERE {%s a mfo:FeedMessage; "
		                         "nmo:communicationChannel %s; "
		                         "nie:title ?t; "
		                         "nie:url ?l; "
		                         "nmo:isRead ?r; "
		                         "nie:plainTextContent ?c; "
		                         "nie:contentCreated ?d} "
	                         "ORDER BY DESC(?d) LIMIT 100", cid, iid, iid, cid);

	tracker_sparql_connection_query_async (priv->tracker, query, NULL, (GAsyncReadyCallback) on_fetch_items, engine);

	g_free (query);
	g_free (cid);
	g_free (iid);
}

static void
handle_feed_inserts (ReaderEngine *engine,
                     gint sub,
                     gint obj)
{
	gchar *query;
	const gchar *subject;
	const gchar *object;
	TrackerSparqlCursor *cursor;
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);

	query = g_strdup_printf ("SELECT tracker:uri(%d) tracker:uri(%d) {}", sub, obj);
	cursor = tracker_sparql_connection_query (priv->tracker, query, NULL, NULL);
	tracker_sparql_cursor_next (cursor, NULL, NULL);
	subject = tracker_sparql_cursor_get_string (cursor, 0, NULL);
	object = tracker_sparql_cursor_get_string (cursor, 1, NULL);

	if (object != NULL && strcmp (object, FEED_MESSAGE_TRACKER_CLASS) == 0)
		collect_items (engine, NULL, subject);

	g_free (query);
	g_object_unref (cursor);
}

static GdkPixbuf*
get_icon_for_channel (const gchar *url)
{
	GdkPixbuf *ret;
	GError *error;

	ret = NULL;
	error = NULL;

	if (url != NULL) {
		ret = gdk_pixbuf_new_from_resource_at_scale (url, ICON_SIZE, ICON_SIZE, TRUE, &error);
		if (ret == NULL) {
			g_message ("Unable to build icon for channel: %s", error->message);
			g_error_free (error);
		}
	}

	if (ret == NULL)
		ret = gtk_icon_theme_load_icon (gtk_icon_theme_get_default (),
						"application-rss+xml-symbolic", ICON_SIZE,
						GTK_ICON_LOOKUP_GENERIC_FALLBACK, NULL);

	return ret;
}

static void
pass_channels (ReaderEngine *engine,
               TrackerSparqlCursor *cursor)
{
	const gchar *subject;
	const gchar *title;
	const gchar *url;
	gint64 unread;
	GtkTreeModel *model;
	GtkTreeIter *iter;
	GtkTreeIter iter2;
	GtkTreeStore *items_model;
	GdkPixbuf *icon;
	ReaderEnginePrivate *priv;

	if (cursor != NULL) {
		priv = reader_engine_get_instance_private (engine);
		model = GTK_TREE_MODEL (priv->data);

		while (tracker_sparql_cursor_next (cursor, NULL, NULL)) {
			subject = tracker_sparql_cursor_get_string (cursor, 0, NULL);
			title = tracker_sparql_cursor_get_string (cursor, 1, NULL);
			unread = tracker_sparql_cursor_get_integer (cursor, 2);
			url = tracker_sparql_cursor_get_string (cursor, 4, NULL);

			if (find_in_model (model, GD_MAIN_COLUMN_ID, subject, &iter)) {
				gtk_list_store_set (GTK_LIST_STORE (model), iter,
							GD_MAIN_COLUMN_PRIMARY_TEXT, title,
							EXTRA_COLUMN_UNREADS, unread, -1);
				gtk_tree_iter_free (iter);
			}
			else {
				items_model = gtk_tree_store_new (ITEM_COLUMN_LAST,
				                                  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
				                                  G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT, G_TYPE_STRING);

				g_signal_connect (items_model, "row-changed", G_CALLBACK (model_item_changed), engine);

				icon = get_icon_for_channel (tracker_sparql_cursor_get_string (cursor, 3, NULL));

				gtk_list_store_append (GTK_LIST_STORE (model), &iter2);
				gtk_list_store_set (GTK_LIST_STORE (model), &iter2,
				                    GD_MAIN_COLUMN_ID, subject,
				                    GD_MAIN_COLUMN_PRIMARY_TEXT, title,
				                    GD_MAIN_COLUMN_ICON, icon,
				                    EXTRA_COLUMN_UNREADS, unread,
				                    EXTRA_COLUMN_URL, url,
				                    EXTRA_COLUMN_FEEDS_MODEL, items_model, -1);
			}

			collect_items (engine, subject, NULL);
		}

		g_object_unref (cursor);
	}
}

static void
on_fetch_channels (GObject *source,
                   GAsyncResult *res,
                   ReaderEngine *engine)
{
	TrackerSparqlCursor *cursor;
	GError *error = NULL;

	cursor = tracker_sparql_connection_query_finish (TRACKER_SPARQL_CONNECTION (source), res, &error);

	if (error != NULL) {
		g_warning ("Unable to fetch channels from Tracker: %s", error->message);
		g_error_free (error);
	}
	else {
		pass_channels (engine, cursor);
	}
}

static void
collect_channels (ReaderEngine *engine,
                  const gchar *subject)
{
	gchar *query;
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);

	if (subject == NULL)
		query = g_strdup ("SELECT ?s ?t ?u ?i ?r "
		                  "WHERE {?s a mfo:FeedChannel; "
		                         "nie:title ?t; "
		                         "mfo:unreadCount ?u; "
		                         "mfo:image ?i; "
		                         "nie:url ?r}");
	else
		query = g_strdup_printf ("SELECT <%s> ?t ?u ?i ?r "
		                         "WHERE {<%s> nie:title ?t; "
		                                "mfo:unreadCount ?u; "
		                                "mfo:image ?i; "
		                                "nie:url ?r}", subject, subject);

	tracker_sparql_connection_query_async (priv->tracker, query, NULL,
	                                       (GAsyncReadyCallback) on_fetch_channels, engine);
	g_free (query);
}

static void
handle_channel_inserts (ReaderEngine *engine,
                        gint sub,
                        gint obj)
{
	gchar *query;
	const gchar *subject;
	const gchar *object;
	TrackerSparqlCursor *cursor;
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);

	query = g_strdup_printf ("SELECT tracker:uri(%d) tracker:uri(%d) {}", sub, obj);
	cursor = tracker_sparql_connection_query (priv->tracker, query, NULL, NULL);
	tracker_sparql_cursor_next (cursor, NULL, NULL);
	subject = tracker_sparql_cursor_get_string (cursor, 0, NULL);
	object = tracker_sparql_cursor_get_string (cursor, 1, NULL);

	if (object != NULL && strcmp (object, FEED_CHANNEL_TRACKER_CLASS) == 0)
		collect_channels (engine, subject);

	g_free (query);
	g_object_unref (cursor);
}

static void
remove_channel_data (GtkTreeModel *model, GtkTreeIter *iter)
{
	GtkListStore *items;
	GDateTime *gdate;
	GtkTreeIter subiter;

	gtk_tree_model_get (model, iter, EXTRA_COLUMN_FEEDS_MODEL, &items, -1);

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (items), &subiter) != FALSE) {
		do {
			gtk_tree_model_get (GTK_TREE_MODEL (items), &subiter, ITEM_COLUMN_TIME, &gdate, -1);
			g_date_time_unref (gdate);
		} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (items), &subiter));
	}

	g_object_unref (items);
}

/*
	Warning: this function scans the whole model to find an item accordly
	only to his subject. Handle with care!
*/
static gboolean
discover_item (ReaderEngine *engine,
               const gchar *subject,
               GtkTreeModel **target_model,
               GtkTreeIter *target_iter)
{
	gboolean found;
	gchar *test_id;
	GtkTreeIter iter;
	GtkTreeIter subiter;
	GtkTreeIter parent;
	GtkTreeStore *items;
	GtkTreeModel *data;
	GtkTreeModel *imodel;
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);
	data = GTK_TREE_MODEL (priv->data);

	if (gtk_tree_model_get_iter_first (data, &iter) == FALSE)
		return FALSE;

	found = FALSE;

	do {
		gtk_tree_model_get (data, &iter, EXTRA_COLUMN_FEEDS_MODEL, &items, -1);
		imodel = GTK_TREE_MODEL (items);

		if (gtk_tree_model_get_iter_first (imodel, &parent) == FALSE)
			continue;

		do {
			if (gtk_tree_model_iter_children (imodel, &subiter, &parent) == FALSE)
				continue;

			do {
				gtk_tree_model_get (imodel, &subiter, ITEM_COLUMN_ID, &test_id, -1);

				if (strcmp (test_id, subject) == 0) {
					found = TRUE;
					*target_model = imodel;
					*target_iter = subiter;
				}

				g_free (test_id);

			} while (found == FALSE && gtk_tree_model_iter_next (imodel, &subiter));

		} while (found == FALSE && gtk_tree_model_iter_next (imodel, &subiter));

	} while (found == FALSE && gtk_tree_model_iter_next (data, &iter));

	return found;
}

static void
handle_feed_deletes (ReaderEngine *engine,
                     gint sub,
                     gint obj)
{
	gchar *query;
	const gchar *subject;
	const gchar *object;
	GtkTreeModel *model;
	GtkTreeIter iter;
	GDateTime *date;
	TrackerSparqlCursor *cursor;
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);

	query = g_strdup_printf ("SELECT tracker:uri(%d) tracker:uri(%d) {}", sub, obj);
	cursor = tracker_sparql_connection_query (priv->tracker, query, NULL, NULL);
	tracker_sparql_cursor_next (cursor, NULL, NULL);
	subject = tracker_sparql_cursor_get_string (cursor, 0, NULL);
	object = tracker_sparql_cursor_get_string (cursor, 1, NULL);

	if (object != NULL && strcmp (object, FEED_MESSAGE_TRACKER_CLASS) == 0) {
		if (discover_item (engine, subject, &model, &iter)) {
			/*
				TODO	If the removed item is the last one for
					his date, remove and free also the
					parent row containing the date header
			*/

			gtk_tree_model_get (model, &iter, ITEM_COLUMN_TIME, &date, -1);
			g_date_time_unref (date);

			gtk_tree_store_remove (GTK_TREE_STORE (model), &iter);
		}
	}

	g_free (query);
	g_object_unref (cursor);
}

static void
handle_channel_deletes (ReaderEngine *engine,
                        gint sub,
                        gint obj)
{
	gchar *query;
	const gchar *subject;
	const gchar *object;
	GtkTreeIter *iter;
	TrackerSparqlCursor *cursor;
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);

	query = g_strdup_printf ("SELECT tracker:uri(%d) tracker:uri(%d) {}", sub, obj);
	cursor = tracker_sparql_connection_query (priv->tracker, query, NULL, NULL);
	tracker_sparql_cursor_next (cursor, NULL, NULL);
	subject = tracker_sparql_cursor_get_string (cursor, 0, NULL);
	object = tracker_sparql_cursor_get_string (cursor, 1, NULL);

	if (object != NULL && strcmp (object, FEED_CHANNEL_TRACKER_CLASS) == 0) {
		if (find_in_model (GTK_TREE_MODEL (priv->data), GD_MAIN_COLUMN_ID, subject, &iter)) {
			remove_channel_data (GTK_TREE_MODEL (priv->data), iter);
			gtk_list_store_remove (priv->data, iter);
			gtk_tree_iter_free (iter);
		}
	}

	g_free (query);
	g_object_unref (cursor);
}

static void
on_message_update (GDBusConnection *connection,
                   const gchar *sender_name,
                   const gchar *object_path,
                   const gchar *interface_name,
                   const gchar *signal_name,
                   GVariant *parameters,
                   ReaderEngine *engine)
{
	GVariantIter *iter1, *iter2;
	gchar *class_name;
	gint graph = 0, subject = 0, predicate = 0, object = 0;

	g_variant_get (parameters, "(&sa(iiii)a(iiii))", &class_name, &iter1, &iter2);

	while (g_variant_iter_loop (iter1, "(iiii)", &graph, &subject, &predicate, &object))
		handle_feed_deletes (engine, subject, object);

	while (g_variant_iter_loop (iter2, "(iiii)", &graph, &subject, &predicate, &object))
		handle_feed_inserts (engine, subject, object);

	g_variant_iter_free (iter1);
	g_variant_iter_free (iter2);
}

static void
on_channel_update (GDBusConnection *connection,
                   const gchar *sender_name,
                   const gchar *object_path,
                   const gchar *interface_name,
                   const gchar *signal_name,
                   GVariant *parameters,
                   ReaderEngine *engine)
{
	GVariantIter *iter1, *iter2;
	gchar *class_name;
	gint graph = 0, subject = 0, predicate = 0, object = 0;

	g_variant_get (parameters, "(&sa(iiii)a(iiii))", &class_name, &iter1, &iter2);

	while (g_variant_iter_loop (iter1, "(iiii)", &graph, &subject, &predicate, &object))
		handle_channel_deletes (engine, subject, object);

	while (g_variant_iter_loop (iter2, "(iiii)", &graph, &subject, &predicate, &object))
		handle_channel_inserts (engine, subject, object);

	g_variant_iter_free (iter1);
	g_variant_iter_free (iter2);
}

static void
reader_engine_finalize (GObject *object)
{
	GtkTreeIter iter;
	ReaderEngine *engine;
	ReaderEnginePrivate *priv;

	engine = READER_ENGINE (object);
	priv = reader_engine_get_instance_private (engine);

	if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (priv->data), &iter) == FALSE)
		return;

	do {
		remove_channel_data (GTK_TREE_MODEL (priv->data), &iter);
	} while (gtk_tree_model_iter_next (GTK_TREE_MODEL (priv->data), &iter));

	g_object_unref (priv->data);
}

static void
reader_engine_init (ReaderEngine *engine)
{
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);

	/*
		TODO error handling
	*/
	priv->connection = g_bus_get_sync (G_BUS_TYPE_SESSION, NULL, NULL);
	priv->tracker = tracker_sparql_connection_get (NULL, NULL);

	priv->data = gtk_list_store_new (EXTRA_COLUMN_LAST,
	                                 G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
	                                 GDK_TYPE_PIXBUF, G_TYPE_LONG, G_TYPE_BOOLEAN, G_TYPE_INT,
	                                 G_TYPE_INT, G_TYPE_STRING, G_TYPE_POINTER);

	g_dbus_connection_signal_subscribe (priv->connection,
	                                    TRACKER_DBUS_SERVICE,
	                                    TRACKER_DBUS_INTERFACE_RESOURCES,
	                                    "GraphUpdated",
	                                    TRACKER_DBUS_OBJECT_RESOURCES,
	                                    FEED_CHANNEL_TRACKER_CLASS,
	                                    G_DBUS_SIGNAL_FLAGS_NONE,
	                                    (GDBusSignalCallback) on_channel_update, engine, NULL);

	g_dbus_connection_signal_subscribe (priv->connection,
	                                    TRACKER_DBUS_SERVICE,
	                                    TRACKER_DBUS_INTERFACE_RESOURCES,
	                                    "GraphUpdated",
	                                    TRACKER_DBUS_OBJECT_RESOURCES,
	                                    FEED_MESSAGE_TRACKER_CLASS,
	                                    G_DBUS_SIGNAL_FLAGS_NONE,
	                                    (GDBusSignalCallback) on_message_update, engine, NULL);

	collect_channels (engine, NULL);
}

static void
reader_engine_class_init (ReaderEngineClass *class)
{
	G_OBJECT_CLASS (class)->finalize = reader_engine_finalize;
}

ReaderEngine*
reader_engine_new ()
{
	return g_object_new (READER_ENGINE_TYPE, NULL);
}

GtkTreeModel*
reader_engine_get_channels_model (ReaderEngine *engine)
{
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);
	return GTK_TREE_MODEL (priv->data);
}

GtkTreeModel*
reader_engine_get_items_model (ReaderEngine *engine, const gchar *channel)
{
	GtkTreeModel *ret;
	GtkTreeIter *iter;
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);

	if (find_in_model (GTK_TREE_MODEL (priv->data), GD_MAIN_COLUMN_ID, channel, &iter) == FALSE)
		return NULL;

	gtk_tree_model_get (GTK_TREE_MODEL (priv->data), iter, EXTRA_COLUMN_FEEDS_MODEL, &ret, -1);
	gtk_tree_iter_free (iter);
	return GTK_TREE_MODEL (ret);
}

gboolean
reader_engine_has_channel (ReaderEngine *engine,
                           const gchar *url)
{
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);
	return find_in_model (GTK_TREE_MODEL (priv->data), EXTRA_COLUMN_URL, url, NULL);
}

void
reader_engine_push_channel (ReaderEngine *engine,
                            GrssFeedChannel *channel)
{
	gchar *query;
	const gchar *image;
	ReaderEnginePrivate *priv;

	image = grss_feed_channel_get_image (channel);
	if (image == NULL)
		image = "";

	priv = reader_engine_get_instance_private (engine);

	query = g_strdup_printf ("INSERT {_:a a mfo:FeedSettings; mfo:updateInterval 20 . "
	                                 "_:b a mfo:FeedChannel; a nie:DataObject; nie:url \"%s\"; "
	                                                        "nie:title \"%s\"; "
	                                                        "mfo:unreadCount 0; "
	                                                        "mfo:image \"%s\"; mfo:feedSettings _:a}",
	                         grss_feed_channel_get_source (channel),
	                         grss_feed_channel_get_title (channel),
	                         image);

	tracker_sparql_connection_update_blank_async (priv->tracker, query, 0, NULL,
	                                              (GAsyncReadyCallback) verify_tracker_update, NULL);
	g_free (query);
}

void
reader_engine_delete_channels (ReaderEngine *engine,
                               GList *channels)
{
	gchar *query;
	gchar *subject;
	GList *liter;
	GtkTreeIter iter;
	ReaderEnginePrivate *priv;

	priv = reader_engine_get_instance_private (engine);

	for (liter = channels; liter; liter = liter->next) {
		gtk_tree_model_get_iter (GTK_TREE_MODEL (priv->data), &iter, liter->data);
		gtk_tree_model_get (GTK_TREE_MODEL (priv->data), &iter, GD_MAIN_COLUMN_ID, &subject, -1);

		query = g_strdup_printf ("DELETE {?s a rdfs:Resource} WHERE {<%s> mfo:feedSettings ?s}; "
		                         "DELETE {?i a rdfs:Resource} WHERE {?i nmo:communicationChannel <%s>}; "
		                         "DELETE {<%s> a rdfs:Resource}", subject, subject, subject);

		tracker_sparql_connection_update_async (priv->tracker, query, 0, NULL,
			                                (GAsyncReadyCallback) verify_tracker_update, NULL);
		g_free (query);
		g_free (subject);

		gtk_tree_path_free (liter->data);

		/*
			The channel itself is removed from the local data
			structure after receiving the proper signal from DBus
		*/
	}

	g_list_free (channels);
}

