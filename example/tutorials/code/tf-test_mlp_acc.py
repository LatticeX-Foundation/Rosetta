import os
import csv
import tensorflow as tf
import numpy as np
import pandas as pd
import argparse

np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'

np.random.seed(0)

EPOCHES = 10
BATCH_SIZE = 16
learning_rate = 0.0002

file_x = '../dsets/ALL/mnist_test_x.csv'
file_y = '../dsets/ALL/mnist_test_y.csv'
real_X, real_Y = pd.read_csv(file_x, header=None).to_numpy(
), pd.read_csv(file_y, header=None).to_numpy()


num_outputs = 10 
num_inputs = 784 
w=[]
b=[]

def mlp(x, num_inputs, num_outputs, num_layers, num_neurons):
    w = []
    b = []
    for i in range(num_layers):
        # weights
        w.append(tf.Variable(tf.random_normal(
            [num_inputs if i == 0 else num_neurons[i - 1],
             num_neurons[i]], dtype=tf.float64),
            name="w_{0:04d}".format(i), dtype=tf.float64
        ))
        # biases
        b.append(tf.Variable(tf.random_normal(
            [num_neurons[i]], dtype=tf.float64),
            name="b_{0:04d}".format(i), dtype=tf.float64
        ))
    w.append(tf.Variable(tf.random_normal(
        [num_neurons[num_layers - 1] if num_layers > 0 else num_inputs,
         num_outputs], dtype=tf.float64) , name="w_out",dtype=tf.float64))
    b.append(tf.Variable(tf.random_normal([num_outputs], dtype=tf.float64) , name="b_out", dtype=tf.float64))

    # x is input layer
    layer = x
    # add hidden layers
    for i in range(num_layers):
        layer = tf.nn.relu(tf.matmul(layer, w[i]) + b[i])
    # add output layer
    layer = tf.matmul(layer, w[num_layers]) + b[num_layers]

    return layer

# input images
x = tf.placeholder(dtype=tf.float64, name="x", 
                    shape=[None, num_inputs]) 
# target output
y = tf.placeholder(dtype=tf.float64, name="y", 
                    shape=[None, num_outputs])
num_layers = 0
num_neurons = []
learning_rate = 0.01
n_epochs = 30
batch_size = 100
n_batches = int(len(real_X)/batch_size)

model = mlp(x=x,
            num_inputs=num_inputs,
            num_outputs=num_outputs,
            num_layers=num_layers,
            num_neurons=num_neurons)

loss = tf.reduce_mean(
    tf.nn.sigmoid_cross_entropy_with_logits(logits=model, labels=y))
optimizer = tf.train.GradientDescentOptimizer(
    learning_rate=learning_rate).minimize(loss)

predictions_check = tf.equal(tf.argmax(model, 1), tf.argmax(y, 1))
accuracy_function = tf.reduce_mean(tf.cast(predictions_check, tf.float32))
# save
saver = tf.train.Saver(var_list=None, max_to_keep=5, name='v2')

init = tf.global_variables_initializer()

with tf.Session() as sess:
    sess.run(init)
    if os.path.exists("./log/ckpt0/checkpoint"):
        saver.restore(sess, './log/ckpt0/model')

    var_list = [v.name for v in tf.global_variables()]
    print(var_list)
    print(sess.run(var_list))
    feed_dict = {x: real_X, y: real_Y}
    accuracy_score = sess.run(accuracy_function, feed_dict=feed_dict)
    print("accuracy={0:.8f}".format(accuracy_score))
    

