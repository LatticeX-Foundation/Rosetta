

import tensorflow as tf
import latticex.rosetta as cb



x = tf.Variable([3.0], name='x')
w = tf.Variable([2.0], name='w')
b = tf.Variable([2.0], name='b')
z = x * w + b
optimizer = tf.train.GradientDescentOptimizer(0.01).minimize(z)
saver = tf.train.Saver()


with tf.Session() as sess:
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

  



