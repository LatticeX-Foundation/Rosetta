import tensorflow as tf
import latticex.rosetta as rst


X = tf.Variable(1.0, name='x')
Y = tf.Variable(2.0, name='y')
Z = X - Y


try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Z)
    print("Pass")
except Exception:
    print("Fail")



Writer = tf.summary.FileWriter("log/sub", tf.get_default_graph())
Writer.close()