#!/usr/bin/env python3
import latticex.rosetta as rtt
import tensorflow as tf
rtt.activate("SecureNN")
Alice = tf.Variable(rtt.private_input(0, 2000001))
Bob = tf.Variable(rtt.private_input(1, 2000000))
res = tf.greater(Alice, Bob)
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    res = sess.run(res)
    print('ret:', sess.run(rtt.SecureReveal(res)))  # ret: 1.0

rtt.deactivate()
