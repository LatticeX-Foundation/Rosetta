#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common

res_flag = True
sess = None


# ===========================
# define tf mpc log1p grad func
# ===========================
def test_log1p_grad(X, out_g, protocol="Helix"):
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
    # run mpc log1p grad
    # ===========================
    print("===========================")
    print("run mpc log1p(X) grad")
    mpc_Z = cb.SecureLog1p(X)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X)])
    print(sess.run(mpc_g))
    print("===========================")

    # ===========================
    # check mpclog1p grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpclog1p grad------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("--------------------------------------------")

    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)


#===========================
# define tf varables
#===========================
X = tf.Variable(0.0)
X2 = tf.Variable( [[0.56, 1], [2.3, 3.43], [5.0, 0.85]] )
X3 = tf.Variable( [[8.26, 7.9], [9.3, 4.43], [10.0, 1.85]] )


#===========================
# run case
#===========================
# test helix grad op
print("run helix protocol...")
test_log1p_grad(X, [1.0])
test_log1p_grad(X2, [[0.64102566, 0.5], [0.3030303 , 0.22573362], [0.16666667, 0.5405405]])
test_log1p_grad(X3, [[0.10799136, 0.11235955], [0.09708738, 0.18416207], [0.09090909, 0.3508772]])

# test snn grad op
print("run snn protocol...")
test_log1p_grad(X, [1.0], protocol="SecureNN")
test_log1p_grad(X2, [[0.64102566, 0.5], [0.3030303 , 0.22573362], [0.16666667, 0.5405405]], protocol="SecureNN")
test_log1p_grad(X3, [[0.10799136, 0.11235955], [0.09708738, 0.18416207], [0.09090909, 0.3508772]], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)


Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()

