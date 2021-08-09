#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as rst
import grad_test_utils as common

res_flag = True
sess = None

# ===========================
# define tf mpc div grad func
# ===========================
def test_sigmoidcrocssentropy_grad(logits, labels, out_g, protocol="Helix"):
    rst.activate(protocol)

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
    # run mpc SCE grad:
    # ===========================
    print("===========================")
    print("run mpc SCE(X,Y) grad")
    Z_mpc = rst.secure_sigmoid_cross_entropy_with_logits(logits=logits, labels=labels)
    mpc_g = tf.gradients(Z_mpc, [common.get_var_from_rtt_tensor(logits), 
                                 common.get_var_from_rtt_tensor(labels)])
    print(sess.run(mpc_g))
    print("===========================")

    # ===========================
    # check mpcSCE grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcSCE grad ------------")
        mpc_out_g.append(sess.run(rst.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("------------------------------------------")


    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)



#===========================
# define tf varables
#===========================
logits = tf.Variable(
    [
        [-10, -3],
        [10.0, 3.1],
        [-10, -3.1],
        [5.0, 3.1],
        [0.003, -0.02],
        [0.002, -0.02]
    ], dtype=tf.float64
)

labels = tf.Variable(
    [
        [0.0, 0.0], # correct 0
        [0.5, 0.5], # correct 1
        [0.5, 0.5], # wrong 1
        [0.0, 0.0], # wrong 0
        [0.5, 0.5], # hard t0 distinguish 0
        [0.5, 0.5]  # hard to distinguish 1
    ], dtype=tf.float64
)


#===========================
# run case
#===========================
eVal = [[[ 0.0,  0.048],
         [ 0.5,  0.5],
         [-0.5, -0.5],
         [ 0.993,  0.957],
         [ 0.0, -0.05],
         [ 0.0, -0.05]], 
        [[ 10.0,  3.0],
         [-10.0, -3.1],
         [ 10,  3.1],
         [-5.0, -3.1],
         [-0.003,  0.02],
         [-0.002,  0.02]]]

# test helix grad op
print("run helix protocol...")
test_sigmoidcrocssentropy_grad(logits, labels, eVal)

# test snn grad op
print("run snn protocol...")
test_sigmoidcrocssentropy_grad(logits, labels, eVal, protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)



Writer = tf.summary.FileWriter("log/sce", tf.get_default_graph())
Writer.close()
rst.deactivate()
