#!/usr/bin/env python3

# Import rosetta package
import latticex.rosetta as rtt
import tensorflow as tf

# You can activate a backend protocol, here we use SecureNN
rtt.activate("SecureNN")

# Get private data from every party
matrix_a = tf.Variable(rtt.private_console_input(0, shape=(3, 2)))
matrix_b = tf.Variable(rtt.private_console_input(1, shape=(2, 1)))
matrix_c = tf.Variable(rtt.private_console_input(2, shape=(1, 4)))

# Just use the native tf.matmul operation.
cipher_result = tf.matmul(tf.matmul(matrix_a, matrix_b), matrix_c)

# Start execution
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    # Take a glance at the ciphertext
    cipher_result_v = sess.run(cipher_result)
    print('local ciphertext result:', cipher_result_v)
    # Set only party a and c can get plain result
    a_and_c_can_get_plain = 0b101
    # Get the result of Rosetta matmul
    print('plaintext matmul result:', sess.run(
        rtt.SecureReveal(cipher_result, a_and_c_can_get_plain)))

rtt.deactivate()
