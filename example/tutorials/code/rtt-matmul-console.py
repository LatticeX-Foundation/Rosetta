#!/usr/bin/env python3

# Import rosetta package
import latticex.rosetta as rtt
import tensorflow as tf

# You can activate a backend protocol, here use SecureNN
rtt.activate("SecureNN")

# Get private data from Alice (input x), Bob (input y)
x = tf.Variable(rtt.private_console_input(0, shape=(2, 3)))
y = tf.Variable(rtt.private_console_input(1, shape=(3, 2)))

# Define matmul operation
res = tf.matmul(x, y)

# Start execution
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    res = sess.run(res)

    # Get the result of Rosetta matmul
    print('matmul:', sess.run(rtt.SecureReveal(res)))

rtt.deactivate()
