#!/usr/bin/env python3

# Import rosetta package
import latticex.rosetta as rtt
import tensorflow as tf

# You can activate a backend protocol, here use SecureNN
rtt.py_protocol_handler.set_loglevel(0);
rtt.activate("SecureNN")


# Get private data from Alice (input x), Bob (input y)
#x0 is variable
#x0 = tf.Variable(rtt.private_input(0, [[2, 0], [0, 2]]))
#x0 is constant
x0 = tf.constant([[2, 0], [0, 2]])
x1 = tf.Variable(rtt.private_input(1, [[3, 0], [0, 3]]))
 #Define matmul operation
res = rtt.mat_mul_add(x0, x1)

 # Start execution
with tf.Session() as sess:
  sess.run(tf.global_variables_initializer())
  res = sess.run(res)

  test = 0b111
  print('matmuladd:', sess.run(rtt.SecureReveal(res, test)))

rtt.deactivate()
