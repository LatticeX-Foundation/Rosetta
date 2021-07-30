import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test
import latticex.rosetta as rtt



def Matmul_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.matmul, 
                    [[1.1, 2.2], [3.3, 4.4]], 
                    [[5.5, 6.6], [7.7, 8.8]], 
                    [[22.99, 26.62], [52.03, 60.5]])


def Matmul_2(task_id):
    return bin_op_test("Helix", task_id, tf.matmul,
                [[1.32, 0.5, -3.4], [0.63, 0.081, -1.3]],
                [[-0.2, 0.93], [-12, 4.3], [1.123, -0.53]],
                [[-10.08, 5.178], [-2.558, 1.6232]])



# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Matmul_1, Matmul_1])
mt_unit_test_fw([Matmul_2, Matmul_2])
mt_unit_test_fw([Matmul_1, Matmul_2])
mt_unit_test_fw([Matmul_1, Matmul_2, Matmul_1, Matmul_2])
mt_unit_test_fw([Matmul_1, Matmul_1, Matmul_1, Matmul_2, Matmul_2, Matmul_2])
