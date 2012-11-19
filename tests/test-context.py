#!/usr/bin/env python

from gi.repository import Gimo

g_old_state = Gimo.PluginState.UNINSTALLED
g_new_state = Gimo.PluginState.UNINSTALLED
g_state_count = 0

def _plugin_state_changed (context, plugin, old_state, new_state):
    global g_old_state
    global g_new_state
    global g_state_count

    g_old_state = old_state
    g_new_state = new_state
    g_state_count += 1

context = Gimo.Context ()
context.connect ("state-changed", _plugin_state_changed)

plugin = Gimo.Plugin (id="test.plugin", path="sub")
assert (not context.query_plugin ("test.plugin"))
assert (context.install_plugin ("/root", plugin))
assert (context.query_plugin ("test.plugin") == plugin)
assert (plugin.get_path () == "/root/sub")

assert (Gimo.PluginState.UNINSTALLED == g_old_state)
assert (Gimo.PluginState.INSTALLED == g_new_state)
assert (1 == g_state_count)

plugin = Gimo.Plugin (id="test.plugin")
assert (not context.install_plugin (None, plugin))
assert (Gimo.get_error () == Gimo.Errors.CONFLICT)

extpts = [Gimo.ExtPoint (id="extpt1"),
          Gimo.ExtPoint (id="extpt2")]
plugin = Gimo.Plugin.new ("test.plugin2",
                          "plugin2",
                          "1.0",
                          "tomnotcat",
                          None,
                          None,
                          None,
                          None,
                          extpts,
                          None)

assert (plugin.query_context () == None)
assert (context.install_plugin (None, plugin))
assert (Gimo.PluginState.UNINSTALLED == g_old_state)
assert (Gimo.PluginState.INSTALLED == g_new_state)
assert (2 == g_state_count)

assert (context.query_plugin ("test.plugin2") == plugin)
assert (plugin.query_context () == context)

# With core plugins.
array = context.query_plugins ()
assert (len (array) > 2)

assert (context.query_extpoint ("test.plugin2.extpt1") == extpts[0])
assert (context.query_extpoint ("test.plugin2.extpt2") == extpts[1])
assert (context.query_extpoint ("test.plugin2.extpt3") == None)

context.uninstall_plugin ("test.plugin2");
assert (not context.query_plugin ("test.plugin2"))
assert (plugin.query_context () == None)
assert (context.query_plugin ("test.plugin"))
assert (context.query_extpoint ("test.plugin2.extpt1") == None)
assert (context.query_extpoint ("test.plugin2.extpt2") == None)
assert (Gimo.PluginState.INSTALLED == g_old_state)
assert (Gimo.PluginState.UNINSTALLED == g_new_state)
assert (3 == g_state_count)
