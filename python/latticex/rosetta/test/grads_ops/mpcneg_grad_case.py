#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common

res_flag = True
sess = None


# ===========================
# define tf mpc neg grad func
# ===========================
def test_neg_grad(X, out_g, protocol="Helix"):
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
    # run mpc neg grad
    # ===========================
    print("===========================")
    print("run mpc neg(X) grad")
    mpc_Z = cb.SecureNeg(X)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X)])
    print(sess.run(mpc_g))
    print("===========================")

    # ===========================
    # check mpc neg grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcneg grad ------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("------------------------------------------")

    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)


#===========================
# define tf varables
#===========================
X = tf.Variable(1.0)
X2 = tf.Variable( [[1.56, 2], [3.3, 4.43], [100.0, 1.85]] )


#===========================
# run case
#===========================
# test helix grad op
print("run helix protocol...")
test_neg_grad(X, [-1.0])
test_neg_grad(X2, [[-1., -1.], [-1., -1.], [-1., -1.]])

# test snn grad op
print("run snn protocol...")
test_neg_grad(X, [-1.0], protocol="SecureNN")
test_neg_grad(X2, [[-1., -1.], [-1., -1.], [-1., -1.]], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)



Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()

cb.deactivate()