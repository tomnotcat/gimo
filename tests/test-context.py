#!/usr/bin/env python

from gi.repository import Gimo

g_old_state = Gimo.PluginState.UNINSTALLED
g_new_state = Gimo.PluginState.UNINSTALLED
g_state_count = 0

def _plugin_state_changed (ctx, info, old_state, new_state):
    global g_old_state
    global g_new_state
    global g_state_count

    g_old_state = old_state
    g_new_state = new_state
    g_state_count += 1

ctx = Gimo.Context ()
ctx.connect ("state-changed", _plugin_state_changed)

info = Gimo.Pluginfo (identifier="test.plugin")
assert (not ctx.query_plugin ("test.plugin"))
status = ctx.install_plugin (info)
assert (ctx.query_plugin ("test.plugin") == info)
assert (status == Gimo.Status.SUCCESS)

assert (Gimo.PluginState.UNINSTALLED == g_old_state)
assert (Gimo.PluginState.INSTALLED == g_new_state)
assert (1 == g_state_count)

info = Gimo.Pluginfo (identifier="test.plugin")
status = ctx.install_plugin (info)
assert (status == Gimo.Status.CONFLICT)
info = Gimo.Pluginfo (identifier="test.plugin2")
assert (info.query_context () == None)
status = ctx.install_plugin (info)
assert (Gimo.PluginState.UNINSTALLED == g_old_state)
assert (Gimo.PluginState.INSTALLED == g_new_state)
assert (2 == g_state_count)

assert (ctx.query_plugin ("test.plugin2") == info)
assert (info.query_context () == ctx)

array = ctx.query_plugins ("hello")
assert (len (array) == 0)

array = ctx.query_plugins (None)
assert (len (array) == 2)

status = ctx.uninstall_plugin ("test.plugin2");
assert (not ctx.query_plugin ("test.plugin2"))
assert (info.query_context () == None)
assert (ctx.query_plugin ("test.plugin"))
assert (status == Gimo.Status.SUCCESS)
assert (Gimo.PluginState.INSTALLED == g_old_state)
assert (Gimo.PluginState.UNINSTALLED == g_new_state)
assert (3 == g_state_count)
