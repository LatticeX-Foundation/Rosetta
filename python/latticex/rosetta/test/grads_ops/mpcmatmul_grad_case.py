#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common

res_flag = True
sess = None


#===========================
# define tf mpcmatmul grad func
#===========================
def test_matmul_grad(X, Y, out_g, protocol="Helix"):
    cb.activate(protocol)

    global sess
    if sess is not None:
        sess.close()

    init = tf.compat.v1.global_variables_initializer()
    sess = tf.compat.v1.Session()
    sess.run(init)

    
    # ===========================
    # run mpc matmul
    # ===========================
    print("===========================")
    print("run mpc matmul(X * Y) grad")
    mpc_Z = cb.SecureMatMul(X, Y)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X), 
                                 common.get_var_from_rtt_tensor(Y)])
    print(sess.run(mpc_g))
    print("===========================")


    # ===========================
    # check mpcmatmul grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcmatmul grad------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("--------------------------------------------")

    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)
    

#===========================
# define tf varables
#===========================
# 2-d, (2,2) x (2,2)
X = tf.Variable([[1., 1.], [1., 1.]])
Y = tf.Variable([[1., 1.], [1., 1.]])

# 2-d, (2,2) x (2,2)
X2 = tf.Variable([[1.1, 2.2], [3.3, 4.4]])
Y2 = tf.Variable([[5.5, 6.6], [7.7, 8.8]])

# 2-d, (2,3) x (3,2)
X3 = tf.Variable([[1.32, 0.5, -3.4], [0.63, 0.081, -1.3]])
Y3 = tf.Variable([[-0.2, 0.93], [-12, 4.3], [1.123, -0.53]])


#===========================
# run case
#===========================
# test helix grad op
print("run helix protocol...")
test_matmul_grad(X, Y, [[[2., 2.], [2., 2.]], [[2., 2.], [2., 2.]]])
test_matmul_grad(X2, Y2, [[[12.1, 16.5], [12.1, 16.5]], [[4.4 , 4.4],[6.6000004, 6.6000004]]])
test_matmul_grad(X3, Y3, [[[ 0.73, -7.7,  0.59300005],[ 0.73, -7.7,  0.59300005]], [[ 1.95 ,  1.95 ],[ 0.581,  0.581],[-4.7, -4.7]]])

# test snn grad op
print("run snn protocol...")
test_matmul_grad(X, Y, [[[2., 2.], [2., 2.]], [[2., 2.], [2., 2.]]], protocol="SecureNN")
test_matmul_grad(X2, Y2, [[[12.1, 16.5], [12.1, 16.5]], [[4.4 , 4.4],[6.6000004, 6.6000004]]], protocol="SecureNN")
test_matmul_grad(X3, Y3, [[[ 0.73, -7.7,  0.59300005],[ 0.73, -7.7,  0.59300005]], [[ 1.95 ,  1.95 ],[ 0.581,  0.581],[-4.7, -4.7]]], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)


Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()
cb.deactivate()
