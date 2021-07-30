import math
import time
import numpy as np
import pandas as pd
import latticex.rosetta as rtt
import tensorflow as tf


a = tf.constant([1])
b = tf.Variable([2])
c = tf.constant([3])

d = a + b
e = d + c


#-----------------------------------------------------------------------------
# for test
from latticex.rosetta.secure import StaticReplacePass
from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts

PassObj = StaticReplacePass()
if isinstance(e, rtt_ts.RttTensor):
    e = PassObj.run(e._raw)
else:
    e = PassObj.run(e)
#------------------------------------------------------------------------------


Writer = tf.summary.FileWriter("log/opt_need_secure_flow", tf.get_default_graph())
Writer.close()
