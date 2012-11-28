#include <glib.h>

#define GETTEXT_PACKAGE "gimo10"
#define VERSION "1.0.0"

#ifdef G_OS_WIN32
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define isblank(c) (c == ' ')
#endif

#define HAVE_INTROSPECTION 1