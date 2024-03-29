from gi.repository import Gimo

def plugin_start (plugin):
    context = plugin.query_context ()
    loader = context.resolve_extpoint ("org.gimo.core.loader.module")
    module = loader.load ("demo-plugin.py")
    assert (module.resolve ("test_plugin_new", None) != None)
    Gimo.bind_string (context, "py_start", "py_start")
    return True

def plugin_run (plugin):
    context = plugin.query_context ()
    Gimo.bind_string (context, "py_run", "py_run")

def plugin_stop (plugin):
    context = plugin.query_context ()
    Gimo.bind_string (context, "py_stop", "py_stop")

def plugin_save (plugin, store):
    store.set ("save", 100)

def plugin_restore (plugin, store):
    if store.get ("save") == 100:
        context = plugin.query_context ()
        Gimo.bind_string (context, "py_update", "py_update")

def symbol1 (plugin):
    assert (plugin.get_id () == "org.gimo.test.plugin2")
    plugin.connect ("start", plugin_start)
    plugin.connect ("run", plugin_run)
    plugin.connect ("stop", plugin_stop)
    plugin.connect ("save", plugin_save)
    plugin.connect ("restore", plugin_restore)
    return Gimo.Plugin ()
