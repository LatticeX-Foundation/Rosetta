import tensorflow as tf
import latticex.rosetta as rtt


rtt.activate("SecureNN")
X = tf.Variable([1.0, 0.0, 0.0, 1.0], name='x')
Y = tf.Variable([1.0, 0.0, 1.0, 0.0], name='y')
Z = tf.logical_and(X, Y)
Z2 = tf.logical_and(tf.cast(X, tf.bool), tf.cast(Y, tf.bool))


# try:
#     train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Z)
#     print("Pass")
# except Exception:
#     print("Fail")

reveal_Z = rtt.SecureReveal(Z)
reveal_Z2 = rtt.SecureReveal(Z2)

try:
    init = tf.global_variables_initializer()
    with tf.Session() as sess:
        sess.run(init)
        print(sess.run(Z))
        print(sess.run(reveal_Z))
        print(sess.run(Z2))
        print(sess.run(reveal_Z2))

    print("Pass")
except Exception:
    print("Fail")


Writer = tf.summary.FileWriter("log/logical_and", tf.get_default_graph())
Writer.close()
rtt.deactivate()