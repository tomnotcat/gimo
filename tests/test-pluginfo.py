#!/usr/bin/env python

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

info = Gimo.Pluginfo (requires=None)
assert (info.get_requires () == None)
