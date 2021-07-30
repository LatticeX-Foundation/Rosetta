import tensorflow as tf
import latticex.rosetta as rtt

protocol = "Wolverine"
rtt.activate(protocol)

X = tf.Variable([[10.5, 0.23, 8.], [7., 6., -10.73], [-9., 8., 0.125]], name='x')
res_0 = tf.nn.softmax(X, 0)
rv = rtt.SecureReveal(res_0)
res_1 = tf.nn.softmax(X)
rv_1 = rtt.SecureReveal(res_1)

#-----------------------------------------------------------------------------
# for test
# from latticex.rosetta.secure import StaticReplacePass
# from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts


# PassObj = StaticReplacePass()
# if isinstance(res, rtt_ts.RttTensor):
#     sm = PassObj.run(res._raw)
# else:
#     sm = PassObj.run(res)
#------------------------------------------------------------------------------

try:
    with tf.Session() as sess:
        print(sess.run(tf.global_variables_initializer()))
        print(sess.run(rv))
        print(sess.run(rv_1))
    print("Pass")
except:
    print("Fail")


rtt.deactivate()

Writer = tf.summary.FileWriter("log/softmax", tf.get_default_graph())
Writer.close()