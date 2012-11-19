#!/usr/bin/env python

from gi.repository import Gimo

plugin = Gimo.Plugin ()
assert (plugin.get_id () == None)
assert (plugin.get_name () == None)
assert (plugin.get_version () == None)
assert (plugin.get_provider () == None)
assert (plugin.get_module () == None)
assert (len (plugin.get_requires ()) == 0)
assert (len (plugin.get_extpoints ()) == 0)
assert (len (plugin.get_extensions ()) == 0)
assert (plugin.get_state () == Gimo.PluginState.UNINSTALLED)

requires = [Gimo.Require (plugin="require-plugin",
                          version="1.0",
                          optional=True)]
extpoints = [Gimo.ExtPoint (id="extpoint1",
                            name="hello")]
extensions = [Gimo.Extension (id="extension1",
                              name="world",
                              point="test.plugin.extpoint1")]
plugin = Gimo.Plugin.new ("test.plugin",
                          "myname",
                          "1.0",
                          "tomnotcat",
                          ".",
                          "mymodule",
                          "mysymbol",
                          requires,
                          extpoints,
                          extensions)
assert (plugin.get_id () == "test.plugin")
assert (plugin.get_name () == "myname")
assert (plugin.get_version () == "1.0")
assert (plugin.get_provider () == "tomnotcat")
assert (plugin.get_path () == ".")
assert (plugin.get_module () == "mymodule")
assert (len (plugin.get_requires ()) == len (requires))
assert (len (plugin.get_extpoints ()) == len (extpoints))
assert (len (plugin.get_extensions ()) == len (extensions))

it = plugin.get_requires ()[0]
assert (it.get_plugin_id () == "require-plugin")
assert (it.get_version () == "1.0")
assert (it.is_optional ())

it = plugin.get_extpoints ()[0]
assert (it.get_local_id () == "extpoint1")
assert (it.get_name () == "hello")
assert (it.get_id () == "test.plugin.extpoint1")
assert (it.query_plugin () == plugin)

it = plugin.get_extensions ()[0]
assert (it.get_local_id () == "extension1")
assert (it.get_name () == "world")
assert (it.get_extpoint_id () == "test.plugin.extpoint1")
assert (it.get_id () == "test.plugin.extension1")
assert (it.query_plugin () == plugin)
