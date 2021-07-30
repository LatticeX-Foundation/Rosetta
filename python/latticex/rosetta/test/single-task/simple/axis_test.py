
#!/usr/bin/python

import latticex.rosetta as cb
import tensorflow as tf
import sys
import numpy as np
np.set_printoptions(suppress=True)

xa0 = tf.Variable(5.)
xa1 = tf.Variable([1., 2., 5., 3., 4.])
xa2 = tf.Variable([[1., 2., -3., 4.], [3., -4., 3., 1.], [5., 6., 3., 4.]])


init = tf.compat.v1.global_variables_initializer()
sess = tf.compat.v1.Session()
sess.run(init)

"""
Max
"""

print("000000000000000000000000000000000000000 axis=N")
xxc = tf.reduce_max(xa0)
print(xxc)
xcc = sess.run(xxc)
print(xcc)
xc = cb.SecureMax(xa0, axis=None)
print(xc)
xcc = sess.run(xc)
print(xcc)

print("111111111111111111111111111111111111111 axis=N")
xxc = tf.reduce_max(xa1)
print(xxc)
xcc = sess.run(xxc)
print(xcc)
xc = cb.SecureMax(xa1, axis=None)
print(xc)
xcc = sess.run(xc)
print(xcc)

print("111111111111111111111111111111111111111 axis=0")
xxc = tf.reduce_max(xa1, axis=0)
print(xxc)
xcc = sess.run(xxc)
print(xcc)
xc = cb.SecureMax(xa1, axis=0)
print(xc)
xcc = sess.run(xc)
print(xcc)


print("22222222222222222222222222222222222222 axis=N")
xxc = tf.reduce_max(xa2, axis=None)
print(xxc)
xc = cb.SecureMax(xa2, axis=None)
print(xc)
xcc = sess.run(xc)
print(xcc)

print("22222222222222222222222222222222222222 axis=0")
xxc = tf.reduce_max(xa2, axis=0)
print(xxc)
xc = cb.SecureMax(xa2, axis=0)
print(xc)
xcc = sess.run(xc)
print(xcc)

print("22222222222222222222222222222222222222 axis=1")
xxc = tf.reduce_max(xa2, axis=1)
print(xxc)
xc = cb.SecureMax(xa2, axis=1)
print(xc)
xcc = sess.run(xc)
print(xcc)


"""
Mean
"""


print("000000000000000000000000000000000000000 axis=N")
xxc = tf.reduce_mean(xa0)
print(xxc)
xcc = sess.run(xxc)
print(xcc)
xc = cb.SecureMean(xa0, axis=None)
print(xc)
xcc = sess.run(xc)
print(xcc)

print("111111111111111111111111111111111111111 axis=N")
xxc = tf.reduce_mean(xa1)
print(xxc)
xcc = sess.run(xxc)
print(xcc)
xc = cb.SecureMean(xa1, axis=None)
print(xc)
xcc = sess.run(xc)
print(xcc)

print("111111111111111111111111111111111111111 axis=0")
xxc = tf.reduce_mean(xa1, axis=0)
print(xxc)
xcc = sess.run(xxc)
print(xcc)
xc = cb.SecureMean(xa1, axis=0)
print(xc)
xcc = sess.run(xc)
print(xcc)


print("22222222222222222222222222222222222222 axis=N")
xxc = tf.reduce_mean(xa2, axis=None)
print(xxc)
xc = cb.SecureMean(xa2, axis=None)
print(xc)
xcc = sess.run(xc)
print(xcc)

print("22222222222222222222222222222222222222 axis=0")
xxc = tf.reduce_mean(xa2, axis=0)
print(xxc)
xc = cb.SecureMean(xa2, axis=0)
print(xc)
xcc = sess.run(xc)
print(xcc)

print("22222222222222222222222222222222222222 axis=1")
xxc = tf.reduce_mean(xa2, axis=1)
print(xxc)
xc = cb.SecureMean(xa2, axis=1)
print(xc)
xcc = sess.run(xc)
print(xcc)

