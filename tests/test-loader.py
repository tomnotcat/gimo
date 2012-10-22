#!/usr/bin/env python

from gi.repository import Gimo

info = Gimo.Pluginfo (identifier="test.plugin1",
                      url="libtestplugin",
                      symbol="test_plugin_new")
dlld = Gimo.Dlloader ()
plugin = dlld.load (info)
assert (plugin)
