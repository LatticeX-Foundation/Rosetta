import tensorflow as tf
import latticex.rosetta as rst



a = tf.Variable([1.0], name='a')
b = tf.Variable([3.0], name='b')
new_a = tf.assign(a, 2, name="assige_2")
new_a2 = tf.assign(a, b, name="assign_b")


try:
    with tf.Session() as sess:
        sess.run(tf.initialize_all_variables())
        print(sess.run(new_a))
        print(sess.run(a))
        print(sess.run(new_a2))
        print(sess.run(a))
    print("Pass")
except:
    print("Fail")


Writer = tf.summary.FileWriter("log/assign", tf.get_default_graph())
Writer.close()
rst.deactivate()
