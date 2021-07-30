import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



# def Rsqrt_1(task_id):
#     return unary_op_test("SecureNN", task_id, tf.rsqrt,
#                          [[1.56, 2], [3.3, 4.43], [100.0, 1.85]], 
#                         [[1.249, 1.414], [1.817, 2.105], [10.0, 1.360]])


# def Rsqrt_2(task_id):
#     return unary_op_test("Helix", task_id, tf.rsqrt,
#                         [[11.26, 200], [4235, 1], [123456, 0.1]], 
#                         [[3.356, 14.142], [65.077, 1.0], [351.363, 0.316]])

def Rsqrt_3(task_id):
    return unary_op_test("SecureNN", task_id, tf.rsqrt,
                         [[0.4, 0.86], [1.2, 1]], 
                        [[1.581, 1.078], [0.912, 1.]])


def Rsqrt_4(task_id):
    return unary_op_test("Helix", task_id, tf.rsqrt,
                        [[1.3, 2.02], [3.14, +2], [4, 19.0]], 
                        [[0.877, 0.703], [0.564, 0.707], [0.5, 0.229]])

# [[0.4, 0.86], [1.2, 1], [1.3, 2.02], [3.14, +2], [4, 19.0]] # x
# [[1.581, 1.078], [0.912, 1.], [0.877, 0.703], [0.564, 0.707], [0.5, 0.229]] # rsqrt expect
# [[0.632, 0.927], [1.095, 1.], [1.140, 1.421], [1.772, 1.414], [2., 4.358]] # sqrt expect

# ===========================
# run test cases
# ===========================
# mt_unit_test_fw([Rsqrt_1, Rsqrt_1])
# mt_unit_test_fw([Rsqrt_2, Rsqrt_2])
# mt_unit_test_fw([Rsqrt_1, Rsqrt_2])
# mt_unit_test_fw([Rsqrt_1, Rsqrt_2, Rsqrt_1, Rsqrt_2])
# mt_unit_test_fw([Rsqrt_1, Rsqrt_1, Rsqrt_1, Rsqrt_2, Rsqrt_2, Rsqrt_2])

mt_unit_test_fw([Rsqrt_3, Rsqrt_3])
mt_unit_test_fw([Rsqrt_4, Rsqrt_4])
mt_unit_test_fw([Rsqrt_3, Rsqrt_4])
mt_unit_test_fw([Rsqrt_3, Rsqrt_4, Rsqrt_3, Rsqrt_4])
mt_unit_test_fw([Rsqrt_3, Rsqrt_3, Rsqrt_3, Rsqrt_4, Rsqrt_4, Rsqrt_4])


