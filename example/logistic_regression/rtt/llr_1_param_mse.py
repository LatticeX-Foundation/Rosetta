#%%
#!/usr/bin/python

# simply implements logitstic regress for liner: y = w*x + b

import numpy as np
import tensorflow as tf
import csv
import latticex.rosetta as rtt
import argparse

np.set_printoptions(suppress=True)


np.random.seed(2020)



NUM = 100 # with 100 or more samples will train w,b with quite large values
BATCH_SIZE = 5
DIS_STEP = 2

parser = argparse.ArgumentParser(description="MPC Logistic Regression with MSE loss demo")
parser.add_argument('--party_id', type=int, help="Party ID")
parser.add_argument('--epochs', type=int, help="train epochs")
parser.add_argument('--dims', type=int, help="train weights dims")
parser.add_argument('--learn_rate', type=float, help="train learn rate")

parser = argparse.ArgumentParser(description="MPC Logistic Regression with MSE loss demo with 1 param")
parser.add_argument('--party_id', type=int, help="Party ID")
args = parser.parse_args()
party_id = args.party_id
    
EPOCHS = args.epochs
DIM = args.dims
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

file_sample = str("./gen_data/") + str(DIM) + "_param_sample_" + str(party_id) + ".csv"
file_label = str("./gen_data/") + str(DIM) + "_param_label_" + str(party_id) + ".csv"

def create_sample(shape=(NUM,1)):
    x_sample = []
    y_sample = []
    x_data = np.double(np.linspace(0, 8, shape[0]) + np.random.uniform(-0.2, 0.2, shape[0]))
    y_data = x_data * 3 + np.random.uniform(-0.2, 0.2, shape[0])

    for i in range(shape[0]):
        x_sample.append([x_data[i]/2]) # secret sharing two equal parts
        y_sample.append([y_data[i]/2]) # secret sharing two equal parts

    return x_sample, y_sample

def show_two_one_graph(x1, y1, x2, y2):
    # show the train graph
    plt.plot(x1, y1, '*')
    plt.plot(x2, y2, '-')
    plt.show()


# input samples
x_data, y_data = create_sample()
# x_data = read_dataset(file_sample)
# y_data = read_dataset(file_label)

print("features: {}".format(x_data))
print("label: {}".format(y_data))

# w, b
w = tf.Variable(tf.zeros([DIM, 1], dtype=tf.float64), dtype = tf.float64, name="w")
b = tf.Variable(tf.zeros([1], dtype=tf.float64), dtype = tf.float64, name="b")

# X, Y
X = tf.placeholder(tf.float64, [None, DIM], name='X')
Y = tf.placeholder(tf.float64, [None, 1], name='Y')

# rw = rtt.MpcReveal(w)
# rb = rtt.MpcReveal(b)

# predict for x function
# pred_X = tf.sigmoid(tf.matmul(X, w) + b)
pred_X = tf.matmul(X, w) + b
# loss and Optimizer
# Mean Squared Error Cost Function
#cost = tf.reduce_mean(tf.square(pred_X-Y)) / 2
cost = tf.reduce_mean(tf.multiply(pred_X-Y, pred_X-Y)) / 2
loss = tf.train.GradientDescentOptimizer(LEARN_RATE).minimize(cost)


# init
init = tf.global_variables_initializer()

with tf.Session() as sess:
    Writer = tf.summary.FileWriter("./log/mse-1param/mpc-{}".format(party_id), tf.get_default_graph())
    Writer.close()

    sess.run(init)
    count = int(len(x_data) / BATCH_SIZE)
    for epoch in range(EPOCHS):
        for batch in range(count):
            print("before w: {}, b: {}".format(sess.run(rtt.MpcReveal(w)), sess.run(rtt.MpcReveal(b))))
            start = BATCH_SIZE * batch
            end = BATCH_SIZE * (batch + 1)
            sess.run(loss, feed_dict={X: x_data[start:end], Y: y_data[start:end]})
            # reveal w, b to check
            print("update w: {}, b: {}".format(sess.run(rtt.MpcReveal(w)), sess.run(rtt.MpcReveal(b))))
        
    shared_w, shared_b = sess.run([w, b])
    trained_w, trained_b = sess.run([rtt.MpcReveal(w), rtt.MpcReveal(b)])
    print("shared-param w: {}".format(shared_w))
    print("shared-param b: {}".format(shared_b))
    print("trained-param W: {}".format(trained_w))
    print("trained-param b: {}".format(trained_b))


# %%
