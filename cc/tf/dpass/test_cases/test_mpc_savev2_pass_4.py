

import tensorflow as tf
import latticex.rosetta as cb

x = tf.Variable(2.0, name="X")
y = tf.Variable(3.0, name="Y")
x1 = tf.Variable(2.0, name="X1")
y1 = tf.Variable(3.0, name="Y1")
b = tf.Variable(0.1, name="b")
b1 = tf.Variable(0.1, name="b1")
z = (x * y + b) + (x1 * b1) + (y1 + b)
init = tf.global_variables_initializer()
saver = tf.train.Saver()

with tf.Session('') as sess:
  sess.run(init)
  try:
    saver.save(sess, "./ckp/test_save_model_4")
    print("Pass")
  except Exception:
    print("Fail")
  #sess.run(z)

# Writer = tf.summary.FileWriter("log", tf.get_default_graph())
# Writer.close()
  



