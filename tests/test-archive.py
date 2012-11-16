#!/usr/bin/env python

import os
from gi.repository import Gimo

# Common
ar = Gimo.Archive ()
obj = Gimo.Archive ()
assert (len (ar.query_objects ()) == 0)
assert (not ar.query_object ("hello"))
assert (ar.add_object ("hello", obj))
assert (ar.query_object ("hello"))
assert (not ar.query_object (""))
assert (len (ar.query_objects ()) == 1)
ar.remove_object ("hello")
assert (not ar.query_object ("hello"))
assert (len (ar.query_objects ()) == 0)

# Xml read
archive = Gimo.XmlArchive ()
assert (archive.read ("demo-archive2.xml"))

info = archive.query_object ("plugin1")
assert (info)
assert (info.get_id () == "org.gimo.test.plugin1")
assert (info.get_name () == "test plugin1")
assert (info.get_version () == "0.1")
assert (info.get_provider () == "tomnotcat")
assert (info.get_module () == "plugin1")
assert (info.get_symbol () == "symbol1")
assert (len (info.get_requires ()) == 0)
assert (len (info.get_extpoints ()) == 1)
assert (len (info.get_extensions ()) == 0)
extpt = info.get_extpoint ("extpt1");
assert (extpt)
assert (extpt.get_name () == "extension point 1")

info = archive.query_object ("plugin2")
assert (info)
assert (info.get_id () == "org.gimo.test.plugin2")
assert (len (info.get_requires ()) == 1)
assert (len (info.get_extpoints ()) == 0)
assert (len (info.get_extensions ()) == 1)
require = info.get_requires ()[0];
assert (require.get_plugin_id () == "org.gimo.test.plugin1")
assert (require.get_version () == "1.0")
assert (require.is_optional ())
ext = info.get_extension ("ext1");
assert (ext);
assert (ext.get_name () == "extension 1")
assert (ext.get_extpoint_id () == "org.gimo.test.plugin1.extpt1")
assert (len (ext.get_configs ()) == 2)
assert (ext.get_config ("config1").get_value () == "value1")
assert (ext.get_config ("config2").get_value () == "value2")

assert (not archive.query_object ("org.gimo.test.plugin2"))
