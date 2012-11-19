from gi.repository import Gimo

def plugin_start (plugin):
    context = plugin.query_context ()
    Gimo.bind_string (context, "py_start", "py_start");
    return True

def plugin_stop (plugin):
    context = plugin.query_context ()
    Gimo.bind_string (context, "py_stop", "py_stop");
    return True

def symbol1 (plugin):
    assert (plugin.get_id () == "org.gimo.test.plugin2")
    plugin.connect ("start", plugin_start)
    plugin.connect ("stop", plugin_stop)
