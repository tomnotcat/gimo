#!/usr/bin/env python

from gi.repository import Gimo

info = Gimo.Pluginfo ()
assert (info.get_id () == None)
assert (info.get_name () == None)
assert (info.get_version () == None)
assert (info.get_provider () == None)
assert (info.get_uri () == None)
assert (info.get_module () == None)
assert (len (info.get_requires ()) == 0)
assert (len (info.get_extpoints ()) == 0)
assert (len (info.get_extensions ()) == 0)
assert (info.get_state () == Gimo.PluginState.UNINSTALLED)

requires = [Gimo.Require (plugin="require-plugin",
                          version="1.0",
                          optional=True)]
extpoints = [Gimo.ExtPoint (id="extpoint1",
                            name="hello")]
extensions = [Gimo.Extension (id="extension1",
                              name="world",
                              point="test.plugin.extpoint1")]
info = Gimo.Pluginfo.new ("test.plugin",
                          "myname",
                          "1.0",
                          "tomnotcat",
                          "/home/test",
                          "mymodule",
                          requires,
                          extpoints,
                          extensions)
assert (info.get_id () == "test.plugin")
assert (info.get_name () == "myname")
assert (info.get_version () == "1.0")
assert (info.get_provider () == "tomnotcat")
assert (info.get_uri () == "/home/test")
assert (info.get_module () == "mymodule")
assert (len (info.get_requires ()) == len (requires))
assert (len (info.get_extpoints ()) == len (extpoints))
assert (len (info.get_extensions ()) == len (extensions))

it = info.get_requires ()[0]
assert (it.get_plugin_id () == "require-plugin")
assert (it.get_version () == "1.0")
assert (it.is_optional ())

it = info.get_extpoints ()[0]
assert (it.get_local_id () == "extpoint1")
assert (it.get_name () == "hello")
assert (it.get_id () == "test.plugin.extpoint1")
assert (it.query_pluginfo () == info)

it = info.get_extensions ()[0]
assert (it.get_local_id () == "extension1")
assert (it.get_name () == "world")
assert (it.get_extpoint_id () == "test.plugin.extpoint1")
assert (it.get_id () == "test.plugin.extension1")
assert (it.query_pluginfo () == info)
