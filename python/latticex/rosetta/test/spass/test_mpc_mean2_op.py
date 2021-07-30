import tensorflow as tf
import latticex.rosetta as rst



X = tf.Variable([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], 
                dtype=tf.float32, name='x')
Z = tf.reduce_mean(X)


try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Z)
    print("Pass")
except Exception:
    print("Fail")



Writer = tf.summary.FileWriter("log/mean2", tf.get_default_graph())
Writer.close()


