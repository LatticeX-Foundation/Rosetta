#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common

res_flag = True
sess = None


# ===========================
# define tf mpc exp grad func
# ===========================
def test_exp_grad(X, out_g, protocol="Helix"):
    # cb.set_float_precision(16)
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
    # run mpc exp grad
    # ===========================
    print("===========================")
    print("run mpc exp(X) grad")
    mpc_Z = cb.SecureExp(X)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X)])
    print(sess.run(mpc_g))
    print("===========================")


    # ===========================
    # check mpc exp grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcexp grad ------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("------------------------------------------")

    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)



#===========================
# define tf varables
#===========================
X = tf.Variable(1.0)
X2 = tf.Variable( [1.56, 2])
X3 = tf.Variable( [[-1.26, -2.], [-3.3, -4.43], [-8.12, -6.85]] )


#===========================
# run case
#===========================
# test helix grad op (unsupport)
# print("run helix protocol...")
# test_exp_grad(X, [2.7183])
# test_exp_grad(X2, [4.7588, 7.3891])
# test_exp_grad(X3, [[0.2837, 0.1353], [0.0369, 0.0119], [0.0003, 0.0011]])

# test snn grad op
print("run snn protocol...")
test_exp_grad(X, [2.7183], protocol="SecureNN")
test_exp_grad(X2, [4.7588, 7.3891], protocol="SecureNN")
test_exp_grad(X3, [[0.2837, 0.1353], [0.0369, 0.0119], [0.0003, 0.0011]], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)


Writer = tf.summary.FileWriter("exp", tf.get_default_graph())
Writer.close()
