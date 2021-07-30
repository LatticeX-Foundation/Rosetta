import tensorflow as tf
import numpy as np
from test_fw import mt_unit_test_fw, bin_op_test
import latticex.rosetta as rtt



def Add_1(task_id):
    return bin_op_test("SecureNN", task_id, tf.add, 
                        [1.1], [2.2], [3.3])


def Add_2(task_id):
    return bin_op_test("Helix", task_id, tf.add, 
                        [1.1, 3.3], [2.2], [3.3, 5.5])


# ===========================
# run test cases
# ===========================
mt_unit_test_fw([Add_1, Add_1])
mt_unit_test_fw([Add_2, Add_2])
mt_unit_test_fw([Add_1, Add_2])
mt_unit_test_fw([Add_1, Add_2, Add_1, Add_2])
mt_unit_test_fw([Add_1, Add_1, Add_1, Add_2, Add_2, Add_2])

