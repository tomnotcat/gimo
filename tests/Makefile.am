AM_CPPFLAGS = ${STRICT_CFLAGS} ${GLIB_CFLAGS} ${GIMO_CFLAGS}
AM_LDFLAGS = ${GLIB_LIBS} ${GIMO_LDFLAGS}

TEST_PLUGIN_LIBTOOL_FLAGS = \
	-module \
	-shared \
	-avoid-version \
	-rpath ${abs_builddir}

noinst_LTLIBRARIES = testplugin.la
testplugin_la_LDFLAGS = ${AM_LDFLAGS} ${TEST_PLUGIN_LIBTOOL_FLAGS}
testplugin_la_SOURCES = test-plugin.c

check_PROGRAMS =
check_SCRIPTS =

# Test plugin module.
check_PROGRAMS += test-module
test_module_SOURCES = test-module.c
check_SCRIPTS += test-module.py

# Test object archive.
check_PROGRAMS += test-archive
test_archive_SOURCES = test-archive.c
check_SCRIPTS += test-archive.py

# Test plugin info.
check_PROGRAMS += test-pluginfo
test_pluginfo_SOURCES = test-pluginfo.c
check_SCRIPTS += test-pluginfo.py

# Test plugin context.
check_PROGRAMS += test-context
test_context_SOURCES = test-context.c
check_SCRIPTS += test-context.py

TESTS = ${check_PROGRAMS} ${check_SCRIPTS}
EXTRA_DIST = ${check_SCRIPTS}
