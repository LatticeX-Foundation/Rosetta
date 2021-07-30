import tensorflow as tf
import latticex.rosetta as rst
import numpy as np


rst.activate("SecureNN")
X = tf.placeholder(tf.float64, [2, 2])
Y = tf.placeholder(tf.float64, [2, 1])


# initialize W & b
W = tf.Variable(tf.zeros([2, 1], dtype=tf.float64))
b = tf.Variable(tf.zeros([1], dtype=tf.float64))


# predict
pred_Y = tf.sigmoid(tf.matmul(X, W) + b)


# loss
logits = tf.matmul(X, W) + b
loss = tf.nn.sigmoid_cross_entropy_with_logits(labels=Y, logits=logits)


# optimizer
try:
    # optimizer
    train = tf.train.GradientDescentOptimizer(0.01).minimize(loss)
    init = tf.global_variables_initializer()


    with tf.Session() as sess:
        sess.run(init)

        # train
        bX = np.array([['1', '2'], ['1', '2']])
        bY = np.array([['2'], ['1']])
        sess.run(train, feed_dict={X: bX, Y: bY})

    print("Pass")
    
except Exception:
    print("Fail")


rst.deactivate()
Writer = tf.summary.FileWriter("log/feed2", tf.get_default_graph())
Writer.close()
