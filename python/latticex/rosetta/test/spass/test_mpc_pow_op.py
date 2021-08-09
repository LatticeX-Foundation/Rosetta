import tensorflow as tf
import latticex.rosetta as rst



X = tf.Variable(1.0, name='x')
Y = tf.constant(2.0, name='y')
Z = tf.pow(X, Y)


try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Z)
    print("Pass")
except Exception:
    print("Fail")



Writer = tf.summary.FileWriter("log/pow", tf.get_default_graph())
Writer.close()
rst.deactivate()


