import tensorflow as tf
import numpy as np

PRECISION = 8.0 / 100
def check_mpc_op_grads(out_g, out_mpc_g):
    out_g = np.asarray(out_g)
    out_g = out_g.reshape(-1)
    out_mpc_g = np.asarray(out_mpc_g)
    out_mpc_g = out_mpc_g.reshape(-1)
    limit_precision = np.full(np.array(out_mpc_g).shape, PRECISION, np.float)
    for i in range(len(out_mpc_g)):
        lh = np.asarray(out_g[i]).reshape(-1)
        rh = np.asarray(out_mpc_g[i]).reshape(-1)
        print(lh)
        print(rh)
        for j in range(len(rh)):
            print("{0} cmp {1}".format(lh[j], float(rh[j])))
            if (abs(lh[j] - float(rh[j])) >PRECISION):
                return False

    return True


def get_var_from_rtt_tensor(rtt_tensor):
    op_name = rtt_tensor._raw.op.inputs[0].op.inputs[0].op.name
    for var in tf.global_variables():
        if (var.op.name == op_name):
            return var


def print_check_result(res_flag):
    if (res_flag == True):
        print("Pass")
    else:
        print("Fail")

