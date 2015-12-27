#!/usr/bin/env python3

import os
import sys

sys.path.append(os.path.join(os.path.abspath(os.path.dirname(__file__)), 'py'))

from potatocache import PotatoCache

PotatoCache()
