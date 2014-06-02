BUILT_SOURCES =

AM_CFLAGS =					\
	-DPACKAGE_LOCALE_DIR=\""$(localedir)"\"	\
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\"	\
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\"	\
	$(MAINTAINER_COMPILER_FLAGS)		\
	-Wall					\
	-g -O2					\
	-I../libgd				\
	$(READER_CFLAGS)

gresource_file = ../data/reader.gresource.xml
resource_files = $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir=../data $(gresource_file))
resources.c: $(gresource_file) $(resource_files)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=../data --generate-source $<
BUILT_SOURCES += resources.c

bin_PROGRAMS = gnome-reader

gnome_reader_SOURCES =		\
	$(BUILT_SOURCES)	\
	common.h		\
	reader-application.c	\
	reader-application.h	\
	reader-appwin.c		\
	reader-appwin.h		\
	reader-channel-add.c	\
	reader-channel-add.h	\
	reader-channels-view.c	\
	reader-channels-view.h	\
	reader-engine.c		\
	reader-engine.h		\
	reader-items-view.c	\
	reader-items-view.h	\
	reader-main.c		\
	reader-topbar.c		\
	reader-topbar.h

gnome_reader_LDFLAGS =		\
	-Wl,--export-dynamic

gnome_reader_LDADD =		\
	$(READER_LIBS)		\
	../libgd/libgd.la

CLEANFILES =			\
	$(BUILT_SOURCES)	\
	$(NULL)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
