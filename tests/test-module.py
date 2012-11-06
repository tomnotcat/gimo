#!/usr/bin/env python

import os
from gi.repository import Gimo

def dlmodule_new (user_data):
    assert (user_data == "hello")
    return Gimo.Dlmodule ()

loader = Gimo.Loader ()
loader.add_paths (os.getenv ("GIMO_MODULE_PATH"))

# Dynamic library
assert (loader.load ("demo-plugin") == None)
factory = Gimo.Factory.new (dlmodule_new, "hello")
assert (loader.register (None, factory))
module = loader.load ("demo-plugin")
assert (module)
plugin = module.resolve ("test_plugin_new", None)
assert (plugin)

# Python module
assert (loader.load ("demo-plugin.py") == None)
module = loader.load ("pymodule-1.0");
assert (module)

# JavaScript module
assert (loader.load ("demo-plugin.js") == None)
module = loader.load ("jsmodule-1.0");
assert (module)
