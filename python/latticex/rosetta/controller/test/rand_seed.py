#!/usr/bin/env python3

from latticex.rosetta.controller.backend_handler import py_protocol_handler
import latticex.rosetta as rtt
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)


print("before activate rand_seed:", rtt.random_seed.get_seed())

protocol = "Helix"
rtt.activate(protocol)

print("py_protocol_handler.rand_seed:", py_protocol_handler.rand_seed(0))
print("after activate rand_seed:", rtt.random_seed.get_seed())

rtt.deactivate()

print("after deactivate rand_seed:", rtt.random_seed.get_seed())

