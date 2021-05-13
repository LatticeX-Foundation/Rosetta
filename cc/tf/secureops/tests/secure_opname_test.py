# load secure_ops.so and check ops

import os, sys
import unittest
import tensorflow as tf
import numpy as np

# load library
basepath = os.path.abspath(os.path.dirname(__file__)) + "/../../../../build/lib/"
libpath = basepath + "/libsecure-ops.so"

support_ops = ["secure_add", "secure_matmul",
               "secure_square", "secure_sub",
               "secure_mul", "secure_abs", "secure_reveal", "secure_add_n",
               "secure_log", "secure_log1p", "secure_pow",
               "secure_div", "secure_truediv", "secure_realdiv", "secure_floordiv",
               "secure_reduce_sum", "secure_reduce_mean", "secure_reduce_max","secure_reduce_min",
               "secure_negative", "secure_apply_gradient_descent",
               "secure_save_v2", "secure_to_tf", "tf_to_secure", "private_input",
               "secure_less", "secure_less_equal", "secure_not_equal",
               "secure_equal", "secure_greater", "secure_greater_equal",
               "secure_sigmoid", "secure_relu", "secure_sigmoid_cross_entropy" ]

def create_run_session(target):
    init = tf.global_variables_initializer()
    with tf.Session() as sess:
        sess.run(init)
        result = sess.run(target)

        return result


class RttLibraryLoadTest(unittest.TestCase):
    def __init__(self, methodName):
        self.rtt = None
        super(RttLibraryLoadTest, self).__init__(methodName=methodName)

    def setUp(self):
        self.rtt = tf.load_op_library(libpath)
        self.assertIsNotNone(self.rtt)

    def tearDown(self):
        self.rtt = None

    def test_has_all_support_ops(self):
        self.assertIsNotNone(self.rtt)

        export_names = {}
        for name in dir(self.rtt):
            export_names[name] = name

        for op in support_ops:
            print("check op: ", op)
            self.assertTrue(op in export_names)

    # def test_tf_to_rtt(self):
    #     self.assertIsNotNone(self.rtt)
    #     a = self.rtt.tf_to_rtt([1, 2])

    #     ret = create_run_session(a)
    #     self.assertEqual(np.double(ret).tolist(), np.double(np.array(["1", "2"])).tolist())

    # def test_secure_to_tf(self):
    #     self.assertIsNotNone(self.rtt)
    #     a = self.rtt.tf_to_rtt([1, 2])
    #     b = self.rtt.secure_to_tf(a, dtype=tf.int32)

    #     ret = create_run_session(b)
    #     self.assertEqual(np.int32(ret).tolist(), np.int32(np.array([1, 2])).tolist())

    # def test_add(self):
    #     self.assertIsNotNone(self.rtt)
    #     a = self.rtt.tf_to_rtt([1, 2])
    #     b = self.rtt.tf_to_rtt([1, 2])
    #     c = self.rtt.secure_add(a, b)

    #     self.assertIsNotNone(c)

    #     # not implement now
    #     # ret = create_run_session(c)
    #     # self.assertEqual(np.double(ret).tolist(), np.double(np.array(["2", "4"])).tolist())

if __name__ == '__main__':
    unittest.main()
