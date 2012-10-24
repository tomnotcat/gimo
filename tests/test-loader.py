#!/usr/bin/env python

from gi.repository import Gimo

def dlmodule_new (user_data):
    assert (user_data == "hello")
    return Gimo.Dlmodule ()

loader = Gimo.Loader ()
loader.register (None, dlmodule_new, "hello")
module = loader.load ("testplugin")
assert (module)
plugin = module.resolve ("test_plugin_new", None)
assert (plugin)
