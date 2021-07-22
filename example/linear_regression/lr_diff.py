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


# ################################################## Args
parser = argparse.ArgumentParser(description="Predict Diff")
parser.add_argument('--sname', type=str, help="script name", required=True)
parser.add_argument('--epochs', type=int, help="epochs", default=10)
args, unparsed = parser.parse_known_args()
# ##################################################
EPOCHES = args.epochs

learning_rate = 0.0002
BATCH_SIZE = 5
DIS_STEP = 5
DIM_NUM = 5


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


csvprefix = "./log/" + args.sname
tfcsvprefix = csvprefix + "/tf"
rttcsvprefix = csvprefix + "/rtt"
columns = ['epoch', 'tf', 'rtt', 'tf-rtt', '(tf-rtt)/rtt', 'tf/rtt']

# weights & biases
diffs = None
wdiffs = None
wdiffsavg = None
bdiffs = None
for i in range(EPOCHES):
    if i % 10 == 0 or (i == EPOCHES-1 and i % 10 != 0):
        tf_file = "{}-{:0>4d}-{}.csv".format(tfcsvprefix, i, "W")
        rtt_file = "{}-{:0>4d}-{}.csv".format(rttcsvprefix, i, "W")
        weights_diff = cope_diff(tf_file, rtt_file)

        tf_file = "{}-{:0>4d}-{}.csv".format(tfcsvprefix, i, "b")
        rtt_file = "{}-{:0>4d}-{}.csv".format(rttcsvprefix, i, "b")
        biases_diff = cope_diff(tf_file, rtt_file)

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
            wdiffsavg = np.vstack((wdiffsavg, np.mean(weights_diff, axis=0)))

#print(diffs[-DIM_NUM-1:, :])
df = pd.DataFrame(diffs, columns=columns)
df.to_csv(csvprefix+"-Wb.csv", index=False, float_format="%.10f")

# predictions
tf_file = "{}-{:0>4d}-{}.csv".format(tfcsvprefix, EPOCHES, "Y")
rtt_file = "{}-{:0>4d}-{}.csv".format(rttcsvprefix, EPOCHES, "Y")
pred_diff = cope_diff(tf_file, rtt_file)

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
