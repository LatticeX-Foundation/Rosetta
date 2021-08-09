#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common

res_flag = True
sess = None


# ===========================
# define tf mpc rsqrt grad func
# ===========================
def test_rsqrt_grad(X, out_g, protocol="Helix"):
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
    # run mpc rsqrt grad
    # ===========================
    print("===========================")
    print("run mpc rsqrt(X) grad")
    mpc_Z = cb.SecureRsqrt(X)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X)])
    print(sess.run(mpc_g))
    print("===========================")


    # ===========================
    # check mpc rsqrt grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcrsqrt grad ------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("------------------------------------------")

    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)



#===========================
# define tf varables
#===========================
X = tf.Variable(1.0)
X2 = tf.Variable( [[1.56, 2], [3.3, 4.43], [8.12, 6.85]] )
X3 = tf.Variable( [[4., 25.], [.9, 2.563], [4.102, 0.689]] )


#===========================
# run case
#===========================
# test helix grad op (unsupport)
# print("run helix protocol...")
# test_rsqrt_grad(X, [-0.5])
# test_rsqrt_grad(X2, [[-0.2566, -0.1767], [-0.0834, -0.0536], [-0.0216, -0.0279]])
# test_rsqrt_grad(X3, [[-0.0625, -0.0040], [-0.5856, -0.1219], [-0.0602, -0.8743]])

# test snn grad op
print("run snn protocol...")
test_rsqrt_grad(X, [-0.5], protocol="SecureNN")
test_rsqrt_grad(X2, [[-0.2566, -0.1767], [-0.0834, -0.0536], [-0.0216, -0.0279]], protocol="SecureNN")
test_rsqrt_grad(X3, [[-0.0625, -0.0040], [-0.5856, -0.1219], [-0.0602, -0.8743]], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)


Writer = tf.summary.FileWriter("rsqrt", tf.get_default_graph())
Writer.close()
cb.deactivate()