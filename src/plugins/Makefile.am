PLUGIN_LIBTOOL_FLAGS = -module -avoid-version -shared
AM_CPPFLAGS = ${STRICT_CFLAGS} ${GLIB_CFLAGS} ${GIMO_CFLAGS}
AM_LDFLAGS = ${GLIB_LIBS} ${GIMO_LDFLAGS} ${PLUGIN_LIBTOOL_FLAGS}

lib_LTLIBRARIES = pymodule-1.0.la jsmodule-1.0.la
pymodule_1_0_la_SOURCES = gimo-pymodule.h gimo-pymodule.c
jsmodule_1_0_la_SOURCES = gimo-jsmodule.h gimo-jsmodule.c
