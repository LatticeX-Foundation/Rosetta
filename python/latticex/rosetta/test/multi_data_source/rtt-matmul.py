#!/usr/bin/env python3

# Import rosetta package
import latticex.rosetta as rtt
import tensorflow as tf

# You can activate a backend protocol, here use SecureNN
rtt.set_backend_loglevel(0)
rtt.activate("Helix")
#rtt.py_protocol_handler.set_loglevel(0)
print('node party', rtt.get_current_node_id(), rtt.node_id_to_party_id(rtt.get_current_node_id()))
print('party node', rtt.get_current_party_id(), rtt.party_id_to_node_id(rtt.get_current_party_id()))
print('data nodes', rtt.get_data_node_ids())
print('computation nodes', rtt.get_computation_node_ids())
print('result nodes', rtt.get_result_node_ids())
nodes = rtt.get_connected_node_ids()
node = rtt.get_current_node_id()
msgid = 'test'
for n in nodes:
    if n != node:
        rtt.send_msg(n, msgid, node + " to " + n)
for n in nodes:
    if n != node:
        msg = rtt.recv_msg(n, msgid, 2 * len(node) + 4)
        print('get msg from ', n, " msg:", msg)

# Get private data from Alice (input x), Bob (input y)
w = tf.Variable(rtt.private_input(0, [[1, 2], [2, 3]]))
x = tf.Variable(rtt.private_input(1, [[1, 2], [2, 3]]))
y = tf.Variable(rtt.private_input(2, [[1, 2], [2, 3]]))
z = tf.Variable(rtt.private_input('p9', [[1, 2], [2, 3]]))

# Define matmul operation
res = tf.matmul(tf.matmul(w, x), tf.matmul(y, z))

# Start execution
with tf.Session() as sess:
    sess.run(tf.global_variables_initializer())
    res = sess.run(res)

    # Get the result of Rosetta matmul
    # ret: [[b'14.000000' b'20.000000'] [b'20.000000' b'29.000000']]
    receivers = (0, 1, 'p9', 2)
    receivers = 0b011
    receivers = None
    print('matmul:', sess.run(rtt.SecureReveal(res, receive_party = receivers)))

rtt.deactivate()
