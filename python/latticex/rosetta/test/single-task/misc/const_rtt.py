
#!/usr/bin/python

import latticex.rosetta as cb
import tensorflow as tf
import sys, os
import numpy as np
np.set_printoptions(suppress=True)

"""
Variable Constant Placeholder
"""

xa0 = tf.Variable(5.)
xa1 = tf.Variable([1., 2., 5., 3., 4.])
xa2 = tf.Variable([[1., 2., -3., 4.], [3., -4., 3., 1.], [5., 6., 3., 4.]])


init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)


xc = tf.multiply(3.4, xa0)
print(xc)
xcc = sess.run(xc)
print("secure multiply(3.4, xa), lh_is_const: ", xcc)

xc = tf.multiply(1.2, 3.4)
print(xc)
xcc = sess.run(xc)
print("rtt_multiply(const,const): ", xcc)

xc = tf.multiply(xa0, 1.4)
print(xc)
xcc = sess.run(xc)
print("secure multiply(xa, 1.4), rh_is_const: ", xcc)

xc = tf.multiply(xc, 2.3)
print(xc)
xcc = sess.run(xc)
print(xcc)
print("secure output multiply 2.3, rh_is_const: ", xcc)

exit(0)

# the following statement is two consts, 
# will throw an exception in MPCOP
xc = tf.constant([1.,2.],dtype=tf.float64)
xp = tf.placeholder(tf.float64, shape=(2,))
xr = tf.multiply(xc, xp)
print(xr)
xrr = sess.run(xr, feed_dict={xp:[3.,4.]})
print(xrr)

xc = tf.constant([1.,2.],dtype=tf.float64)
xp = tf.placeholder(tf.float64, shape=(2,))
xr = tf.multiply(xc, xp)
print(xr)
xrr = sess.run(xr, feed_dict={xp:[3.,4.]})
print(xrr)