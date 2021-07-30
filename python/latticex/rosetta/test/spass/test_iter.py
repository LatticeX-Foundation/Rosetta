import latticex.rosetta as rtt
import numpy as np
import tensorflow as tf

# You can activate a backend protocol, here use SecureNN
rtt.activate("SecureNN")

# Get private data from Alice (input x), Bob (input y)
x = tf.Variable(np.ones([100, 50]))
y = tf.Variable(np.ones([100, 50]))

px = tf.data.Dataset.from_tensor_slices(x)
py = tf.data.Dataset.from_tensor_slices(y)
print("--------------------------------")
print("make dataset from tensor slices.")
print(px)
print(py)
print("--------------------------------")

iter_x = px.make_initializable_iterator()
iter_y = py.make_initializable_iterator()
print("--------------------------------")
print("create iterators.")
print(iter_x)
print(iter_y)
print("--------------------------------")


next_x = iter_x.get_next()
next_y = iter_y.get_next()
print("--------------------------------")
print("create get next.")
print(next_x)
print(next_y)
print("--------------------------------")

# Define matmul operation
res = tf.multiply(next_x, next_y)
print("create res=next_x * next_y")

# Start execution
try:
    with tf.Session() as sess:
        print("to run session...")
        sess.run(tf.global_variables_initializer())

        print("global initialized and to make iterator initialization....")
        sess.run([iter_x.initializer, iter_y.initializer])

        print('matmul:', sess.run(rtt.SecureReveal(res)))
    print("Pass")
except:
    print("Fail")


rtt.deactivate()
Writer = tf.summary.FileWriter("log/iter", tf.get_default_graph())
Writer.close()
