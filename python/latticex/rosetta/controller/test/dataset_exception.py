#!/usr/bin/env python3

import random
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

# ##############################
"""
Exception tests (Usages && 28 Cases)
SampleAligned:   test cases (data owner) * (label owner)
     data owner: P0, P1, P2, P0/P1, P0/P2, P1/P2, P0/P1/P2
    label owner: P0, P1, P2
FeatureAligned:  test cases (data owner) == (label owner)
     data owner: P0, P1, P2, P0/P1, P0/P2, P1/P2, P0/P1/P2
    label owner: P0, P1, P2, P0/P1, P0/P2, P1/P2, P0/P1/P2
"""
# ##############################

"""
All (maybe) exception case - SampleAligned
"""
print("All (maybe) exception case - SampleAligned")

print("All (maybe) exception case - SampleAligned - [construct data (X,Y)]")
n = 5
d = (partyid+1)*3 - partyid//2
XX = np.array(range(0, n*d)).reshape([n, d])
print("X.shape:", XX.shape)
YY = np.array(range(0, n)).reshape([n, 1])
print("Y.shape:", YY.shape)


def get_X0(data_owner):
    if random.randint(1, 10) > 5:
        return XX
    return None


def get_Y0(label_owner):
    if random.randint(1, 10) > 5:
        return YY
    return None


def TEST0(data_owner, label_owner):
    print("All (maybe) exception case - SampleAligned - [data_owner={}, label_owner={}]".format(
        data_owner, label_owner))

    dset = rtt.PrivateDataset(data_owner=data_owner, label_owner=label_owner)
    try:
        res = dset.load_X(get_X0(data_owner))
        print("load_X res.shape:", res.shape)
        print("load_X res:", res)
    except Exception as e:
        print(e)

    dset = rtt.PrivateDataset(data_owner=data_owner, label_owner=label_owner)
    try:
        res = dset.load_y(get_Y0(label_owner))
        print("load_y res.shape:", res.shape)
        print("load_y res:", res)
    except Exception as e:
        print(e)

    dset = rtt.PrivateDataset(data_owner=data_owner, label_owner=label_owner)
    try:
        XX_ = get_X0(data_owner)
        YY_ = get_Y0(label_owner)
        resX, resy = dset.load_data(XX_, YY_)
        print("load_data resX.shape:", resX.shape)
        print("load_data resX:", resX)
        print("load_data resy.shape:", resy.shape)
        print("load_data resy:", resy)
    except Exception as e:
        print(e)


"""
SampleAligned:   test cases (data owner) * (label owner)
     data owner: P0, P1, P2, P0/P1, P0/P2, P1/P2, P0/P1/P2
    label owner: P0, P1, P2
"""
TEST0((0,), 0)
TEST0((0,), 1)
TEST0((0,), 2)
TEST0((1,), 0)
TEST0((1,), 1)
TEST0((1,), 2)
TEST0((2,), 0)
TEST0((2,), 1)
TEST0((2,), 2)

TEST0((0, 1), 0)
TEST0((0, 1), 1)
TEST0((0, 1), 2)
TEST0((0, 2), 0)
TEST0((0, 2), 1)
TEST0((0, 2), 2)
TEST0((1, 2), 0)
TEST0((1, 2), 1)
TEST0((1, 2), 2)

TEST0((0, 1, 2), 0)
TEST0((0, 1, 2), 1)
TEST0((0, 1, 2), 2)


"""
All (maybe) exception case - FeatureAligned
"""
print("All (maybe) exception case - FeatureAligned")

print("All (maybe) exception case - FeatureAligned - [construct data (X,Y)]")
d = 5
n = (partyid+1)*3 - partyid//2
XX = np.array(range(0, n*d)).reshape([n, d])
print("XX.shape:", XX.shape)
YY = np.array(range(0, n)).reshape([n, 1])
print("YY.shape:", YY.shape)


def get_X1(data_owner):
    if random.randint(1, 10) > 5:
        return XX
    return None


def get_Y1(data_owner):
    if random.randint(1, 10) > 5:
        return YY
    return None


def TEST1(data_owner):
    print("All (maybe) exception case - FeatureAligned - [data_owner={}]".format(
        data_owner))

    # note, when dataset_type is tt.DatasetType.FeatureAligned, ignore label_owner
    label_owner = random.randint(0, 2)
    dset = rtt.PrivateDataset(data_owner=data_owner, label_owner=label_owner,
                              dataset_type=rtt.DatasetType.FeatureAligned)
    try:
        res = dset.load_X(get_X1(data_owner))
        print("load_X res.shape:", res.shape)
        print("load_X res:", res)
    except Exception as e:
        print(e)

    try:
        res = dset.load_y(get_Y1(data_owner))
        print("load_y res.shape:", res.shape)
        print("load_y res:", res)
    except Exception as e:
        print(e)

    try:
        XX_ = get_X1(data_owner)
        YY_ = get_Y1(data_owner)
        resX, resy = dset.load_data(XX_, YY_)
        print("load_data resX.shape:", resX.shape)
        print("load_data resX:", resX)
        print("load_data resy.shape:", resy.shape)
        print("load_data resy:", resy)
    except Exception as e:
        print(e)


"""
FeatureAligned:  test cases (data owner) == (label owner)
     data owner: P0, P1, P2, P0/P1, P0/P2, P1/P2, P0/P1/P2
    label owner: P0, P1, P2, P0/P1, P0/P2, P1/P2, P0/P1/P2
"""
TEST1((0,))
TEST1((1,))
TEST1((2,))
TEST1((0, 1))
TEST1((0, 2))
TEST1((1, 2))
TEST1((0, 1, 2))


print(rtt.get_perf_stats(True))
rtt.deactivate()
