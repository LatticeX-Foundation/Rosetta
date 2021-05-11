#!/usr/bin/env python3
import tensorflow as tf

Alice = tf.Variable(2000001)
Bob = tf.Variable(2000000)

res = tf.greater(Alice, Bob)
init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    ret = sess.run(res)
    print("ret:", ret)  # ret: True
