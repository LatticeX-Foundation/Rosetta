import tensorflow as tf
import latticex.rosetta as rst


import latticex.rosetta as cb
import tensorflow as tf


X = tf.random_uniform([1], dtype=tf.float64)
Y = tf.Variable(2.0, name='y')
Z = Y - X


try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Z)
    print("Pass")
except Exception:
    print("Fail")



Writer = tf.summary.FileWriter("log/random_uniform", tf.get_default_graph())
Writer.close()
rst.deactivate()