SUBDIRS = . plugins

lib_LTLIBRARIES = libgimo-1.0.la
libgimo_1_0_la_CFLAGS = ${STRICT_CFLAGS} ${GLIB_CFLAGS} ${EXPAT_CFLAGS}
libgimo_1_0_la_LDFLAGS = -version-info ${LT_VERSION_INFO} \
	${GLIB_LIBS} ${EXPAT_LIBS}
libgimo_1_0_la_SOURCES_COMMON = gimo-types.h gimo-types.c \
	gimo-enums.h gimo-enums.c gimo-error.h gimo-error.c \
	gimo-context.h gimo-context.c gimo-plugin.h gimo-plugin.c \
	gimo-require.h gimo-require.c gimo-extpoint.h gimo-extpoint.c \
	gimo-extension.h gimo-extension.c gimo-loader.h gimo-loader.c \
	gimo-factory.h gimo-factory.c gimo-loadable.h gimo-loadable.c \
	gimo-module.h gimo-module.c gimo-dlmodule.h gimo-dlmodule.c \
	gimo-archive.h gimo-archive.c gimo-xmlarchive.h gimo-xmlarchive.c \
	gimo-marshal.h gimo-marshal.c gimo-utils.h gimo-utils.c \
	gimo-extconfig.h gimo-extconfig.c gimo-datastore.h gimo-datastore.c \
	gimo-runnable.h gimo-runnable.c gimo-signalbus.h gimo-signalbus.c
libgimo_1_0_la_SOURCES = ${libgimo_1_0_la_SOURCES_COMMON} \
	gimo-intl.h

gimoincludedir=$(includedir)/gimo-1.0
gimoinclude_HEADERS = gimo-types.h gimo-enums.h gimo-error.h \
	gimo-context.h gimo-plugin.h gimo-require.h gimo-extpoint.h \
	gimo-extension.h gimo-loader.h gimo-factory.h gimo-loadable.h \
	gimo-module.h gimo-dlmodule.h gimo-archive.h gimo-xmlarchive.h \
	gimo-marshal.h gimo-utils.h gimo-extconfig.h gimo-datastore.h \
	gimo-runnable.h gimo-signalbus.h gimo.h

CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)

if HAVE_INTROSPECTION

INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) \
	--warn-all --warn-error
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)
INTROSPECTION_GIRS = Gimo-1.0.gir

Gimo-1.0.gir: $(INTROSPECTION_SCANNER) Makefile

Gimo_1_0_gir_INCLUDES = GLib-2.0 GObject-2.0 Gio-2.0
Gimo_1_0_gir_CFLAGS = $(INCLUDES) -I${srcdir}
Gimo_1_0_gir_LIBS = ./libgimo-1.0.la
Gimo_1_0_gir_FILES = ${libgimo_1_0_la_SOURCES_COMMON}
Gimo_1_0_gir_EXPORT_PACKAGES = gimo-1.0

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif # HAVE_INTROSPECTION
