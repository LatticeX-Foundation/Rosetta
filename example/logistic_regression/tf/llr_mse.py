"""
Show case of native TF logistic regrssion with loss function: MSE
"""
import argparse
import time
import csv

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)



parser = argparse.ArgumentParser(description="Tensorflow Logistic Regression with MSE loss demo")
parser.add_argument('--epochs', type=int, help="train epochs")
parser.add_argument('--dims', type=int, help="train weights dims")
parser.add_argument('--learn_rate', type=float, help="train learn rate")
args = parser.parse_args()

EPOCHS = args.epochs
DIM_NUM = args.dims

LEARN_RATE = args.learn_rate
BATCH_SIZE = 5
DIS_STEP = 2


# n-D
X = tf.placeholder(tf.float64, [None, DIM_NUM])
# binary classes [0, 1]
Y = tf.placeholder(tf.float64, [None, 1])

W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64), dtype = tf.float64)
b = tf.Variable(tf.zeros([1], dtype=tf.float64), dtype = tf.float64)

pred = tf.nn.sigmoid(tf.matmul(X, W) + b)
cost = tf.reduce_mean(tf.multiply(Y - pred, Y - pred), name="plain_mse") / 2
#error: cost = tf.reduce_mean(tf.square(Y - pred, name="plain_mse")) / 2
loss = tf.train.GradientDescentOptimizer(LEARN_RATE).minimize(cost)

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
            #print(each_r)
    return res_data

file_name_prefix = str("../playground/datasets/") + str(DIM_NUM) + "D/" + str(DIM_NUM) + "d"
plain_attr_ds = read_dataset(file_name_prefix + "_attr_plain.csv")
plain_label_ds = read_dataset(file_name_prefix + "_label_plain.csv")

with tf.Session() as native_sess:
    native_sess.run(init)
    total_batch = int(len(plain_attr_ds) / BATCH_SIZE)
    start_t = time.time()
    for epoch in range(EPOCHS):
        avg_loss = 0.0
        #print("batch:", total_batch)
        curr_loss = None
        curr_cost = None
        for i in range(total_batch):
            #print("BEF curr_W:", native_sess.run(W))
            #print("BEF curr_b:", native_sess.run(b))
            batch_attr = plain_attr_ds[(i * BATCH_SIZE) : (i + 1) * BATCH_SIZE]
            #print("curr batch attr:", batch_attr)
            batch_label = plain_label_ds[(i * BATCH_SIZE) : (i + 1) * BATCH_SIZE]
            #print("curr batch label:", batch_label)
            curr_cost, curr_loss = native_sess.run([cost, loss],
                                feed_dict={X:batch_attr, Y: batch_label})
            avg_loss += curr_cost / total_batch


        if (epoch + 1) % DIS_STEP == 0:
            print("****Epoch:", epoch+1)
            #print(avg_loss)
            print("curr_W:\n", native_sess.run(W))
            print("curr_b:\n", native_sess.run(b))
            #print(f"Epoch: {epoch:5},  loss={float(avg_loss):20.9}")

    end_t = time.time()

    print("************ Native Trainning Finished!**************")
    print("The training cost: ", end_t - start_t, "seconds")
    print("trained-param W:", native_sess.run(W).tolist())
    print("trained-param b:", native_sess.run(b).tolist())

    print("************ Native Fitting on TRAIN dataset**************")
    is_pass_all = True
    self_check_pred = tf.sigmoid(tf.matmul(X,W) + b)
    real_label = Y
    pred_o, real_o =  native_sess.run([self_check_pred, real_label], feed_dict = {X:plain_attr_ds, Y: plain_label_ds})
    
    true_count=int(0)
    total_count=int(len(pred_o))
    for i in range(len(pred_o)):
            if round(pred_o[i][0]) == round(real_o[i][0]):
                true_count += 1
            else:
                print("{}-th predict error, (pred: {} <===> real: {}".format(i, pred_o[i][0], real_o[i][0]))

    print("trained-acc: ", (true_count * 100.0)/total_count)
    if true_count == total_count:
        print("Pass")
    else:
        print("Fail")

