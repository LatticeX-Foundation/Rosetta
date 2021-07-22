#!/usr/bin/env python3

import latticex.rosetta as rtt
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)


protocol = "Helix"
protocol = "SecureNN"
rtt.activate(protocol)
rtt.backend_log_to_stdout(True)
partyid = rtt.get_party_id()

dset = rtt.PrivateDataset(data_owner=(0, 1), label_owner=0)
resX, resy = dset.load_data(
    "./testdataset/p0_attr_plain.csv", "./testdataset/p0_label_plain.csv")


dset = rtt.PrivateDataset(data_owner=(0, 2), label_owner=0)
resX, resy = dset.load_data(
    "./testdataset/p1_attr_plain.csv", "./testdataset/p0_label_plain.csv")


print(rtt.get_perf_stats(True))
rtt.deactivate()
