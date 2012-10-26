#!/usr/bin/env python

from gi.repository import Gimo

ar = Gimo.Archive ()
obj = Gimo.Archive ()
assert (not ar.query_object ("hello"))
assert (ar.add_object ("hello", obj))
assert (ar.query_object ("hello"))
assert (not ar.query_object (""))
ar.remove_object ("hello")
assert (not ar.query_object ("hello"))
