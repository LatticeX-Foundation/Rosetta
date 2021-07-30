#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common

res_flag = True
sess = None


# ===========================
# define tf mpc sqrt grad func
# ===========================
def test_sqrt_grad(X, out_g, protocol="Helix"):
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
    # run mpc sqrt grad
    # ===========================
    print("===========================")
    print("run mpc sqrt(X) grad")
    mpc_Z = cb.SecureSqrt(X)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X)])
    print(sess.run(mpc_g))
    print("===========================")


    # ===========================
    # check mpc sqrt grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcsqrt grad ------------")
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
# test_sqrt_grad(X, [0.5])
# test_sqrt_grad(X2, [[0.4003, 0.3536], [0.2752, 0.2376], [0.1755 , 0.1910]])
# test_sqrt_grad(X3, [[0.25, 0.1], [0.527 , 0.3123], [0.2469, 0.6024]])

# test snn grad op
print("run snn protocol...")
test_sqrt_grad(X, [0.5], protocol="SecureNN")
test_sqrt_grad(X2, [[0.4003, 0.3536], [0.2752, 0.2376], [0.1755 , 0.1910]], protocol="SecureNN")
test_sqrt_grad(X3, [[0.25, 0.1], [0.527 , 0.3123], [0.2469, 0.6024]], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)


Writer = tf.summary.FileWriter("sqrt", tf.get_default_graph())
Writer.close()
