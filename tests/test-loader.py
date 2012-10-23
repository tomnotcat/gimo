#!/usr/bin/env python

from gi.repository import Gimo

module = Gimo.Dlmodule ()
assert (module.open ("testplugin"))
plugin = module.resolve ("test_plugin_new", None)
assert (plugin)
