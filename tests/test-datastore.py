#!/usr/bin/env python

from gi.repository import Gimo

store = Gimo.DataStore ()
assert (store.get ("test") == None)
store.set ("test", 1)
assert (store.get ("test") == 1)
store.set ("test", "hello")
assert (store.get ("test") == "hello")

data = Gimo.DataStore ()
data.set ("test", "world")
store.set ("test", data)
assert (store.get ("test") == data)
assert (store.get ("test") != "hello")
assert (store.get ("test").get ("test") == "world")
