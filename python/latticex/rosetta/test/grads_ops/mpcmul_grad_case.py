#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common


res_flag = True
sess = None

# ===========================
# define tf mpc mul grad func
# ===========================
def test_mul_grad(X, Y, out_g, protocol="Helix"):
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
    # run mpc mul grad
    # ===========================
    print("===========================")
    print("run mpc mul(X * Y) grad")
    mpc_Z = cb.SecureMul(X, Y)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X), 
                                 common.get_var_from_rtt_tensor(Y)])
    print(sess.run(mpc_g))
    print("===========================")
    

    # ===========================
    # check mpcmul grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcmul grad ------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("------------------------------------------")

    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)


#===========================
# case: positive * positive
#===========================
X = tf.Variable(1.1)
Y = tf.Variable(2.2)
X2 = tf.Variable([1.1, 3.3])
Y2 = tf.Variable(2.2)

#===========================
# case: positive * negative
#===========================
X3 = tf.Variable(15.23)
Y3 = tf.Variable(-102.2)
X4 = tf.Variable([43.1, 113.3])
Y4 = tf.Variable(-202.2)

#===========================
# case: negative * positive
#===========================
X5 = tf.Variable(-1.234)
Y5 = tf.Variable(254.2)
X6 = tf.Variable([-991.123, -308.3])
Y6 = tf.Variable(20.2)

#===========================
# case: negative * negative
#===========================
X7 = tf.Variable(-16.31)
Y7 = tf.Variable(-32.)
X8 = tf.Variable([-10.001])
Y8 = tf.Variable([-82.1, -3.3])


#===========================
# run case
#===========================
# test helix grad op
print("run helix protocol...")
test_mul_grad(X, Y, [2.2, 1.1])
test_mul_grad(X2, Y2, [[2.2, 2.2], 4.4])
test_mul_grad(X3, Y3, [-102.2, 15.23])
test_mul_grad(X4, Y4, [[-202.2, -202.2], 156.4])
test_mul_grad(X5, Y5, [254.2, -1.234])
test_mul_grad(X6, Y6, [[20.2, 20.2], -1299.423])
test_mul_grad(X7, Y7, [-32.0, -16.31])
test_mul_grad(X8, Y8, [-85.4, [-10.001, -10.001]])

# test snn grad op
print("run snn protocol...")
test_mul_grad(X, Y, [2.2, 1.1], protocol="SecureNN")
test_mul_grad(X2, Y2, [[2.2, 2.2], 4.4], protocol="SecureNN")
test_mul_grad(X3, Y3, [-102.2, 15.23], protocol="SecureNN")
test_mul_grad(X4, Y4, [[-202.2, -202.2], 156.4], protocol="SecureNN")
test_mul_grad(X5, Y5, [254.2, -1.234], protocol="SecureNN")
test_mul_grad(X6, Y6, [[20.2, 20.2], -1299.423], protocol="SecureNN")
test_mul_grad(X7, Y7, [-32.0, -16.31], protocol="SecureNN")
test_mul_grad(X8, Y8, [-85.4, [-10.001, -10.001]], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)



Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()

