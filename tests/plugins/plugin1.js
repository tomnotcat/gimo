const Gimo = imports.gi.Gimo;
const assert = imports.jsUnit.assert;

function plugin_start (plugin)
{
    var context = plugin.query_context ();
    var loader = context.resolve_extpoint ("org.gimo.core.loader.module");
    var module = loader.load ("sub-plugin.js");

    assert (module != null);
    assert (module.resolve ("test_plugin_new", null, true) != null);
    assert (module.resolve ("test_plugin_new2", null, true) == null);

    Gimo.bind_string (context, "js_start", "js_start");
    return true;
}

function plugin_run (plugin)
{
    var context = plugin.query_context ();
    Gimo.bind_string (context, "js_run", "js_run");
}

function plugin_stop (plugin)
{
    var context = plugin.query_context ();
    Gimo.bind_string (context, "js_stop", "js_stop");
}

function symbol1 (plugin)
{
    assert (plugin.get_id () == "org.gimo.test.plugin1");
    plugin.connect ("start", plugin_start);
    plugin.connect ("run", plugin_run);
    plugin.connect ("stop", plugin_stop);
    return plugin;
}