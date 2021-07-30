import tensorflow as tf
import latticex.rosetta as rst
import numpy as np

# the precision requirement of saving and restoring a float number
PRECISION = 1.0/100

# two initial values with double type.
num_a = np.array(
    [
        [1, 20.20],
        [0.3, 4]
    ], dtype =np.float_)
num_z = num_a * 10


x = tf.Variable(num_a, name="X")
z = x * 10
rv_z = rst.SecureReveal(z)
init = tf.global_variables_initializer()
saver = tf.train.Saver()

with tf.Session('') as sess:
  sess.run(init)
  try:
    saver.save(sess, "./ckp/test_save_model_1")
    
  except Exception:
    print("Fail")
  #sess.run(z)

with tf.compat.v1.Session() as mpc_check_result_sess:
    saver.restore(mpc_check_result_sess, "./ckp/test_save_model_1")
    a_out, z_out = mpc_check_result_sess.run([x, rv_z])
    print('restored x=', a_out)
    print('restored z=', z_out)

    precision_target = np.full(num_a.shape, PRECISION, np.float_)
    if ((abs(a_out.astype(float) - num_a) < precision_target).all() and \
        (abs(z_out.astype(float) - num_z) < precision_target).all()):
        print("num_a and num_z restored correctly!")
        print("Pass")
    else:
        print("num_a or num_b restored failed!")
        print("Fail")

#Writer = tf.summary.FileWriter("log", tf.get_default_graph())
#Writer.close()
  



