""" 
Show case of native TF logistic regrssion with sigmoid cross entropy loss function
"""
import time

import csv
import tensorflow as tf
import numpy as np

np.set_printoptions(suppress=True)

from util import read_dataset, savecsv
import pandas as pd
import argparse
import os
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)

#import latticex.rosetta as rstt
# ################################################## Args
parser = argparse.ArgumentParser(description="Rosetta Linear Regression")
parser.add_argument('--dims', type=int, help="dims", default=2)
parser.add_argument('--epochs', type=int, help="epochs", default=100)
args, unparsed = parser.parse_known_args()
# ##################################################
EPOCHES = args.epochs
DIM_NUM = args.dims

learning_rate = 0.1
#EPOCHES = 100
BATCH_SIZE = 5
DIS_STEP = 2

#DIM_NUM = 2
# n-D
X = tf.placeholder(tf.float64, [None, DIM_NUM])
# binary classes [0, 1]
Y = tf.placeholder(tf.float64, [None, 1])

W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64), dtype = tf.float64)
b = tf.Variable(tf.zeros([1], dtype=tf.float64), dtype = tf.float64)
logits = tf.matmul(X, W) + b
sig_v = tf.sigmoid(logits)
#loss = tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(labels = Y,
                                            #    logits = logits,
                                            #    name="native_sce"))

loss = tf.reduce_mean(-Y*tf.log(sig_v) -(1 - Y) * tf.log(1 - sig_v))
optimizer = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)

init = tf.global_variables_initializer()
# def read_dataset(file_name = None):
#     if file_name is None:
#         print("Error! No file name!")
#         return
#     res_data = []
#     with open(file_name, 'r') as f:
#         cr = csv.reader(f)
#         for each_r in cr:
#             curr_r = [np.array([v], dtype=np.float_)[0] for v in each_r]
#             res_data.append(curr_r)
#             #print(each_r)
#     return res_data

file_name_prefix = str("../datasets/") + str(DIM_NUM) + "D/" + str(DIM_NUM) + "d"
plain_attr_ds = read_dataset(file_name_prefix + "_attr_plain.csv")
plain_label_ds = read_dataset(file_name_prefix + "_label_plain.csv")

save_file_prefix = "./comp_log/native"

with tf.Session() as native_sess:
    native_sess.run(init)
    xW, xb = native_sess.run([W, b])
    print("init weight:{} \nbias:{}".format(xW, xb))

    total_batch = int(len(plain_attr_ds) / BATCH_SIZE)
    start_t = time.time()
    for epoch in range(EPOCHES):
        avg_loss = 0.0
        #print("batch:", total_batch)
        curr_loss = None
        for i in range(total_batch):
            # if i <= 2:
            #     print(i, "-th:")
            #     print("BEF curr_W:", native_sess.run(W))
            #     print("BEF curr_b:", native_sess.run(b))
            batch_attr = plain_attr_ds[(i * BATCH_SIZE) : (i + 1) * BATCH_SIZE]
            #print("curr batch attr:", batch_attr)
            batch_label = plain_label_ds[(i * BATCH_SIZE) : (i + 1) * BATCH_SIZE]
            #print("curr batch label:", batch_label) 
            _, curr_loss = native_sess.run([optimizer, loss], feed_dict={X:batch_attr, Y: batch_label})
            # _, curr_loss, logit_v, sig_pv1, sig_pv2, log_v1, ty, log_v2 = native_sess.run([optimizer, loss, logits,
            #                                             sig_v, 1-sig_v, tf.log(1 - sig_v),
            #                                             1-Y, (1 - Y) * tf.log(1 - sig_v)],
            #                     feed_dict={X:batch_attr, Y: batch_label})
            # if i <= 2:
            #     print("logits:", logit_v, "\n sig:", sig_pv1, "and \n", sig_pv2)
            #     print("logv1:", log_v1, "\n logv2:",  log_v2, "\n ty:", ty, "\n loss:", curr_loss)
            avg_loss += curr_loss / total_batch
            

        if (epoch + 1) % DIS_STEP == 0:
            print("****Epoch:", epoch+1)
            #print(avg_loss)
            cW, cb = native_sess.run([W, b])
            print("curr_W:\n", cW)
            print("curr_b:\n", cb)
            #print("i:{:0>4d} weight:{} \nbias:{}".format(epoch+1, cW, cb))
            savecsv("{}-{:0>4d}-{}.csv".format(save_file_prefix, epoch+1, "W"), cW)
            savecsv("{}-{:0>4d}-{}.csv".format(save_file_prefix, epoch+1, "b"), cb)

            #print(f"Epoch: {epoch:5},  loss={float(avg_loss):20.9}")

    end_t = time.time()

    print("************ Native Trainning Finished!**************")
    print("The training cost: ", end_t - start_t, "seconds")
    print("W:", native_sess.run(W))
    print("b:", native_sess.run(b))

    print("************ Native Fitting on TRAIN dataset**************")
    is_pass_all = True
    self_check_pred = tf.sigmoid(tf.matmul(X,W) + b)
    real_label = Y
    pred_o, real_o =  native_sess.run([self_check_pred, real_label], feed_dict = {X:plain_attr_ds, Y: plain_label_ds})
    savecsv("{}-{:0>4d}-{}.csv".format(save_file_prefix, EPOCHES, "Y"), pred_o)

    for i in range(len(pred_o)):
            check = "TRUE"
            if round(pred_o[i][0]) != round(real_o[i][0]):
                check = "FALSE"
                is_pass_all = False 
            print("{:0>3d}".format(i), check + " (pred:", pred_o[i][0], "<===>", "real:", real_o[i][0], ")")    

    if is_pass_all:
        print("Pass")
    else:
        print("Fail")

#write log
Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()
