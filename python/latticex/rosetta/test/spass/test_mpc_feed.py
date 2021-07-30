import tensorflow as tf
import latticex.rosetta as rst
import numpy as np


rst.activate("SecureNN")
X = tf.placeholder(tf.float64, [2])
print(X)
Y = tf.Variable(tf.ones([2], dtype=tf.float64))

# predict
loss = X + Y

# optimizer
try:
    train = tf.train.GradientDescentOptimizer(0.01).minimize(loss)
    print(train)

    init = tf.global_variables_initializer()
    print(init)

    with tf.Session() as sess:
        sess.run(init)
        bX = np.array(['1', '2'])
        sess.run(train, feed_dict={X: bX})
    print("Pass")
    
except Exception:
    print("Fail")


rst.deactivate()
Writer = tf.summary.FileWriter("log/feed", tf.get_default_graph())
Writer.close()
