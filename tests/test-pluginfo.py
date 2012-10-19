#!/usr/bin/env python

from gi.repository import GLib
from gi.repository import Gimo

info = Gimo.Pluginfo ()
assert (info.get_identifier () == None)
assert (info.get_url () == None)
assert (info.get_klass () == None)
assert (info.get_name () == None)
assert (info.get_version () == None)
assert (info.get_provider () == None)
assert (len (info.get_requires ()) == 0)
assert (len (info.get_extpoints ()) == 0)
assert (len (info.get_extensions ()) == 0)

requires = [Gimo.Require ()]
extpoints = []
extensions = []
info = Gimo.Pluginfo.new ("test.plugin",
                          "/home/test",
                          "myklass",
                          "myname",
                          "1.0",
                          "tomnotcat",
                          requires,
                          extpoints,
                          extensions)
assert (info.get_identifier () == "test.plugin")
assert (info.get_url () == "/home/test")
assert (info.get_klass () == "myklass")
assert (info.get_name () == "myname")
assert (info.get_version () == "1.0")
assert (info.get_provider () == "tomnotcat")
assert (len (info.get_requires ()) == len (requires))
assert (len (info.get_extpoints ()) == len (extpoints))
assert (len (info.get_extensions ()) == len (extensions))
