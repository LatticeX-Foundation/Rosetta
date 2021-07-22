import tensorflow as tf
import latticex.rosetta as rtt

protocol = "Wolverine"
rtt.activate(protocol)

X = tf.Variable([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], 
                dtype=tf.float32, name='x')
Z = tf.reduce_mean(X, axis=0, keepdims=True)
real_z = rtt.SecureReveal(Z)

# Writer = tf.summary.FileWriter("log/mean3", tf.get_default_graph())
# Writer.close()

init = tf.global_variables_initializer()
with tf.Session() as sess:
    sess.run(init)
    print(sess.run(real_z))






