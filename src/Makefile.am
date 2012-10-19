lib_LTLIBRARIES = libgimo-1.0.la
libgimo_1_0_la_CFLAGS = ${STRICT_CFLAGS} ${GLIB_CFLAGS}
libgimo_1_0_la_LDFLAGS = -version-info ${LT_VERSION_INFO} \
	${GLIB_LIBS}
libgimo_1_0_la_SOURCES = gimo-enums.h gimo-types.h \
	gimo-registry.h gimo-registry.c gimo-pluginfo.h gimo-pluginfo.c \
	gimo-require.h gimo-require.c gimo-extpoint.h gimo-extpoint.c \
	gimo-extension.h gimo-extension.c gimo-plugin.h gimo-plugin.c \
	gimo-loader.h gimo-loader.c gimo-dlloader.h gimo-dlloader.c \
	gimo-utils.h gimo-utils.c

CLEANFILES =

-include $(INTROSPECTION_MAKEFILE)

if HAVE_INTROSPECTION

INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) \
	--warn-all --warn-error
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)
INTROSPECTION_GIRS = Gimo-1.0.gir

Gimo-1.0.gir: $(INTROSPECTION_SCANNER) Makefile

Gimo_1_0_gir_INCLUDES = GObject-2.0
Gimo_1_0_gir_CFLAGS = $(INCLUDES) -I${srcdir}
Gimo_1_0_gir_LIBS = ./libgimo-1.0.la
Gimo_1_0_gir_FILES = ${libgimo_1_0_la_SOURCES}

girdir = $(datadir)/gir-1.0
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-1.0
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)

endif # HAVE_INTROSPECTION
