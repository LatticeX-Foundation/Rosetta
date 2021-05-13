# unittest of secure math ops

import unittest
import os, sys
import tensorflow as tf
import numpy as np
import math
import argparse

# set output buffer to zero
class ZeroBufferOut(object):
    def __init__(self, stream):
        self.stream = stream

    def write(self, data):
        self.stream.write(data)
        self.stream.flush()

    def writelines(self, datas):
        self.stream.writelines(datas)
        self.stream.flush()

    def __getattr__(self, attr):
        return getattr(self.stream, attr)


sys.stdout = ZeroBufferOut(sys.stdout)

basepath = os.path.abspath(os.path.dirname(__file__)) + "/../../../../build/lib" 
rosettapth = (
    os.path.abspath(os.path.dirname(__file__))
    + "/../../../../build/lib.linux-x86_64-3.6/latticex"
)

rosettapth2 = (
    os.path.abspath(os.path.dirname(__file__))
    + "/../../../../build/lib.linux-x86_64-3.7/latticex"
)


sys.path.append(basepath)
sys.path.append(rosettapth2)
sys.path.append(rosettapth)

# inp = input("input: ")
# print(inp)
print(sys.path)
import _rosetta as rst

print(dir(rst))

# load library
libpath = basepath + "/libsecure-ops.so"
rtt = tf.load_op_library(libpath)

parser = argparse.ArgumentParser(description="LatticeX Rosetta")
parser.add_argument(
    "--party_id", type=int, help="Party ID", required=True, choices=[0, 1, 2]
)
parser.add_argument(
    "--cfgfile",
    type=str,
    help="Config File",
    default=os.path.abspath(".") + "/CONFIG.json",
)
args, unparsed = parser.parse_known_args()
# ###########################
party_id = args.party_id
cfgfile = args.cfgfile
print("party id: {} with config json file: {}".format(party_id, cfgfile))


def create_run_session(target):
    print("-----  create_run_session ---")
    init = tf.global_variables_initializer()
    with tf.Session() as sess:
        sess.run(init)
        result = sess.run(target)

    return result

def create_run_session_ex(in1, in2):
    print("-----  create_run_session ---")
    init = tf.global_variables_initializer()
    with tf.Session() as sess:
        sess.run(init)
        ret1, ret2 = sess.run([in1, in2])

    return ret1, ret2


def create_init_protocol_handler(prot="SecureNN", config=None, loglevel=2):
    proto_handler = rst.protocol.ProtocolHandler()
    proto_handler.set_loglevel(loglevel)
    configfile = open(config)
    content = configfile.read()
    configfile.close()
    proto_handler.activate(prot, content)
    
    print("-------  init mpc proto_handler and activate ok -----")

    return proto_handler, content

def test_reveal():
    print("----- test_reveal -----")
    in1 = tf.Variable(["1.1", "2.2", "3.3"], name="in1")
    
    ret1 = rtt.secure_reveal(in1, 1) # p0
    ret2 = rtt.secure_reveal(in1, 2) # p1
    ret4 = rtt.secure_reveal(in1, 4) # p2
    ret3 = rtt.secure_reveal(in1, 3) # p0, p1
    ret5 = rtt.secure_reveal(in1, 5) # p0, p2
    ret6 = rtt.secure_reveal(in1, 6) # p1, p2
    ret7 = rtt.secure_reveal(in1, 7) # p2, p1, p0
    result1 = create_run_session(ret1)
    print("reveal result1: ", result1)
    
    result2 = create_run_session(ret2)
    print("reveal result2: ", result2)
    
    result4 = create_run_session(ret4)
    print("reveal result4: ", result4)
    
    result3 = create_run_session(ret3)
    print("reveal result3: ", result3)
    
    result5 = create_run_session(ret5)
    print("reveal result5: ", result5)
    
    result6 = create_run_session(ret6)
    print("reveal result6: ", result6)
    
    result7 = create_run_session(ret7)
    print("reveal result7: ", result7)
    
    print("-----   test_reveal (OK) -----")

def test_private_input():
    print("----- test_private_input -----")
    #in1 = [[1,2,3], [4,5,6]]
    in1 = [["1","2","3"], ["4","5","6"]]
    v11 = rtt.private_input(in1, data_owner=0)
    v12 = rtt.private_input(in1, data_owner=0)
    v13 = rtt.private_input(in1, data_owner=0)

    v21 = rtt.private_input(in1, data_owner=1)
    v22 = rtt.private_input(in1, data_owner=1)
    v23 = rtt.private_input(in1, data_owner=1)

    v31 = rtt.private_input(in1, data_owner=2)
    v32 = rtt.private_input(in1, data_owner=2)
    v33 = rtt.private_input(in1, data_owner=2)
    
    ret11 = rtt.secure_reveal(v11, 7)
    ret12 = rtt.secure_reveal(v12, 7)
    ret13 = rtt.secure_reveal(v13, 7)
    ret21 = rtt.secure_reveal(v21, 7)
    ret22 = rtt.secure_reveal(v22, 7)
    ret23 = rtt.secure_reveal(v23, 7)
    ret31 = rtt.secure_reveal(v31, 7)
    ret32 = rtt.secure_reveal(v32, 7)
    ret33 = rtt.secure_reveal(v33, 7)
    result1 = create_run_session(ret11) # , ret12, ret13, ret21, ret22, ret23, ret31, ret32, ret33]) # , ret2, ret3, ret4, ret5, ret6, ret7, ret8, ret9
    print("reveal result1: ", result1)

    result2 = create_run_session(ret12)
    print("reveal result2: ", result2)
    
    result3 = create_run_session(ret13)
    print("reveal result4: ", result3)
    
    result4 = create_run_session(ret21)
    print("reveal result3: ", result4)
    
    result5 = create_run_session(ret22)
    print("reveal result5: ", result5)
    
    result6 = create_run_session(ret23)
    print("reveal result6: ", result6)
    
    result7 = create_run_session(ret31)
    print("reveal result7: ", result7)

    result8 = create_run_session(ret32)
    print("reveal result8: ", result8)

    result9 = create_run_session(ret33)
    print("reveal result9: ", result9)
    
    print("!! expect all to be: [[1,2,3],[4,5,6]] !!")
    print("-----  test_private_input (OK) -----")

def test_add():
    print("-----   test_add -----")
    assert rtt
    in1 = tf.Variable(["1","1"], name="in1")
    in2 = tf.Variable(["2","2"], name="in2")
    c = rtt.secure_add(in1, in2)
    ret = rtt.secure_reveal(c)
    result = create_run_session(ret)
    print("add result: ", result, ", expect: [3, 3]")
    print("-----   test_add (OK) -----")

def test_sub():
    print("-----   test_sub -----")
    assert rtt
    in1 = tf.Variable(["1","1.1"], name="in1")
    in2 = tf.Variable(["2","2.2"], name="in2")
    c = rtt.secure_sub(in1, in2)
    ret = rtt.secure_reveal(c)
    ## ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("sub result: ", result, ", expect: [-1, -1.1]")
    print("-----   test_sub (OK) -----")

def test_mul():
    print("-----   test_mul -----")
    assert rtt
    in1 = tf.Variable(["18458288", "18458288"], name="in1")
    in2 = tf.Variable(["184582879", "1845828797"], name="in2") # ok
    # in1 = tf.Variable(["1845828800", "1845828801"], name="in1")
    # in2 = tf.Variable(["69458287980", "1845828797"], name="in2") # ok
    # in1 = tf.Variable(["3"], name="in1")
    # in2 = tf.Variable(["2"], name="in2") # ok
    # a = rtt.tf_to_secure(in1, name="a")
    #b = rtt.tf_to_secure(in2, name="b")
    c = rtt.secure_mul(in1, in2)
    ret = rtt.secure_reveal(c)
    # # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    # print("secret-sharing value: ", cstring)
    print("mul result: ", result, ", expect: ", 18458288*184582879, 18458288*1845828797)
    print("-----   test_mul (OK) -----")

def test_div():
    print("-----   test_div -----")
    assert rtt
    in1 = tf.Variable(["1","1"], name="in1")
    in2 = tf.Variable(["2","2"], name="in2")
    # a = rtt.tf_to_secure(in1, name="a")
    # b = rtt.tf_to_secure(in2, name="b")
    c = rtt.secure_div(in1, in2)
    ret = rtt.secure_reveal(c)
    # # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)

    print("div result: ", result, ", expect: [0.5, 0.5]")
    print("-----   test_div (OK) -----")

def test_div_const():
    print("-----   test_div -----")
    assert rtt
    in1 = tf.Variable(["1","1"], name="in1")
    in2 = tf.constant(["4","4"], name="in2")
    c = rtt.secure_div(in1, in2, rh_is_const=True)
    ret = rtt.secure_reveal(c)
    # # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)

    print("div result: 1/4: ", result)
    print("-----   test_div (OK) -----")

def test_matmul():
    print("-----   test_matmul -----")
    in1 = tf.Variable([["1845828799","1"],["1845828799","1"]], name="in1")
    in2 = tf.Variable([["1845828799","1"],["1845828799","1"]], name="in2")
    # # a = rtt.tf_to_secure(in1, name="a")
    # b = rtt.tf_to_secure(in2, name="b")
    c = rtt.secure_matmul(in1, in2)
    ret = rtt.secure_reveal(c)
    # # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("matmul result: ", result, ", expect: [[3407083957063611200, 1845828800], [3407083957063611200, 1845828800]]")
    print("-----   test_matmul (OK) -----")

def test_less():
    print("-----   test_less -----")
    in1 = tf.Variable(["1","3"], name="in1")
    in2 = tf.Variable(["2","2"], name="in2")
    # a = rtt.tf_to_secure(in1, name="a")
    # b = rtt.tf_to_secure(in2, name="b")
    c = rtt.secure_less(in1, in2)
    ret = rtt.secure_reveal(c)
    # # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("less result: ", result, ", expect: [1, 0]")
    print("-----   test_less (OK) -----")

def test_less_equal():
    print("-----   test_less_equal -----")
    in1 = tf.Variable(["1","1"], name="in1")
    in2 = tf.Variable(["2","2"], name="in1")
    # # a = rtt.tf_to_secure(in1, name="a")
    # b = rtt.tf_to_secure(in2, name="b")
    c = rtt.secure_less_equal(in1, in2)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("less_equal result: ", result, ", expect: [1, 1]")
    print("-----   test_less_equal (OK) -----")

def test_equal():
    print("-----   test_equal -----")
    in1 = tf.Variable(["1","1"], name="in1")
    in2 = tf.Variable(["2","2"], name="in2")
    # # a = rtt.tf_to_secure(in1, name="a")
    # b = rtt.tf_to_secure(in2, name="b")
    c = rtt.secure_equal(in1, in2)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("equal result: ", result, ", expect: [0, 0]")
    print("-----   test_equal (OK) -----")

def test_not_equal():
    print("-----   test_not_equal -----")
    in1 = tf.Variable(["1","1"], name="in1")
    in2 = tf.Variable(["2","2"], name="in2")
    # # a = rtt.tf_to_secure(in1, name="a")
    # b = rtt.tf_to_secure(in2, name="b")
    c = rtt.secure_not_equal(in1, in2)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("not_equal result: ", result, ", expect: [1, 1]")
    print("-----   test_not_equal (OK) -----")

def test_greater():
    print("-----   test_greater -----")
    in1 = tf.Variable(["1","1"], name="in1")
    in2 = tf.Variable(["2","2"], name="in2")
    # # a = rtt.tf_to_secure(in1, name="a")
    # b = rtt.tf_to_secure(in2, name="b")
    c = rtt.secure_greater(in1, in2)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("greater result: ", result, ", expect: [0, 0]")
    print("-----   test_greater (OK) -----")

def test_greater_equal():
    print("-----   test_greater_equal -----")
    in1 = tf.Variable(["1","1"], name="in1")
    in2 = tf.Variable(["2","2"], name="in2")
    # # a = rtt.tf_to_secure(in1, name="a")
    # b = rtt.tf_to_secure(in2, name="b")
    c = rtt.secure_greater_equal(in1, in2)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("greater_equal result: ", result, ", expect: [0, 0]")
    print("-----   test_greater_equal (OK) -----")

def test_pow2():
    print("-----   test_pow2 -----")
    in1 = tf.Variable(["1","2"], name="in1")
    in2 = tf.constant(["1","1"], name="in2")
    # a = rtt.tf_to_secure(in1, name="a")
    # b = tf.constant(["1","2"], name="b") #rtt.tf_to_secure(in2, name="b")
    c = rtt.secure_pow(in1, in2)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("pow2 result: ", result, ", math.pow: ", [1.0, 2.0])
    print("-----   test_pow2 (OK) -----")

def test_log():
    print("-----   test_log -----")
    in1 = tf.Variable(["2","10"], name="in1")
    # # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_log(in1)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, ", math.log 2, 10: ", math.log(2), math.log(10))
    print("-----   test_log (OK) -----")

def test_log1p():
    print("-----   test_log1p -----")
    in1 = tf.Variable(["2","5"], name="in1")
    # # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_log1p(in1)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, ", math.log1p 2, 10: ", math.log1p(2), math.log1p(5))
    print("-----   test_log1p (OK) -----")

def test_square():
    print("-----   test_square -----")
    in1 = tf.Variable(["1","2"], name="in1")
    # # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_square(in1)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, ", math.square 1, 2: ", math.pow(1,2), math.pow(2,2))
    print("-----   test_square (OK) -----")

def test_sigmoid():
    print("-----   test_sigmoid -----")
    in1 = tf.Variable(["1","2"], name="in1")
    # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_sigmoid(in1)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, ", math.sigmoid 1, 2: ", 1/(1+math.exp(-1)), 1/(1+math.exp(-2)))
    print("-----   test_sigmoid (OK) -----")



def sigmoid(x):
    return 1.0/(1.0+math.exp(-x))
def ideal_sigmoid_entropy_with_logit(z,x):
    return math.log(sigmoid(x))*(-z) + (-1+z) * math.log(1 - sigmoid(x))


# let x = logits, z = labels. The logistic loss is
# z * -log(sigmoid(x)) + (1 - z) * -log(1 - sigmoid(x))
def test_sigmoid_entropy_with_logit():
    print("-----   test_sigmoid_entropy_with_logit -----")
    in1 = tf.Variable(["1","2"], name="in1")
    in2 = tf.Variable(["1","2"], name="in2")
    # a = rtt.tf_to_secure(in1, name="a")
    # b = rtt.tf_to_secure(in1, name="b")
    c = rtt.secure_sigmoid_cross_entropy(in1, in2)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    x = [1,2]
    z = [1,2]
    sigmoid_entropy = []
    for i in (0,1):
        sigmoid_entropy.append(ideal_sigmoid_entropy_with_logit(z[i], x[i]))

    print("secure result: ", result, ", math.sigmoid_entropy_with_logit 1, 2: ", sigmoid_entropy)
    print("-----   test_sigmoid_entropy_with_logit (OK) -----")


def test_relu():
    print("-----   test_relu -----")
    in1 = tf.Variable(["-1", "2", "0", "-2"], name="in1")
    # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_relu(in1)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, "math.relu -1, 2 , 0, -2: ", 0, 2, 0, 0)
    print("-----   test_relu (OK) -----")

def test_relu_prime():
    print("-----   test_relu_prime -----")
    print("-----   relu-input: -1, 2, 0, -2 ------")
    in1 = tf.Variable(["-1", "2", "0", "-2"], name="in1")
    # in1 = tf.Variable(["-1", "2"], name="in1") # sometimes ok
    # in1 = tf.Variable(["2", "-1", "0", "1"], name="in1")
    # in1 = tf.Variable(["2", "-1"], name="in1")
    # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_relu_prime(in1)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, "math.relu_prime -1, 2, 0, -2: ", 0, 1, 1, 0)
    print("-----   test_relu_prime (OK) -----")

def test_abs_prime():
    print("-----   test_abs_prime -----")
    in1 = tf.Variable(["-0.1", "0.4", "1", "2"], name="in1")
    # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_abs_prime(in1)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, "math.abs_prime -0.1, 0.4, 1, 2: ", -1, 1, 1, 1)
    print("-----   test_abs_prime (OK) -----")

def test_abs():
    print("-----   test_abs -----")
    in1 = tf.Variable(["-1", "2", "-0.1", "0.4"], name="in1")
    # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_abs(in1)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, "math.abs -1, 2, -0.1, 0.4: ", 1, 2, 0.1, 0.4)
    print("-----   test_abs (OK) -----")


def test_reduce_sum():
    print("-----   test_reduce_sum -----")
    in1 = tf.Variable(["-1","2"], name="in1")
    # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_reduce_sum(in1, reduction_indices=0)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, "expect: ", 1)
    print("-----   test_reduce_sum (OK) -----")

def test_add_n_core_dump():
    print("-----   test_add_n -----")
    in1 = tf.Variable(["-1", "2"], name="in1")
    # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_add_n([in1])
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, ", math.add_n -1, 2: ", 2)
    print("-----   test_add_n (OK) -----")

def test_add_n():
    print("-----   test_add_n -----")
    in1 = tf.Variable([["-1", "2"], ["-1", "2"]], name="in1")
    in2 = tf.Variable([["-1", "2"], ["-1", "2"]], name="in2")
    # a = rtt.tf_to_secure(in1, name="a")
    # b = rtt.tf_to_secure(in1, name="b")
    c = rtt.secure_add_n([in1, in2])
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, ", expect: ", [[-2, 4], [-2, 4]])
    print("-----   test_add_n (OK) -----")

def test_reduce_mean():
    print("-----   test_reduce_mean -----")
    in1 = tf.Variable(["-1", "2"], name="in1")
    # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_reduce_mean(in1, reduction_indices=0)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, ", expect: ", 0.5)
    print("-----   test_reduce_mean (OK) -----")

def test_reduce_min():
    print("-----   test_reduce_min -----")
    in_values = [["-1", "2", "1"], ["-4", "6", "3"], ["3", "7", "2"]] # ["2", "1", "5", "4"]
    in1 = tf.Variable(in_values, name="in1")
    a = rtt.tf_to_secure(in1, name="a")
    c_1 = rtt.secure_reduce_min(in1, reduction_indices=-1)
    c0 = rtt.secure_reduce_min(in1, reduction_indices=0)
    c1 = rtt.secure_reduce_min(in1, reduction_indices=1)

    ret_1 = rtt.secure_reveal(c_1)
    ret0 = rtt.secure_reveal(c0)
    ret1 = rtt.secure_reveal(c1)
    result_1 = create_run_session(ret_1)
    result0 = create_run_session(ret0)
    result1 = create_run_session(ret1)
    print("input:", in_values, ", secure result axis=-1: ", result_1, ", expect: ", 1)
    print("input:", in_values, "secure result axis=0: ", result0, ", expect: ", [-4, 2, 1])
    print("input:", in_values, "secure result axis=1: ", result1, ", expect: ", [-1, -4, 2])

    print("-----   test_reduce_min (OK) -----")

def test_reduce_max():
    print("-----   test_reduce_max -----")
    in_values = [["-1", "2", "1"], ["-4", "6", "3"], ["3", "7", "2"]] # ["2", "1", "5", "4"]
    in1 = tf.Variable(in_values, name="in1")
    a = rtt.tf_to_secure(in1, name="a")
    c_1 = rtt.secure_reduce_max(in1, reduction_indices=-1)
    c0 = rtt.secure_reduce_max(in1, reduction_indices=0)
    c1 = rtt.secure_reduce_max(in1, reduction_indices=1)

    ret_1 = rtt.secure_reveal(c_1)
    ret0 = rtt.secure_reveal(c0)
    ret1 = rtt.secure_reveal(c1)
    result_1 = create_run_session(ret_1)
    result0 = create_run_session(ret0)
    result1 = create_run_session(ret1)
    print("input:", in_values, ", secure result axis=-1: ", result_1, ", expect: ", 7)
    print("input:", in_values, "secure result axis=0: ", result0, ", expect: ", [3, 7, 3])
    print("input:", in_values, "secure result axis=1: ", result1, ", expect: ", [2, 6, 7])

    print("-----   test_reduce_max (OK) -----")

import struct
_float_precision = 13
def encode_float_to_secure(var):
    secure_vars = []
    for v in var:
        dv = float(v)
        # for cast schema of convert
        aint64 = np.uint64(np.int64(np.double(dv)*(2** _float_precision)))
        pack_d = struct.pack('@Q', aint64)
        secure_vars.append("H_" + pack_d.hex())

    print("secure vars: ", secure_vars)
    return secure_vars

def test_apply_gradient_descent():
    print("-----   test_apply_gradient_descent -----")
    # manual encode variable input to secure type
    # -1: H_0000fcffffffffff,
    # 2: H_0000080000000000

    secure_vars = ["-1", "2"]
    var = tf.Variable(secure_vars, name="var")
    delta = tf.constant(["1","2"], name="delta")
    alpha = tf.constant(0.5, name="alpha", dtype=tf.float64)
    # origin_vars = rtt.secure_to_tf(var, dtype=tf.float64)
    # b = rtt.tf_to_secure(delta)
    c = rtt.secure_apply_gradient_descent(var, alpha, delta)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, "ideal apply_gradient_descent: -2, 4: ", -1 - 0.5*1, 2 - 0.5*2)
    print("-----   test_apply_gradient_descent (OK) -----")

def test_negative():
    print("-----   test_negative -----")
    in1 = tf.Variable(["1","-2"], name="in1")
    # a = rtt.tf_to_secure(in1, name="a")
    c = rtt.secure_negative(in1)
    ret = rtt.secure_reveal(c)
    # ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(ret)
    print("secure result: ", result, ", expect: -1, 2")
    print("-----   test_negative (OK) -----")


def test_all_protocol_ops():
    test_private_input()
    # test_reveal() # pass
    # test_add() # pass
    # test_sub() # pass
    # test_mul() # pass
    # test_matmul() # pass
    # test_div() # pass
    # test_div_const() # pass

    # test_less() # pass
    # test_less_equal() # pass
    # test_equal() # pass
    # test_not_equal() # pass
    # test_greater() # pass
    # test_greater_equal() # pass
    
    # test_pow2() # pass

    #########################
    # test_log() # not pass for snn
    # test_log1p() # not pass for snn
    #########################

    # test_square() # pass
    # test_abs() # pass
    # test_abs_prime() # pass
    # test_negative() # pass
    # test_reduce_sum() # pass 
    # test_reduce_mean() # pass 
    # ## max, min ok
    # test_reduce_min() # pass
    # test_reduce_max() # pass
    # test_add_n() # pass

    # test_sigmoid() # pass
    # test_sigmoid_entropy_with_logit() # pass, helix not precise
    # test_relu() # pass
    # test_relu_prime() # pass

    # test_apply_gradient_descent()

# main
if __name__ == "__main__":
    protocol = "SecureNN"
    print("---  to activate ", protocol)

    prot_handler, cfg_content = create_init_protocol_handler(protocol, cfgfile)
    # prot_handler.set_loglevel(0) # trace, debug , info, ...
    # prot_handler.set_logfile("log/secure_op_test.log")
    test_all_protocol_ops()
    print("--- {} ops test ok ----".format(protocol))

    # protocol = "Helix"
    # print("---  to activate ", protocol)
    # prot_handler.activate(protocol, cfg_content)
    # test_all_protocol_ops()
    print("--- {} ops test ok ----".format(protocol))
