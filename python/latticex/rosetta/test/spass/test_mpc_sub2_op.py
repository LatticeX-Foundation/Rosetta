import tensorflow as tf
import latticex.rosetta as rst


X = tf.Variable(1.0, name='x')
Z = X - 4.0


try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Z)
    print("Pass")
except Exception:
    print("Fail")



Writer = tf.summary.FileWriter("log/sub2", tf.get_default_graph())
Writer.close()