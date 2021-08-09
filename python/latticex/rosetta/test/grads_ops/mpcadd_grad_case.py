#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common

res_flag = False
sess = None


# ===========================
# define tf mpc add grad func
# ===========================
def test_add_grad(X, Y, out_g, protocol="Helix"):
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
    # run mpc add grad
    # ===========================
    print("===========================")
    print("run mpc add(X + Y) grad")
    mpc_Z = cb.SecureAdd(X, Y)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X), 
                                 common.get_var_from_rtt_tensor(Y)])
    print(sess.run(mpc_g))
    print("===========================")

    # ===========================
    # reveal value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcadd grad ------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("------------------------------------------")

    # ===========================
    # check mpc add grads value
    # ===========================
    global res_flag
    res_flag = common.check_mpc_op_grads(out_g, mpc_out_g)


#===========================
# define tf varables
#===========================
X = tf.Variable(1.1, dtype=tf.float64)
Y = tf.Variable(2.2)
X2 = tf.Variable([1.1, 3.3])
Y2 = tf.Variable(2.2)


#===========================
# run case
#===========================
# test helix grad op
print("run helix protocol...")
test_add_grad(X, Y, [1.0, 1.0])
test_add_grad(X2, Y2, [[1., 1.], 2.0])

# test snn grad op
print("run snn protocol...")
test_add_grad(X, Y, [1.0, 1.0], protocol="SecureNN")
test_add_grad(X2, Y2, [[1., 1.], 2.0], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)



Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()
cb.deactivate()

