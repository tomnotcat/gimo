#!/usr/bin/env python
import sys, os

name = "g-ir-scanner.py"
path = os.environ['Path']
scanner = ""

for it in path.split (';'):
    if os.path.exists (it + os.path.sep + name):
        scanner = it + os.path.sep + name
        break

srcs = '''../../src/gimo-archive.h ../../src/gimo-archive.c ../../src/gimo-binding.h ../../src/gimo-binding.c \
 ../../src/gimo-context.h ../../src/gimo-context.c ../../src/gimo-dlmodule.h ../../src/gimo-dlmodule.c \
 ../../src/gimo-enums.h ../../src/gimo-enums.c ../../src/gimo-runnable.h ../../src/gimo-runnable.c \
 ../../src/gimo-error.h ../../src/gimo-error.c ../../src/gimo-extconfig.h ../../src/gimo-extconfig.c \
 ../../src/gimo-extension.h ../../src/gimo-extension.c ../../src/gimo-extpoint.h ../../src/gimo-extpoint.c \
 ../../src/gimo-factory.h ../../src/gimo-factory.c ../../src/gimo-loadable.h ../../src/gimo-loadable.c \
 ../../src/gimo-loader.h ../../src/gimo-loader.c ../../src/gimo-module.h ../../src/gimo-module.c \
 ../../src/gimo-plugin.h ../../src/gimo-plugin.c ../../src/gimo-signalbus.h ../../src/gimo-signalbus.c \
 ../../src/gimo-require.h ../../src/gimo-require.c ../../src/gimo-types.c ../../src/gimo-types.h \
 ../../src/gimo-utils.c ../../src/gimo-utils.h ../../src/gimo-xmlarchive.c ../../src/gimo-xmlarchive.h'''

os.system ("cd " + sys.argv[1] + " && python.exe \"" + scanner + "\" --add-include-path=. --warn-all --namespace=Gimo --nsversion=1.0 --no-libtool \
 --include=GObject-2.0   --library=" + sys.argv[2] + " -I../../src --output Gimo-1.0.gir " + srcs +
" && g-ir-compiler --includedir=. Gimo-1.0.gir -o Gimo-1.0.typelib")
