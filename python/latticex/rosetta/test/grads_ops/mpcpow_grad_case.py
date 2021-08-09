#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common



res_flag = True
sess = None


# ===========================
# define tf mpc pow grad func
# ===========================
def test_pow_grad(X, Y, out_g, protocol="Helix"):
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
    # run mpc pow grad
    # ===========================
    print("===========================")
    print("run mpc pow(X * Y) grad")
    mpc_Z = cb.SecurePow(X, Y)
    mpc_g = tf.gradients(mpc_Z, [common.get_var_from_rtt_tensor(X)])
    print(sess.run(mpc_g))
    print("===========================")
   
   
    # ===========================
    # check mpcpow grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcpow grad------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("--------------------------------------------")


    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)


#===========================
# define tf varables
#===========================
X = tf.Variable(2.0)
X2 = tf.Variable([1.3,   -2.1])
X3 = tf.Variable([-3.8, 41.43])
X4 = tf.Variable([20.0, -13.1])
powC2 = tf.constant(2.0, dtype=tf.float64)
powC3 = tf.constant(3.0, dtype=tf.float64)



#===========================
# run case
#===========================
# test helix grad op
print("run helix protocol...")
test_pow_grad(X, powC2, [4.0, 0.0])
test_pow_grad(X, powC3, [12.0, 0])
test_pow_grad(X2, powC2, [2.6, -4.2])
test_pow_grad(X2, powC3, [5.07, 13.23])
test_pow_grad(X3, powC2,[-7.6 , 82.86])
test_pow_grad(X3, powC3, [43.32, 5149.335])
test_pow_grad(X4, powC2, [40. , -26.2])
test_pow_grad(X4, powC3, [1200. , 514.83])

# test snn grad op
print("run snn protocol...")
test_pow_grad(X, powC2, [4.0, 0.0], protocol="SecureNN")
test_pow_grad(X, powC3, [12.0, 0], protocol="SecureNN")
test_pow_grad(X2, powC2, [2.6, -4.2], protocol="SecureNN")
test_pow_grad(X2, powC3, [5.07, 13.23], protocol="SecureNN")
test_pow_grad(X3, powC2,[-7.6 , 82.86], protocol="SecureNN")
test_pow_grad(X3, powC3, [43.32, 5149.335], protocol="SecureNN")
test_pow_grad(X4, powC2, [40. , -26.2], protocol="SecureNN")
test_pow_grad(X4, powC3, [1200. , 514.83], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)



Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()

cb.deactivate()