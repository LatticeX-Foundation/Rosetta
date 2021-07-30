import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test



# def Sqrt_1(task_id):
#     return unary_op_test("SecureNN", task_id, tf.sqrt,
#                         [[1.56, 2], [3.3, 4.43], [100.0, 1.85]], 
#                         [[1.249, 1.414], [1.817, 2.105], [10.0, 1.360]])


# def Sqrt_2(task_id):
#     return unary_op_test("Helix", task_id, tf.sqrt,
#                         [[11.26, 200], [4235, 1], [123456, 0.1]], 
#                         [[3.356, 14.142], [65.077, 1.0], [351.363, 0.316]])


# [[0.4, 0.86], [1.2, 1], [1.3, 2.02], [3.14, +2], [4, 19.0]] # x
# [[1.581, 1.078], [0.912, 1.], [0.877, 0.703], [0.564, 0.707], [0.5, 0.229]] # rsqrt expect
# [[0.632, 0.927], [1.095, 1.], [1.140, 1.421], [1.772, 1.414], [2., 4.358]] # sqrt expect

def Sqrt_3(task_id):
    return unary_op_test("SecureNN", task_id, tf.sqrt,
                         [[0.4, 0.86], [1.2, 1]], 
                        [[0.632, 0.927], [1.095, 1.]])


def Sqrt_4(task_id):
    return unary_op_test("Helix", task_id, tf.sqrt,
                        [[1.3, 2.02], [3.14, +2], [4, 19.0]], 
                        [[1.140, 1.421], [1.772, 1.414], [2., 4.358]])   

# ===========================
# run test cases
# ===========================
# mt_unit_test_fw([Sqrt_1, Sqrt_1])
# mt_unit_test_fw([Sqrt_2, Sqrt_2])
# mt_unit_test_fw([Sqrt_1, Sqrt_2])
# mt_unit_test_fw([Sqrt_1, Sqrt_2, Sqrt_1, Sqrt_2])
# mt_unit_test_fw([Sqrt_1, Sqrt_1, Sqrt_1, Sqrt_2, Sqrt_2, Sqrt_2])


mt_unit_test_fw([Sqrt_3, Sqrt_3])
mt_unit_test_fw([Sqrt_4, Sqrt_4])
mt_unit_test_fw([Sqrt_3, Sqrt_4])
mt_unit_test_fw([Sqrt_3, Sqrt_4, Sqrt_3, Sqrt_4])
mt_unit_test_fw([Sqrt_3, Sqrt_3, Sqrt_3, Sqrt_4, Sqrt_4, Sqrt_4])


