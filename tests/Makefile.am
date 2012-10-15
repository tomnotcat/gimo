AM_CPPFLAGS = ${STRICT_CFLAGS} ${GLIB_CFLAGS} ${GIMO_CFLAGS}
AM_LDFLAGS = ${GLIB_LIBS} ${GIMO_LDFLAGS}

check_PROGRAMS =
check_SCRIPTS =

# Test plugin.
check_PROGRAMS += test-plugin
test_plugin_SOURCES = test-plugin.c

TESTS = ${check_PROGRAMS} ${check_SCRIPTS}
EXTRA_DIST = ${check_SCRIPTS}