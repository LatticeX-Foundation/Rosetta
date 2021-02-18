import os
import math
import matplotlib.pyplot as plt
import tensorflow as tf
import numpy as np
import pandas as pd

# import latticex.rosetta as rtt

np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)

EPOCHES = 1
BATCH_SIZE = 32
learning_rate = 0.003

# real data
# ######################################## difference from rosettta
tutorials_path = os.path.abspath(os.path.join(os.getcwd(), "../tutorials/"))
training_features_file_path = tutorials_path + "/dsets/ALL/iris_training_features.csv"
training_label_file_path = tutorials_path + "/dsets/ALL/iris_training_label.csv"
training_features = pd.read_csv(training_features_file_path, header=None).to_numpy()
training_label = pd.read_csv(training_label_file_path, header=None).to_numpy() - 1
test_features_file_path = tutorials_path + "/dsets/ALL/iris_test_features.csv"
test_label_file_path = tutorials_path + "/dsets/ALL/iris_test_label.csv"
test_features = pd.read_csv(test_features_file_path, header=None).to_numpy()
test_label = pd.read_csv(test_label_file_path, header=None).to_numpy() - 1
# ######################################## difference from rosettta
DIM_NUM = training_features.shape[1]

X = tf.placeholder(tf.float32, [None, DIM_NUM])
Y = tf.placeholder(tf.float32, [None, 1])
print(X)
print(Y)

# initialize W & b
W = tf.Variable(tf.zeros([DIM_NUM, 1]))
b = tf.Variable(tf.zeros([1]))
print(W)
print(b)

# predict
pred_Y = tf.matmul(X, W) + b
print(pred_Y)

# loss
loss = tf.square(Y - pred_Y)
loss = tf.reduce_mean(loss)
print(loss)

# optimizer
train = tf.train.GradientDescentOptimizer(learning_rate).minimize(loss)
print(train)

init = tf.global_variables_initializer()
print(init)

with tf.Session() as sess:
    sess.run(init)
    xW, xb = sess.run([W, b])
    print("init weight:{} \nbias:{}".format(xW, xb))

    # train
    BATCHES = math.ceil(len(training_features) / BATCH_SIZE)
    for e in range(EPOCHES):
        for i in range(BATCHES):
            bX = training_features[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            bY = training_label[(i * BATCH_SIZE): (i + 1) * BATCH_SIZE]
            sess.run(train, feed_dict={X: bX, Y: bY})

            j = e * BATCHES + i
            if j % 50 == 0 or (j == EPOCHES * BATCHES - 1 and j % 50 != 0):
                xW, xb = sess.run([W, b])
                print("I,E,B:{:0>4d},{:0>4d},{:0>4d} weight:{} \nbias:{}".format(
                    j, e, i, xW, xb))

    # predict
    Y_pred = sess.run(pred_Y, feed_dict={X: test_features, Y: test_label})
    # print("Y_pred:\n", Y_pred)
    # print("Y_true:\n", test_label)
    Y_comp = np.hstack((Y_pred,test_label))
    print("Y_comp:\n", Y_comp)
    # Compute the accuracy.
    Y_pred_cls0 = np.where(Y_pred >= 0, Y_pred, -1)
    Y_pred_cls0 = np.where(Y_pred < 0, Y_pred_cls0, 0)
    Y_pred_cls2 = np.where(Y_pred < 0.05, Y_pred, 1)
    Y_pred_cls2 = np.where(Y_pred >= 0.05, Y_pred_cls2, 0)
    Y_pred_cls012 = Y_pred_cls0 + Y_pred_cls2
    Y_comp = np.hstack((Y_pred_cls012, test_label))
    print(Y_comp)
    acc = len([i for i in Y_pred_cls012 if i in test_label]) / len(test_label)
    print('acc: ', acc)
