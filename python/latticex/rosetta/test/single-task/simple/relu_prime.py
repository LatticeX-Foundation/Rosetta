import tensorflow as tf
import latticex.rosetta as rtt

print("inputs: ", [1.0, 0.0, 0, 0, -0.0, 0.0001, -0.000001, -0.000125, -1])
inputs = tf.Variable([1.0, 0.0, 0, 0, -0.0, 0.0001, -0.000001, -0.000125, -1], name="input")
relu_prime = rtt.SecureReveal(rtt.SecureReluPrime(inputs))

init = tf.compat.v1.global_variables_initializer()
with tf.compat.v1.Session() as sess:
    sess.run(init)

    ret = sess.run(relu_prime)
    print("relu-prime: {}".format(ret))

print("ok")
rtt.deactivate()
