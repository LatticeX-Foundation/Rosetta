# load rtt_ops.so and check ops

import os, sys
import unittest
import tensorflow as tf
import numpy as np

# load library
basepath = os.path.abspath(os.path.dirname(__file__)) + "/../../../build/lib"
libpath = basepath + "/librtt_ops.so"

# support_ops = ["rtt_add", "rtt_mat_mul", 
#                "rtt_reduce_mean", "rtt_square", 
#                "rtt_apply_gradient_descent", "rtt_mul", 
#                "rtt_reduce_sum", "rtt_sub", 
#                "rtt_div", "rtt_neg", 
#                "rtt_save_v2", "rtt_to_tf", "tf_to_rtt", 
#                "rtt_less", "rtt_less_equal", 
#                "rtt_equal", "rtt_greater", "rtt_greater_equal", 
#                "rtt_sigmoid", "rtt_sigmoid_cross_entropy"]

support_ops = ["rtt_add", "rtt_matmul", 
               "rtt_square", "rtt_sub", 
               "rtt_mul", "rtt_abs", 
               "rtt_log", "rtt_log1p", "rtt_pow",
               "rtt_div", "rtt_truediv", "rtt_realdiv", "rtt_floordiv", 
               "rtt_reduce_sum", "rtt_reduce_mean", "rtt_reduce_max","rtt_reduce_min", 
               "rtt_negative", "rtt_apply_gradient_descent", 
               "rtt_save_v2", "rtt_to_tf", "tf_to_rtt",
               "rtt_less", "rtt_less_equal", "rtt_not_equal",
               "rtt_equal", "rtt_greater", "rtt_greater_equal", 
               "rtt_sigmoid", "rtt_relu", "rtt_sigmoid_cross_entropy" ]

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
        
    def test_tf_to_rtt(self):
        self.assertIsNotNone(self.rtt)
        a = self.rtt.tf_to_rtt([1, 2])
        
        ret = create_run_session(a)
        self.assertEqual(np.double(ret).tolist(), np.double(np.array(["1", "2"])).tolist())
        
    def test_rtt_to_tf(self):
        self.assertIsNotNone(self.rtt)
        a = self.rtt.tf_to_rtt([1, 2])
        b = self.rtt.rtt_to_tf(a, dtype=tf.int32)
        
        ret = create_run_session(b)
        self.assertEqual(np.int32(ret).tolist(), np.int32(np.array([1, 2])).tolist())

    def test_add(self):
        self.assertIsNotNone(self.rtt)
        a = self.rtt.tf_to_rtt([1, 2])
        b = self.rtt.tf_to_rtt([1, 2])
        c = self.rtt.rtt_add(a, b)
        
        self.assertIsNotNone(c)
        
        # not implement now
        # ret = create_run_session(c)
        # self.assertEqual(np.double(ret).tolist(), np.double(np.array(["2", "4"])).tolist())
        
        
        
if __name__ == '__main__':
    unittest.main()