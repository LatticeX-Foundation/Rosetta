#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common

res_flag = True
sess = None

# ===========================
# define tf mpc div grad func
# ===========================
def test_div_grad(X, Y, out_g, protocol="Helix"):
    cb.activate(protocol)

    global sess
    if sess is not None:
        sess.close()

    # ===========================
    # init global var
    # ===========================
    init = tf.compat.v1.global_variables_initializer()
    sess = tf.compat.v1.Session()
    sess.run(init)

    # ===========================
    # run mpc div grad
    # ===========================
    print("===========================")
    print("run mpc div(X,Y) grad")
    mpc_Z = cb.SecureTruediv(X, Y)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X), 
                                 common.get_var_from_rtt_tensor(Y)])
    print(sess.run(mpc_g))
    print("===========================")

    # ===========================
    # check mpcdiv grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcdiv grad ------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("------------------------------------------")


    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)



#===========================
# case: positive / positive
#===========================
X = tf.Variable(1.1)
Y = tf.Variable(2.2)
X2 = tf.Variable([1.1, 3.3])
Y2 = tf.Variable(2.2)

#===========================
# case: positive / negative
#===========================
X3 = tf.Variable(15.23)
Y3 = tf.Variable(-102.2)
X4 = tf.Variable([43.1, 113.3])
Y4 = tf.Variable(-202.2)

#===========================
# case: negative / positive
#===========================
X5 = tf.Variable(-1.234)
Y5 = tf.Variable(254.2)
X6 = tf.Variable([-991.123, -308.3])
Y6 = tf.Variable(20.2)

#===========================
# case: negative / negative
#===========================
X7 = tf.Variable(-16.31)
Y7 = tf.Variable(-32.)
X8 = tf.Variable([-10.001])
Y8 = tf.Variable(-82.1, -3.3)


#===========================
# run case
#===========================
# test helix grad op
print("run helix protocol...")
test_div_grad(X, Y, [0.45454544, -0.22727272])
test_div_grad(X2, Y2, [[0.45454544, 0.45454544], -0.9090909])
test_div_grad(X3, Y3, [-0.009784736, -0.0014581362])
test_div_grad(X4, Y4, [[-0.0049456, -0.0049456], -0.003825379])
test_div_grad(X5, Y5, [0.0039339103, 1.9096953e-05])
test_div_grad(X6, Y6, [[0.04950495, 0.04950495], 3.1845477])
test_div_grad(X7, Y7, [-0.03125, 0.015927734])
test_div_grad(X8, Y8, [[-0.01218027], 0.0014837377])

# test snn grad op
print("run snn protocol...")
test_div_grad(X, Y, [0.45454544, -0.22727272], protocol="SecureNN")
test_div_grad(X2, Y2, [[0.45454544, 0.45454544], -0.9090909], protocol="SecureNN")
test_div_grad(X3, Y3, [-0.009784736, -0.0014581362], protocol="SecureNN")
test_div_grad(X4, Y4, [[-0.0049456, -0.0049456], -0.003825379], protocol="SecureNN")
test_div_grad(X5, Y5, [0.0039339103, 1.9096953e-05], protocol="SecureNN")
test_div_grad(X6, Y6, [[0.04950495, 0.04950495], 3.1845477], protocol="SecureNN")
test_div_grad(X7, Y7, [-0.03125, 0.015927734], protocol="SecureNN")
test_div_grad(X8, Y8, [[-0.01218027], 0.0014837377], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)



Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()
