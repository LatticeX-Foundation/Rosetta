
import matplotlib.pyplot as plt
import os
import sys
import csv
import tensorflow as tf
import numpy as np
from util import score_linear_regression, score_logistic_regression, pretty
import pandas as pd
import argparse


pd.set_option('display.width', 1000)
np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)


# ################################################## Args
parser = argparse.ArgumentParser(description="Model Metrixs")
parser.add_argument('--sname', type=str, help="script name", required=True)
parser.add_argument('--model', type=str, help="model", default='logistic')
args, unparsed = parser.parse_known_args()
# ##################################################

csvprefix = "./log/"
tfcsvprefix = "./log/tf-" + args.sname + "/tf"
rttcsvprefix = "./log/rtt-" + args.sname + "/rtt"

y_real_file = tfcsvprefix + "-real-Y.csv"
y_pred_file_tf = tfcsvprefix + "-pred-Y.csv"
y_pred_file_rtt = rttcsvprefix + "-pred-Y.csv"

Y = pd.read_csv(y_real_file, sep=',', header=None, names=['L'])
predYtf = pd.read_csv(y_pred_file_tf, sep=',', header=None, names=['L'])
predYrtt = pd.read_csv(y_pred_file_rtt, sep=',', header=None, names=['L'])

print(args.model)
if args.model == 'logistic':
    emetrixs = score_logistic_regression(
        predYtf.to_numpy(), Y.to_numpy(), tag='tensorflow')
    print(pretty(emetrixs))

    emetrixs = score_logistic_regression(
        predYrtt.to_numpy(), Y.to_numpy(), tag='rosetta')
    print(pretty(emetrixs))

    # hist
    plt.title("tensorflow rosetta")
    plt.xlabel("Probability")
    plt.ylabel("Frequency")
    plt.hist(predYtf.to_numpy(), bins=50, label="TensorFlow")
    plt.hist(predYrtt.to_numpy(), bins=50, label="Rosetta")
    plt.legend()
    plt.savefig(csvprefix+"/" + args.sname + "-hist.png")
    plt.clf()


if args.model == 'linear':
    emetrixs = score_linear_regression(
        predYtf.to_numpy(), Y.to_numpy(), tag='tensorflow')
    print(pretty(emetrixs))

    emetrixs = score_linear_regression(
        predYrtt.to_numpy(), Y.to_numpy(), tag='rosetta')
    print(pretty(emetrixs))
