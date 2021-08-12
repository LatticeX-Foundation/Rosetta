

import tensorflow as tf
import latticex.rosetta as cb


x = tf.Variable(2.0, name="X")
y = tf.Variable(3.0, name="Y")
x1 = tf.Variable(2.0, name="X1")
y1 = tf.Variable(3.0, name="Y1")
b = tf.Variable(0.1, name="b")
b1 = tf.Variable(0.1, name="b1")
z = (x * y + b) + (x1 * b1) + (y1 + b)
optimizer = tf.train.GradientDescentOptimizer(0.01).minimize(z)
saver = tf.train.Saver()


config = tf.ConfigProto(inter_op_parallelism_threads = 16, intra_op_parallelism_threads = 16)
with tf.Session(config = config) as sess:
    tf.global_variables_initializer().run()
    try:
      sess.run(optimizer)
      print("Pass")
    except Exception:
      print("Fail")

    try:
      saver.save(sess, "./ckp/test_save_model")
      print("Pass")
    except Exception:
      print("Fail")

#writer = tf.summary.FileWriter("log", tf.get_default_graph())
#writer.close()
cb.deactivate()

  



