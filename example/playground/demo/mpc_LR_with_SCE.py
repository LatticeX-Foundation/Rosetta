""" 
Show case of MPC TF logistic regrssion with sigmoid cross entropy loss function
"""
import argparse
import time
import csv

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

# Note: 
# after importing our package, 
# the TF operations and some functions will be replaced by
# corresponding Mpc operations
import latticex.rosetta as rstt

# In this demo, two parties, which both have the secret-shared data,
# will try to train a logistic regression classification model With
# our rosetta MPC framework. 
learning_rate = 0.01
EPOCHES = 100
BATCH_SIZE = 5
DIS_STEP = 2
#SAVE_STEP = 5

DIM_NUM = 2


# n-D
X = tf.placeholder(tf.float64, [None, DIM_NUM])
# binary classes [0, 1]
Y = tf.placeholder(tf.float64, [None, 1])

W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64), dtype = tf.float64)
b = tf.Variable(tf.zeros([1], dtype=tf.float64), dtype = tf.float64)

#logits = tf.matmul(X, W)+b
loss =  tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(labels = Y,
                                                logits = tf.matmul(X, W) + b,
                                                name="mpc_sce"))
optimizer = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)

init = tf.global_variables_initializer()
def read_dataset(file_name = None):
    if file_name is None:
        print("Error! No file name!")
        return
    res_data = []
    with open(file_name, 'r') as f:
        cr = csv.reader(f)
        for each_r in cr:
            curr_r = [np.array([v], dtype=np.float_)[0] for v in each_r]
            res_data.append(curr_r)
            print(each_r)
    return res_data

file_name_prefix = str("../datasets/") + str(DIM_NUM) + "D/" + str(DIM_NUM) + "d"

parser = argparse.ArgumentParser(description="MPC Logistic Regression with SCE loss demo")
parser.add_argument('--party_id', type=int, help="Party ID")
args = parser.parse_args()
my_party_id = args.party_id

if my_party_id == 2:
    my_party_id = 0

file_name_suffix = "share_" + str(my_party_id) + ".csv"
shared_attr_ds = read_dataset(file_name_prefix + "_attr_" + file_name_suffix)
shared_label_ds = read_dataset(file_name_prefix + "_label_" + file_name_suffix)

# print(shared_attr_ds)
# print(shared_label_ds)

with tf.Session() as mpc_sess:
    mpc_sess.run(init)
    total_batch = int(len(shared_attr_ds) / BATCH_SIZE)
    start_t = time.time()
    for epoch in range(EPOCHES):
        avg_loss = 0.0
        #print("batch:", total_batch)
        curr_loss = None
        for i in range(total_batch):
            #print("BEF curr_W:", mpc_sess.run(W))
            #print("BEF curr_b:", mpc_sess.run(b))
            batch_attr = shared_attr_ds[(i * BATCH_SIZE) : (i + 1) * BATCH_SIZE]
            #print("curr batch attr:", batch_attr)
            batch_label = shared_label_ds[(i * BATCH_SIZE) : (i + 1) * BATCH_SIZE]
            #print("curr batch label:", batch_label) 
            _, curr_loss = mpc_sess.run([optimizer, loss],
                                feed_dict={X:batch_attr, Y: batch_label})
            #avg_loss += curr_loss / total_batch
            
            #print("AFT curr_W:", mpc_sess.run(W))
            #print("AFT curr_b:", mpc_sess.run(b))

        if (epoch + 1) % DIS_STEP == 0:
            #type(avg_loss)
            #print("****Epoch: ", epoch, "\n ****current loss:")
            #print(avg_loss)
            print("****Epoch:", epoch+1)
            print("curr_W:\n", mpc_sess.run(rstt.MpcReveal(W)))
            print("curr_b:\n", mpc_sess.run(rstt.MpcReveal(b)))
    end_t = time.time()

    print("************ MPC Trainning Finished! **************")
    print("The training cost: ", end_t - start_t, "seconds")
    print("W:", mpc_sess.run(rstt.MpcReveal(W)))
    print("b:", mpc_sess.run(rstt.MpcReveal(b)))

    print("************ MPC Fitting on TRAIN dataset **************")
    self_check_pred =rstt.MpcReveal(tf.sigmoid(tf.matmul(X,W) + b))
    # self_check_pred =rstt.MpcReveal(rstt.MpcSigmoid(rstt.MpcAdd(rstt.MpcMatMul(X, W), b)))
    real_label = rstt.MpcReveal(Y)
    pred_o, real_o =  mpc_sess.run([self_check_pred, real_label], feed_dict = {X:shared_attr_ds, Y: shared_label_ds})
    is_pass_all = True
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
