import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, unary_op_test, PRECISION



def Exp_1(task_id):
    return unary_op_test("SecureNN", task_id, tf.exp,
                        [[1.56, 2], [3.3, 4.43], [-1, 0.5]], 
                        [[4.759, 7.389], [27.113, 83.913], [0.368, 1.649]])


def Exp_2(task_id):
    return unary_op_test("Helix", task_id, tf.exp,
                        [[11.26, 0], [-0.5, 1], [-7.2, 0.1]], 
                        [[77652.594, 1.], [0.607, 2.718], [0.0007, 1.105]])


def Exp_3(task_id):
    return unary_op_test("SecureNN", task_id, tf.exp,
                        [[-1.01, -2.00], [-3.01, 1.3]], 
                        [[0.364,  0.135], [0.049, 3.669]], precision=0.2)


def Exp_4(task_id):
    return unary_op_test("Helix", task_id, tf.exp,
                        [[2.02, 3.14], [+2, -0.01]], 
                        [[7.538, 23.104], [7.389, 0.990]], precision=0.5)
                        

# [-1.01, -2.00],  [-3.01, 1.3],   [2.02, 3.14],    [+2, -0.01] # x 
# [0.364,  0.135], [0.049, 3.669], [7.538, 23.104], [7.389, 0.990] # exp expect


# ===========================
# run test cases
# ===========================
# mt_unit_test_fw([Exp_1, Exp_1])
# mt_unit_test_fw([Exp_2, Exp_2])
# mt_unit_test_fw([Exp_1, Exp_2])
# mt_unit_test_fw([Exp_1, Exp_2, Exp_1, Exp_2])
# mt_unit_test_fw([Exp_1, Exp_1, Exp_1, Exp_2, Exp_2, Exp_2])


mt_unit_test_fw([Exp_3, Exp_3])
mt_unit_test_fw([Exp_4, Exp_4])
mt_unit_test_fw([Exp_3, Exp_4])
mt_unit_test_fw([Exp_3, Exp_4, Exp_3, Exp_4])
mt_unit_test_fw([Exp_3, Exp_3, Exp_3, Exp_4, Exp_4, Exp_4])


