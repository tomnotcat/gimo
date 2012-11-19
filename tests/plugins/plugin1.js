const Gimo = imports.gi.Gimo;
const assert = imports.jsUnit.assert;

function plugin_start (plugin)
{
    var context = plugin.query_context ();
    Gimo.bind_string (context, "js_start", "js_start");
    return true;
}

function plugin_stop (plugin)
{
    var context = plugin.query_context ();
    Gimo.bind_string (context, "js_stop", "js_stop");
    return true;
}

function symbol1 (plugin)
{
    assert (plugin.get_id () == "org.gimo.test.plugin1");
    plugin.connect ("start", plugin_start);
    plugin.connect ("stop", plugin_stop);
}