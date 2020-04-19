"""
Show case of MPC TF logistic regrssion with cost function MSE
"""
import argparse
import time
import csv

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as rstt

# LEARN_RATE = 0.001
BATCH_SIZE = 5
DIS_STEP = 2

parser = argparse.ArgumentParser(description="MPC Logistic Regression with MSE loss demo")
parser.add_argument('--party_id', type=int, help="Party ID")
parser.add_argument('--epochs', type=int, help="train epochs")
parser.add_argument('--dims', type=int, help="train weights dims")
parser.add_argument('--learn_rate', type=float, help="train learn rate")
args = parser.parse_args()
my_party_id = args.party_id

if my_party_id == 2:
    my_party_id = 0
    
EPOCHS = args.epochs
DIM_NUM = args.dims
LEARN_RATE = args.learn_rate

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

file_name_prefix = str("../playground/datasets/") + str(DIM_NUM) + "D/" + str(DIM_NUM) + "d"
file_name_suffix = "share_" + str(my_party_id) + ".csv"

shared_attr_ds = read_dataset(file_name_prefix + "_attr_" + file_name_suffix)
shared_label_ds = read_dataset(file_name_prefix + "_label_" + file_name_suffix)

print("features: {}".format(shared_attr_ds))
print("label: {}".format(shared_label_ds))


# n-D
X = tf.placeholder(tf.float64, [None, DIM_NUM])
# binary classes [0, 1]
Y = tf.placeholder(tf.float64, [None, 1])

W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64), dtype = tf.float64, name="W")
b = tf.Variable(tf.zeros([1], dtype=tf.float64), dtype = tf.float64, name="b")

pred = tf.sigmoid(tf.matmul(X, W) + b)
#pred = tf.matmul(X, W) + b
cost = tf.reduce_mean(tf.multiply(Y - pred, Y - pred), name="plain_mse") / 2
#error: cost = tf.reduce_mean(tf.square(Y - pred, name="plain_mse")) / 2
loss = tf.train.GradientDescentOptimizer(LEARN_RATE).minimize(cost)

init = tf.global_variables_initializer()

with tf.Session() as mpc_sess:
    Writer = tf.summary.FileWriter("./log/llr_mse/mpc-{}".format(args.party_id), tf.get_default_graph())
    Writer.close()
    
    mpc_sess.run(init)
    total_batch = int(len(shared_attr_ds) / BATCH_SIZE)
    start_t = time.time()
    for epoch in range(EPOCHS):
        avg_loss = 0.0
        #print("batch:", total_batch)
        curr_loss = None
        curr_cost = None
        for i in range(total_batch):
            #print("current W:", mpc_sess.run(rstt.MpcReveal(W)))
            #print("current b:", mpc_sess.run(rstt.MpcReveal(b)))
            batch_attr = shared_attr_ds[(i * BATCH_SIZE) : (i + 1) * BATCH_SIZE]
            #print("batch attr:", batch_attr)
            batch_label = shared_label_ds[(i * BATCH_SIZE) : (i + 1) * BATCH_SIZE]
            #print("batch label:", batch_label)
            curr_cost, curr_loss = mpc_sess.run([cost, loss],
                                feed_dict={X:batch_attr, Y: batch_label})
            #avg_loss += curr_cost / total_batch

            #print("update W:", mpc_sess.run(rstt.MpcReveal(W)))
            #print("update b:", mpc_sess.run(rstt.MpcReveal(b)))

        #if (epoch + 1) % DIS_STEP == 0:
            #type(avg_loss)
            #print("****Epoch: ", epoch, "\n ****current loss:")
            #print(avg_loss)
            # print("****Epoch: ", epoch, "\n ****current model:")
            # print("W:", mpc_sess.run(rstt.MpcReveal(W)))
            # print("b:", mpc_sess.run(rstt.MpcReveal(b)))
    end_t = time.time()

    print("The training cost: ", end_t - start_t, "seconds")
    print("************ MPC Trainning Finished! **************")
    print("trained-param W: ", mpc_sess.run(rstt.MpcReveal(W)).tolist())
    print("trained-param b: ", mpc_sess.run(rstt.MpcReveal(b)).tolist())

    print("************ MPC Fitting on TRAIN dataset **************")
    real_label = rstt.MpcReveal(Y)
    pred_reveal = rstt.MpcReveal(pred)
    pred_o, real_o =  mpc_sess.run([pred_reveal, real_label], feed_dict = {X:shared_attr_ds, Y: shared_label_ds})
    
    true_count=int(0)
    total_count=int(len(pred_o))
    for i in range(len(pred_o)):
            if round(pred_o[i][0]) == round(real_o[i][0]):
                true_count += 1   
            else:
                print("{}-th predict error, (pred: {} <===> real: {}".format(i, pred_o[i][0], real_o[i][0]))

    print("trained-acc: ", true_count * 100.0/total_count)
    if true_count == total_count:
        print("Pass")
    else:
        print("Fail")
