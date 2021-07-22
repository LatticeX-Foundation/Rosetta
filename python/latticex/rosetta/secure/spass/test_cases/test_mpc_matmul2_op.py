import tensorflow as tf
import latticex.rosetta as rst




X = tf.Variable([[1.0, 2.0], [3.0, 4.0 ], [5.0, 6.0]],  name='x')
Y = tf.Variable([[7.0, 8.0], [9.0, 10.0], [11.0, 12.0]],  name='y')
Z = tf.matmul(X, Y, transpose_a=True)

Y1 = tf.Variable([[7.0, 8.0]], name="y1")
Z1 = tf.matmul(X, Y1, transpose_b=True)

Y2 = tf.Variable([[7.0, 8.0, 9.0]], name="y2")
Z2 = tf.matmul(X, Y2, transpose_a=True, transpose_b=True)

try:
    sess = tf.Session()
    sess.run(tf.global_variables_initializer())
    print(sess.run(Z))
    print(sess.run(Z1))
    print(sess.run(Z2))
    print("Pass")
except Exception:
    print("Fail")


# try:
#     train_step = tf.train.GradientDescentOptimizer(0.01).minimize(Z)
#     print("Pass")
# except Exception:
#     print("Fail")



Writer = tf.summary.FileWriter("log/matmul2", tf.get_default_graph())
Writer.close()


