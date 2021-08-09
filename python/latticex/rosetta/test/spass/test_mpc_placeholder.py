import tensorflow as tf
import latticex.rosetta as rst



sig_v = tf.Variable(2.0)
Y = tf.placeholder(tf.float64)
One = tf.constant(1.0, dtype=tf.float64)
loss = tf.reduce_mean(-Y * tf.log(sig_v) - tf.subtract(One, Y) * tf.log(1 - sig_v))


try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(loss)
    print("Pass")
except Exception:
    print("Fail")



Writer = tf.summary.FileWriter("log/ph", tf.get_default_graph())
Writer.close()
rst.deactivate()