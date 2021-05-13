
import re
import matplotlib.pyplot as plt
import os
import sys
import csv
import tensorflow as tf
import numpy as np
from util import read_dataset, savecsv, loadcsv
import pandas as pd
import argparse


pd.set_option('display.width', 1000)
np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)


def getids(rootdir):
    lists = os.listdir(rootdir)
    ids = []
    for i in range(0, len(lists)):
        path = os.path.join(rootdir, lists[i])
        if os.path.isdir(path):
            continue

        filename = lists[i]
        m = re.match(r'(tf|rtt)-(\d+|real|pred)-(W|Y|b).csv', filename)
        if m is None:
            continue

        if m[1] == 'tf' and m[3] == 'W':
            ids.append(m[2])

    ids.sort()
    return ids


def cope_diff(tf_file, rtt_file):
    val_tf = loadcsv(tf_file).reshape(-1, 1)
    val_rtt = loadcsv(rtt_file).reshape(-1, 1)
    val_rtt[abs(val_rtt) < 1e-5] = 1e-5
    epoch_index = np.full(val_tf.shape, i)
    diff_div = val_tf/val_rtt
    diff_sub = val_tf - val_rtt
    diff_percent = (val_tf - val_rtt) / val_rtt
    cc = np.hstack((epoch_index, val_tf, val_rtt,
                    diff_sub, diff_percent, diff_div))
    return cc


# ################################################## Args
parser = argparse.ArgumentParser(description="Model Plot")
parser.add_argument('--sname', type=str, help="script name", required=True)
args, unparsed = parser.parse_known_args()
# ##################################################
csvprefix = "./log/" + args.sname
tfdir = "./log/tf-" + args.sname
rttdir = "./log/rtt-" + args.sname
ids = getids(tfdir)

tf_real = tfdir + '/tf-real-Y.csv'
tf_pred = tfdir + '/tf-pred-Y.csv'
rt_pred = rttdir + '/rtt-pred-Y.csv'

tf_W = []
tf_b = []
rt_W = []
rt_b = []
for id_ in ids:
    tf_W.append('{}/tf-{}-W.csv'.format(tfdir, id_))
    tf_b.append('{}/tf-{}-b.csv'.format(tfdir, id_))
    rt_W.append('{}/rtt-{}-W.csv'.format(rttdir, id_))
    rt_b.append('{}/rtt-{}-b.csv'.format(rttdir, id_))

# #### weights & biases
diffs = None
wdiffs = None
wdiffsavg = None
bdiffs = None

for i in range(len(ids)):
    weights_diff = cope_diff(tf_W[i], rt_W[i])
    biases_diff = cope_diff(tf_b[i], rt_b[i])
    diff = np.vstack((weights_diff, biases_diff))

    if diffs is None:
        diffs = diff
        wdiffs = weights_diff
        bdiffs = biases_diff
        wdiffsavg = np.mean(weights_diff, axis=0)
    else:
        diffs = np.vstack((diffs, diff))
        wdiffs = np.vstack((wdiffs, weights_diff))
        bdiffs = np.vstack((bdiffs, biases_diff))
        wdiffsavg = np.vstack(
            (wdiffsavg, np.mean(weights_diff, axis=0)))

# #### save
csvprefix = "./log/" + args.sname
columns = ['epoch', 'tf', 'rtt', 'tf-rtt', '(tf-rtt)/rtt', 'tf/rtt']

# print(diffs[-DIM_NUM-1:, :])
df = pd.DataFrame(diffs, columns=columns)
df.to_csv(csvprefix+"-Wb.csv", index=False, float_format="%.10f")

# predictions
pred_diff = cope_diff(tf_pred, rt_pred)

df = pd.DataFrame(pred_diff, columns=columns)
df.to_csv(csvprefix+"-Y.csv", index=False, float_format="%.10f")


# plot
plt.title("weights tf-rtt")
plt.xlabel("iterations")
plt.plot(wdiffs[:, 3])
plt.savefig(csvprefix+"-W-diff.png")
plt.clf()

plt.title("weights (tf-rtt)/rtt")
plt.xlabel("iterations")
plt.plot(wdiffs[:, 4])
plt.savefig(csvprefix+"-W-diff4.png")
plt.clf()

plt.title("weights tf/rtt")
plt.xlabel("iterations")
plt.plot(wdiffs[:, 5])
plt.savefig(csvprefix+"-W-diff5.png")
plt.clf()

# plot
plt.title("bias tf-rtt")
plt.xlabel("iterations")
plt.plot(bdiffs[:, 3])
plt.savefig(csvprefix+"-b-diff.png")
plt.clf()

plt.title("bias (tf-rtt)/rtt")
plt.xlabel("iterations")
plt.plot(bdiffs[:, 4])
plt.savefig(csvprefix+"-b-diff4.png")
plt.clf()

plt.title("bias tf/rtt")
plt.xlabel("iterations")
plt.plot(bdiffs[:, 5])
plt.savefig(csvprefix+"-b-diff5.png")
plt.clf()

# plot
plt.title("predict tf-rtt")
plt.xlabel("samples")
plt.plot(pred_diff[:, 3])
plt.savefig(csvprefix+"-Y-diff.png")
plt.clf()

plt.title("predict (tf-rtt)/rtt")
plt.xlabel("samples")
plt.plot(pred_diff[:, 4])
plt.savefig(csvprefix+"-Y-diff4.png")
plt.clf()

plt.title("predict tf/rtt")
plt.xlabel("samples")
plt.plot(pred_diff[:, 5])
plt.savefig(csvprefix+"-Y-diff5.png")
plt.clf()
