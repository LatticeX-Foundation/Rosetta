

import tensorflow as tf
import latticex.rosetta as cb


x = tf.Variable([3.0], name='x')
w = tf.Variable([2.0], name='w')

z = x * w
optimizer = tf.train.GradientDescentOptimizer(0.01).minimize(z)


with tf.Session() as sess:
    tf.global_variables_initializer().run()
    try:
      sess.run(optimizer)
      print("Pass")
    except Exception:
      print("Fail")

#writer = tf.summary.FileWriter("log", tf.get_default_graph())
#writer.close()

  



