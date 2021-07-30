import tensorflow as tf
import latticex.rosetta as rst




X = tf.Variable(1.0, name='x')
Z = tf.nn.relu(X)


try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Z)
    print("Pass")
except Exception:
    print("Fail")


Writer = tf.summary.FileWriter("log/relu", tf.get_default_graph())
Writer.close()