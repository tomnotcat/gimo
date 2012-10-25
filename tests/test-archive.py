#!/usr/bin/env python

from gi.repository import Gimo

def dlmodule_new (user_data):
    assert (user_data == "hello")
    return Gimo.Dlmodule ()

loader = Gimo.Loader ()

# Dynamic library
assert (loader.load ("testplugin") == None)
loader.register (None, dlmodule_new, "hello")
module = loader.load ("testplugin")
assert (module)
plugin = module.resolve ("test_plugin_new", None)
assert (plugin)

# Python module
assert (loader.load ("testplugin.py") == None)
module = loader.load ("pymodule-1.0");
assert (module)

# JavaScript module
assert (loader.load ("testplugin.js") == None)
module = loader.load ("jsmodule-1.0");
assert (module)