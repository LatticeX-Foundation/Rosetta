import tensorflow as tf
import latticex.rosetta as rst


rst.activate()
X = tf.Variable(1.0, name='x')
Y = tf.Variable(2.0, name='y')
Z = tf.multiply(X, Y)
init = tf.global_variables_initializer()


try:
    with tf.Session() as sess:
        sess.run(init)
        for i in range(5):
            print(sess.run(Z))
    print("Pass")
except Exception as ex:
    print("Exception at:%s" %ex)
    print("Fail")


rst.deactivate()
Writer = tf.summary.FileWriter("log/pridict", tf.get_default_graph())
Writer.close()


