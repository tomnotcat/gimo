#!/usr/bin/env python

import os
from gi.repository import Gimo

def dlmodule_new (user_data):
    assert (user_data == "hello")
    return Gimo.Dlmodule ()

loader = Gimo.Loader ()
assert (len (loader.dup_paths ()) == 0)
loader.add_paths (os.getenv ("GIMO_PLUGIN_PATH"))
paths = loader.dup_paths ()
assert (len (paths) == 4)
assert (paths[3] + ":" +
        paths[2] + ":" +
        paths[1] + ":" +
        paths[0] == os.getenv ("GIMO_PLUGIN_PATH"))

# Dynamic library
assert (loader.load ("demo-plugin.so") == None)
factory = Gimo.Factory.new (dlmodule_new, "hello")
assert (loader.register (None, factory))
module = loader.load ("demo-plugin.so")
assert (module)
plugin = module.resolve ("test_plugin_new", None, True)
assert (plugin)

# Python module
assert (loader.load ("demo-plugin.py") == None)
module = loader.load ("pymodule-1.0.so");
assert (module)

# JavaScript module
assert (loader.load ("demo-plugin.js") == None)
module = loader.load ("jsmodule-1.0.so");
assert (module)
