#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common


res_flag = True
sess = None


# ===========================
# define tf mpc sigmoid grad func
# ===========================
def test_sigmoid_grad(X, out_g, protocol="Helix"):
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
    # run mpc sigmoid grad
    # ===========================
    print("===========================")
    print("run mpc sigmoid(X) grad")
    mpc_Z = cb.SecureSigmoid(X)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X)])
    print(sess.run(mpc_g))
    print("===========================")

    # ===========================
    # check mpcsigmoid grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcsigmoid grad------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("--------------------------------------------")

    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)



#===========================
# define tf varables
#===========================
X1 = tf.Variable([0.0])
X2 = tf.Variable([1.0])
X3 = tf.Variable([-1.0])
X4 = tf.Variable( [0.123456, 1.23638, -0.886, 1.439])


#===========================
# run case
#===========================
# test helix grad op
print("run helix protocol...")
test_sigmoid_grad(X1, [0.25])
test_sigmoid_grad(X2, [0.19661193])
test_sigmoid_grad(X3, [0.19661194])
test_sigmoid_grad(X4, [0.24904983, 0.1744117 , 0.20670937, 0.15495124])

# test snn grad op
print("run snn protocol...")
test_sigmoid_grad(X1, [0.25], protocol="SecureNN")
test_sigmoid_grad(X2, [0.19661193], protocol="SecureNN")
test_sigmoid_grad(X3, [0.19661194], protocol="SecureNN")
test_sigmoid_grad(X4, [0.24904983, 0.1744117 , 0.20670937, 0.15495124], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)


Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()

cb.deactivate()