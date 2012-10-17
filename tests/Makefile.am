AM_CPPFLAGS = ${STRICT_CFLAGS} ${GLIB_CFLAGS} ${GIMO_CFLAGS}
AM_LDFLAGS = ${GLIB_LIBS} ${GIMO_LDFLAGS}

check_PROGRAMS =
check_SCRIPTS =

# Test plugin info.
check_PROGRAMS += test-pluginfo
test_pluginfo_SOURCES = test-pluginfo.c

TESTS = ${check_PROGRAMS} ${check_SCRIPTS}
EXTRA_DIST = ${check_SCRIPTS}