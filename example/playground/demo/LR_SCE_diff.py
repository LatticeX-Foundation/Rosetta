import matplotlib.pyplot as plt
import os
import sys
import csv
import tensorflow as tf
import numpy as np
from util import read_dataset, savecsv, loadcsv
import pandas as pd
import argparse

np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)

DIM_NUM = 2
# ################################################## Args
parser = argparse.ArgumentParser(description="MPC LR with SCE Loss")
parser.add_argument('--dim', type=int, help="dim", default=2)
parser.add_argument('--epochs', type=int, help="epochs", default=100)
args, unparsed = parser.parse_known_args()
# ##################################################
EPOCHES = args.epochs
DIM_NUM = args.dim

learning_rate = 0.1
BATCH_SIZE = 5
DIS_STEP = 2

def cope_diff(tf_file, rtt_file):
    val_tf = loadcsv(tf_file).reshape(-1, 1)
    val_rtt = loadcsv(rtt_file).reshape(-1, 1)
    #bad usage of i
    epoch_index = np.full(val_tf.shape, i+1)
    val_tf[abs(val_tf) < 1e-5] = 1e-5
    diff_div = val_rtt/val_tf
    diff_sub = val_tf - val_rtt
    #if val_tf == 0:
    #    diff_percent = diff_sub
    #    diff_div = val_rtt
    #else:
    diff_percent = (val_tf - val_rtt) / val_tf
    diff_div = val_rtt/val_tf
    cc = np.hstack((epoch_index, val_tf, val_rtt,
                    diff_sub, diff_percent, diff_div))
    return cc

csvprefix = "./comp_log/diff"
tfcsvprefix = "./comp_log/native"
rttcsvprefix ="./comp_log/mpc"
columns = ['epoch', 'Native TF', 'MPC', 'TF-MPC', '(TF-MPC)/TF', 'MPC/TF']

# weights & biases
diffs = None
wdiffs = None
wdiffsavg = None
bdiffs = None
for i in range(EPOCHES):
    if (i + 1) % DIS_STEP == 0:
        tf_file = "{}-{:0>4d}-{}.csv".format(tfcsvprefix, i+1, "W")
        rtt_file = "{}-{:0>4d}-{}.csv".format(rttcsvprefix, i+1, "W")
        weights_diff = cope_diff(tf_file, rtt_file)
        tf_file = "{}-{:0>4d}-{}.csv".format(tfcsvprefix, i+1, "b")
        rtt_file = "{}-{:0>4d}-{}.csv".format(rttcsvprefix, i+1, "b")
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

df = pd.DataFrame(diffs, columns=columns)
df.to_csv(csvprefix+"-Wb.csv", index=False, float_format="%.10f")

# predictions
tf_file = "{}-{:0>4d}-{}.csv".format(tfcsvprefix, EPOCHES, "Y")
rtt_file = "{}-{:0>4d}-{}.csv".format(rttcsvprefix, EPOCHES, "Y")
pred_diff = cope_diff(tf_file, rtt_file)

df = pd.DataFrame(pred_diff, columns=columns)
df.to_csv(csvprefix+"-Y.csv", index=False, float_format="%.10f")

config_str = "config: [learn rate:" + str(learning_rate) + " attr nums:" + str(DIM_NUM) + " Epoch:" + str(EPOCHES) + "]"

total_iter = DIM_NUM * EPOCHES / DIS_STEP
common_x = [ 2*(j+1) for j in range(int(EPOCHES/DIS_STEP))]

# plot
plt.title("weights absolute error\n"+ config_str)
plt.xlabel("Epoch")
plt.ylabel("(tf-rtt)")
#300 = 5 * 120 / 2 
for i in range(DIM_NUM):
    #this_dim = []
    #for j in range(EPOCHES):
    this_dim = [wdiffs[:, 3][it*DIM_NUM + i] for it in range(int(EPOCHES/DIS_STEP))]
    plt.plot(common_x, this_dim, label = str(i) + "-th attr")
    # mark last point
    plt.text(common_x[-1], this_dim[-1], ("{0:.5f}".format(float(this_dim[-1]))),
                ha='center', va='bottom')
plt.legend(loc="best")
plt.savefig(csvprefix+"-W-diff.png")
plt.clf()

plt.title("weights‘ comparative error curve\n"+ 
            "config: [learn rate:"+ str(learning_rate) + " attr nums:" +
            str(DIM_NUM) + " Epoch:" + str(EPOCHES) + "]")
plt.xlabel("Epoch")
plt.ylabel("(tf-rtt)/rtt")
#300 = 5 * 120 / 2 
for i in range(DIM_NUM):
    #this_dim = []
    #for j in range(EPOCHES):
    this_dim = [wdiffs[:, 4][it*DIM_NUM + i] for it in range(int(EPOCHES/DIS_STEP))]
    plt.plot(common_x, this_dim, label = str(i) + "-th attr")
    # mark last point
    plt.text(common_x[-1], this_dim[-1], ("{0:.5f}".format(float(this_dim[-1]))),
                ha='center', va='bottom')
plt.legend(loc="best")
plt.savefig(csvprefix+"-W-CE.png")
plt.clf()

plt.title("bias’ absolute error\n"+ config_str)
plt.ylabel("tf-rtt")
plt.xlabel("Epoch")
plt.plot(common_x, bdiffs[:, 3])
# mark last point
plt.text(common_x[-1], bdiffs[:, 3][-1], ("{0:.5f}".format(float(bdiffs[:, 3][-1]))),
            ha='center', va='bottom')
plt.savefig(csvprefix+"-b-diff.png")
plt.clf()

plt.title("bias' comparative error\n" + config_str)
plt.xlabel("Epoch")
plt.ylabel("(tf-rtt)/rtt")
plt.plot(common_x, bdiffs[:, 4])
# mark last point
plt.text(common_x[-1], bdiffs[:, 4][-1], ("{0:.5f}".format(float(bdiffs[:, 4][-1]))),
            ha='center', va='bottom')
plt.savefig(csvprefix+"-b-CE.png")
plt.clf()

plt.title("Class labels' absolute error\n" + config_str)
plt.ylabel("tf-rtt")
plt.xlabel("training samples")
plt.plot(range(1,len(pred_diff[:, 3])+1), pred_diff[:, 3])
# mark last point
plt.text(len(pred_diff[:, 3]), pred_diff[:, 3][-1], ("{0:.5f}".format(float(pred_diff[:, 3][-1]))),
            ha='center', va='bottom')
plt.savefig(csvprefix+"-Y-diff.png")
plt.clf()

