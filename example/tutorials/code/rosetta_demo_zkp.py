#!/usr/bin/env python3

# Import rosetta package
import latticex.rosetta as rtt
import tensorflow as tf

# You can activate a backend protocol, here we use Mystique
rtt.activate("Mystique")

# P0 is the Prover, providing all the witnesses(private), and
# P1 is the Verifier
matrix_a = tf.Variable(rtt.private_console_input(0, shape=(3, 2)))
matrix_b = tf.Variable(rtt.private_console_input(0, shape=(2, 3)))

# Just use the native tf.matmul operation.
cipher_result = tf.matmul(matrix_a, matrix_b)

# Start execution
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    # Take a glance at the ciphertext
    cipher_result_v = sess.run(cipher_result)
    print('local ciphertext result:', cipher_result_v)
    # Get the result of Rosetta matmul
    print('plaintext result:', sess.run(rtt.SecureReveal(cipher_result)))

rtt.deactivate()
