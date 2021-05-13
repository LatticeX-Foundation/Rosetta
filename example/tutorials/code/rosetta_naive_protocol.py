#!/usr/bin/env python3

# Import rosetta package
import latticex.rosetta as rtt
import tensorflow as tf

# Attention!
# This is just for presentation of integrating a new protocol.
# NEVER USE THIS PROTOCOL IN PRODUCTION ENVIRONMENT!
rtt.activate("Naive")

# Get private data from P0 and P1
matrix_a = tf.Variable(rtt.private_console_input(0, shape=(3, 2)))
matrix_b = tf.Variable(rtt.private_console_input(1, shape=(3, 2)))

# Just use the native tf.multiply operation.
cipher_result = tf.multiply(matrix_a, matrix_b)

# Start execution
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    # Take a glance at the ciphertext
    cipher_a = sess.run(matrix_a)
    print('local shared matrix a:\n', cipher_a)
    cipher_result_v = sess.run(cipher_result)
    print('local ciphertext result:\n', cipher_result_v)
    # Get the result of Rosetta multiply
    print('plaintext result:\n', sess.run(rtt.SecureReveal(cipher_result)))

rtt.deactivate()
