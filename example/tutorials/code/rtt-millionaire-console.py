import latticex.rosetta as rtt
import tensorflow as tf
Alice = tf.Variable(rtt.private_console_input(0))
Bob = tf.Variable(rtt.private_console_input(1))
res = tf.greater(Alice, Bob)
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    res = sess.run(res)
    print('ret:', sess.run(rtt.MpcReveal(res)))  # ret: 1.0
