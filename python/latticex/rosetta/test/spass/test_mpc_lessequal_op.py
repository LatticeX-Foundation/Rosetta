import tensorflow as tf
import latticex.rosetta as rst



X = tf.Variable([1.0, 2.0], name='x')
Y = tf.Variable([1.0, 2.0], name='y')
Z = tf.Variable([1.3, 1.4], name="z")
comp = tf.less_equal(X, Y)
T = tf.cast(comp, dtype=tf.float64)
Res = Z + T


try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Res)
    print("Pass")
except Exception:
    print("Fail")


Writer = tf.summary.FileWriter("log/less-eq", tf.get_default_graph())
Writer.close()
rst.deactivate()