#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8
#
# Copyright © 2014 vfeenstr <vfeenstr@dhcp130-229.brq.redhat.com>
#
# Distributed under terms of the MIT license.

import foo as bar
from bar2 import baz as blah
from bar3 import baz
from ... import blahblub as blubblah
from ...something import blahblub as blubblah2
from blu.bla.bli import foo as blublabli
import blu.bla.bli as totally_different

@property(abc)
@property
def foobar():
    from sys import *

