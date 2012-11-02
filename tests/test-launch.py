#!/usr/bin/env python

import os

launch = "../tools/gimo-launch "
output = os.popen(launch).read()
output = os.popen(launch + "--start a --start b").read()

print output
