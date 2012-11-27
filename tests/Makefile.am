all: all-am
	@if test -n "${TEST}"; then \
		$(TESTS_ENVIRONMENT) ${TEST}; \
	fi

AM_CPPFLAGS = ${STRICT_CFLAGS} ${GLIB_CFLAGS} ${GIMO_CFLAGS} \
	-DTEST_TOP_SRCDIR="\"${top_srcdir}/tests/\""
AM_LDFLAGS = ${GLIB_LIBS} ${GIMO_LDFLAGS}

DEMO_PLUGIN_LIBTOOL_FLAGS = \
	-module \
	-shared \
	-avoid-version \
	-rpath ${abs_builddir}

noinst_LTLIBRARIES = demo-plugin.la
demo_plugin_la_LDFLAGS = ${AM_LDFLAGS} ${DEMO_PLUGIN_LIBTOOL_FLAGS}
demo_plugin_la_SOURCES = demo-plugin.c

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

# Test plugin descriptor.
check_PROGRAMS += test-plugin
test_plugin_SOURCES = test-plugin.c
check_SCRIPTS += test-plugin.py

# Test plugin context.
check_PROGRAMS += test-context
test_context_SOURCES = test-context.c
check_SCRIPTS += test-context.py

TESTS = ${check_PROGRAMS}

-include $(INTROSPECTION_MAKEFILE)

if HAVE_INTROSPECTION
	TESTS += ${check_SCRIPTS}
endif

EXTRA_DIST = ${check_SCRIPTS} \
	demo-plugin.py demo-plugin.js sub-plugin.js demo-plugin.xml \
	demo-archive1.xml demo-archive2.xml \
	plugins/plugin1.js \
	plugins/plugin2.py \
	plugins/plugin1.xml \
	plugins/plugin2.xml

test:
	$(TESTS_ENVIRONMENT) ${TEST}
