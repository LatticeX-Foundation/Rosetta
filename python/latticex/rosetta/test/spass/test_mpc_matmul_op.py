import tensorflow as tf
import latticex.rosetta as rst




X = tf.Variable([[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], 
                name='x')

Y = tf.Variable([[7.0, 8.0], [9.0, 10.0], [11.0, 12.0]], 
                name='y')
Q = tf.Variable([[7.0, 8.0], [9.0, 10.0], [11.0, 12.0]], 
                name='y')
Z = tf.matmul(X, Y)
P = tf.matmul(Z, Q, transpose_a=False, transpose_b=True)


try:
    train_step = tf.train.GradientDescentOptimizer(0.01).minimize(P)
    print("Pass")
except Exception:
    print("Fail")



Writer = tf.summary.FileWriter("log/matmul", tf.get_default_graph())
Writer.close()
rst.deactivate()


