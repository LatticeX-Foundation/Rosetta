import tensorflow as tf
import latticex.rosetta as rst



X = tf.Variable(2.0, name='x')
Z = tf.square(X)


try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Z)
    print("Pass")
except Exception:
    print("Fail")


Writer = tf.summary.FileWriter("log/square", tf.get_default_graph())
Writer.close()
rst.deactivate()