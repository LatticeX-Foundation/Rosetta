import tensorflow as tf
import latticex.rosetta as rst



X = tf.Variable([1.0, 2.0], name='x')
Z = tf.reduce_max(X, axis=0)

try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Z)
    print("Pass")
except Exception:
    print("Fail")


Writer = tf.summary.FileWriter("log/max2", tf.get_default_graph())
Writer.close()