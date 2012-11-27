PLUGIN_LIBTOOL_FLAGS = -module -avoid-version -shared
AM_CPPFLAGS = ${GLIB_CFLAGS} ${GIMO_CFLAGS}
AM_LDFLAGS = ${GLIB_LIBS} ${GIMO_LDFLAGS} ${PLUGIN_LIBTOOL_FLAGS}

pluginlibdir = ${libdir}/gimo-plugins-1.0
pluginlib_LTLIBRARIES = pymodule-1.0.la jsmodule-1.0.la
pluginlib_DATA = pymodule-1.0.xml jsmodule-1.0.xml

pymodule_1_0_la_SOURCES = gimo-pymodule.h gimo-pymodule.c
pymodule_1_0_la_CFLAGS = ${AM_CPPFLAGS} ${PYGOBJECT_CFLAGS}
pymodule_1_0_la_LDFLAGS = ${AM_LDFLAGS} ${PYGOBJECT_LIBS}

jsmodule_1_0_la_SOURCES = gimo-jsmodule.h gimo-jsmodule.c
jsmodule_1_0_la_CFLAGS = ${AM_CPPFLAGS} ${GJS_CFLAGS}
jsmodule_1_0_la_LDFLAGS = ${AM_LDFLAGS} ${GJS_LIBS}

EXTRA_DIST = pymodule-1.0.xml jsmodule-1.0.xml