import time, sys
import numpy as np
np.set_printoptions(suppress=True)

import tensorflow as tf
# import rosetta
import latticex.rosetta as rtt
assert rtt

#rtt.activate("SecureNN")

# sys.stdout zero buffer
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

def create_run_session(target):
    print("target: ", target, ", type:", type(target))
    init = tf.global_variables_initializer()
    with tf.Session() as sess:
        sess.run(init)
        result = sess.run(target)

    return result

# binary ops
def tf_add(a, b):
    return tf.add(a, b)
def tf_sub(a, b):
    return tf.subtract(a, b)
def tf_mul(a, b):
    return tf.multiply(a, b)
def tf_div(a, b):
    return tf.div(a, b)
def tf_floor_div(a, b):
    return tf.floordiv(a, b)
def tf_matmul(a, b):
    return tf.matmul(a, b)
def tf_pow_const(a, b):
    return tf.pow(a, b)
def tf_less(a, b):
    return tf.less(a, b)
def tf_less_equal(a, b):
    return tf.less_equal(a, b)
def tf_equal(a, b):
    return tf.equal(a, b)
def tf_not_equal(a, b):
    return tf.not_equal(a, b)
def tf_greater(a, b):
    return tf.greater(a, b)
def tf_greater_equal(a, b):
    return tf.greater_equal(a, b)

binary_fns = {"add": tf_add, "sub": tf_sub,
              "mul":tf_mul, "div":tf_div, "floordiv":tf_floor_div,
              "matmul":tf_matmul, "pow":tf_pow_const,
              "less":tf_less, "less_equal":tf_less_equal,
              "equal":tf_equal, "not_equal":tf_not_equal,
              "greater":tf_greater, "greater_equal":tf_greater_equal}


# unary ops
def tf_negative(a):
    return tf.negative(a)
def tf_square(a):
    return tf.square(a)
def tf_abs(a):
    return tf.abs(a)
def tf_log(a):
    return tf.log(a)
def tf_log1p(a):
    return tf.log1p(a)
def secure_abs_prime(a):
    return rtt.SecureAbsPrime(a)

unary_fns = {"negative": tf_negative, "square": tf_square,
            "abs": tf_abs, "abs_prime": secure_abs_prime,
            "log": tf_log, "log1p": tf_log1p}


# reduce ops
def tf_reduce_max(a, axis=None, keep_dims=False):
    return tf.reduce_max(a, axis=axis, keep_dims=keep_dims)
def tf_reduce_min(a, axis=None, keep_dims=False):
    return tf.reduce_min(a, axis=axis, keep_dims=keep_dims)
def tf_reduce_mean(a, axis=None, keep_dims=False):
    return tf.reduce_mean(a, axis=axis, keep_dims=keep_dims)
def tf_reduce_sum(a, axis=None, keep_dims=False):
    return tf.reduce_sum(a, axis=axis, keep_dims=keep_dims)
def tf_reduce_add_n(a):
    return tf.add_n(a)
    # TODO: use tf.add_n
    # return tf.add_n(a)

reduce_fns = {"reduce_max": tf_reduce_max, "reduce_min": tf_reduce_min,
              "reduce_mean": tf_reduce_mean, "reduce_sum": tf_reduce_sum,
              "add_n": tf_reduce_add_n}

# nn ops
def tf_relu(a):
    return tf.nn.relu(a)
def rtt_relu_prime(a):
    return rtt.SecureReluPrime(a)
def tf_sigmoid(a):
    return tf.sigmoid(a)
def tf_sigmoid_cross_entropy_with_logits(labels, logits):
    # should use tf.nn.sigmoid_xx
    return tf.nn.sigmoid_cross_entropy_with_logits(labels=labels,logits=logits)

nn_fns = {"relu": tf_relu, "relu_prime": rtt_relu_prime,
          "sigmoid": tf_sigmoid, "sigmoid_cross_entropy_with_logits": tf_sigmoid_cross_entropy_with_logits}


def call_binary(opname, a, b, is_reveal):
    if opname in binary_fns:
        if is_reveal:
            return rtt.SecureReveal(binary_fns[opname](a, b))
        else:
            return binary_fns[opname](a, b)
def call_unary(opname, b):
    if opname in unary_fns:
        return rtt.SecureReveal(unary_fns[opname](b))
def call_reduce(opname, a, axis=None):
    if opname in reduce_fns:
        if axis:
            return rtt.SecureReveal(reduce_fns[opname](a, axis=axis))
        else:
            return rtt.SecureReveal(reduce_fns[opname](a))

def call_nn(opname, a, b=None):
    if opname in nn_fns:
        if b:
            return rtt.SecureReveal(nn_fns[opname](a, b))
        else:
            return rtt.SecureReveal(nn_fns[opname](a))

def input_variables():
    in1 = tf.Variable([[1,1], [1,1]], name="in1")
    in2 = tf.Variable([[2,2], [2,2]], name="in2")
    return in1, in2
def input_constants():
    in1 = tf.constant([[1,1], [1,1]], name="in1")
    in2 = tf.constant([[2,2], [2,2]], name="in2")
    return in1, in2

def get_binary_inputs(lh_is_const=False, rh_is_const=False, dims=2):
    if dims == 2:
        v1 = [[100000,2], [3,4]]
        v2 = [[200000,2], [2,2]]
    elif dims == 1:
        v1 = [[1, 2]]
        v2 = [[2, 2]]
    elif dims == 0:
        v1 = 1
        v2 = 2
    else:
        assert False, 'dims: {} not support!'.format(dims)

    in1 = tf.constant(v1, name="in1") if lh_is_const else tf.Variable(rtt.private_input(0,v1), name="in1")
    in2 = tf.constant(v2, name="in2") if rh_is_const else tf.Variable(rtt.private_input(0,v2), name="in2")
    return in1, in2


def get_input(is_const=False, dims=2):
    if dims == 2:
        v1 = [[1,2,3], [1,2,3]]
    elif dims == 1:
        v1 = [-1,0,3]
    else:
        v1 = -1

    in1 = tf.constant(v1, name="in1") if is_const else tf.Variable(rtt.private_input(0,v1), name="in1")
    return in1

def get_reduce_input(is_const=False, dims=2):
    if dims == 2:
        v1 = [[1, 2, 3], [1, 2, 1]]
    elif dims == 1:
        v1 = [1, 2, 3]
    else:
        v1 = 1

    in1 = tf.constant(v1, name="in1") if is_const else tf.Variable(rtt.private_input(0,v1), name="in1")
    return in1


def test_binary_op(opname, lh_is_const=False, rh_is_const=False, is_reveal=True):
    print("----- test binary op: [{}] ----".format(opname))

    in1, in2 = get_binary_inputs(lh_is_const, rh_is_const)
    c = call_binary(opname, in1, in2, is_reveal)
    # ret = rtt.rtt_reveal(c)
    result = create_run_session(c)
    print(opname, "result: ", result)

    print("-----   [{}] (OK) -----".format(opname))

def test_unary_op(opname):
    print("----- test unary op: [{}] ----".format(opname))

    in1 = get_input()
    c = call_unary(opname, in1)
    # ret = rtt.rtt_reveal(c)
    result = create_run_session(c)
    print(opname, "result: ", result)

    print("-----   [{}] (OK) -----".format(opname))

def test_reduce_op(opname, axis=None):
    print("----- test reduce op: [{}] ----".format(opname))

    in1 = get_reduce_input()
    c = call_reduce(opname, in1, axis)
    # ret = rtt.rtt_reveal(c)
    result = create_run_session(c)
    print(opname, "result: ", result)

    print("-----   [{}] (OK) -----".format(opname))

def test_nn_op(opname, arg2=None):
    print("----- test nn op: [{}] ----".format(opname))

    in1 = get_input()
    in2 = None
    if arg2:
        in2 = get_input()
    c = call_nn(opname, in1, in2)
    # ret = rtt.rtt_reveal(c)
    result = create_run_session(c)
    print(opname, "result: ", result)

    print("-----   [{}] (OK) -----".format(opname))

def test_snn_pow(lh_is_const=False, rh_is_const=True):
    print("---- test snn pow ----")
    in1 = tf.Variable([1, 2], name="in1")
    # in2 = tf.constant(["1","1"], name="in2") # not support now
    # in2 = tf.constant(["1"], name="in2") # not support now
    in2 = tf.constant(1, name="in2")
    c = tf.pow(in1, in2)#, lh_is_const=False, rh_is_const=True)
    rc = rtt.SecureReveal(c)
    #ret = rtt.secure_reveal(c)
    ## ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(rc)
    print("pow result: ", result, ", math.pow(x, 2): ", 2.0, 4.0)
    print("-----   test_pow (OK) -----")

def test_accumutive_op(opfn, N=2):
    print("---- test snn add_n ----")
    elem = ["1", "2", "3"]
    inputs = []
    for i in range(N):
        inputs.append(elem)

    c = opfn(inputs)
    rc = rtt.SecureReveal(c)
    #ret = rtt.secure_reveal(c)
    ## ret = rtt.secure_to_tf(reveal_c, dtype=tf.float64)
    result = create_run_session(rc)
    print("add result: ", result, ", math.add_n([1,2,3],....): ", 2,4,6)
    print("-----   test_add (OK) -----")

def test_const_mul():
    rtt.activate("SecureNN")
    in1 = tf.Variable([1,2], name="a")
    in2 = tf.constant([1,2], name="b")
    ret = tf.multiply(in1, in2)

    init = tf.global_variables_initializer()
    with tf.Session() as sess:
        sess.run(init)
        result = sess.run(ret)

    print(result)

def test_private_input_op(name):
    in1 = rtt.PrivateInput(tf.Variable([1,2]), data_owner=0)
    in2 = rtt.PrivateInput(tf.Variable([2,3]), data_owner=1)
    ret = rtt.SecureReveal(tf.multiply(in1, in2)) # expect [2,6]

    result = create_run_session(ret)
    print("private_input and result: ", result, ", expect: [2,6]")
    print("test_private_input ok.")

# the precision requirement of saving and restoring a float number
PRECISION = 1.0/1000

def test_all_ops(prot):
    print("--------------------- begin {}-protocol testing!  ----------------".format(prot))
    rtt.activate(prot)

    ###### binary binary ops
    print("\n\n-------------    test binary op with two variables  ----------")
    test_binary_op("add") # ok
    test_binary_op("sub") # ok
    test_binary_op("div") # ok
    test_binary_op("floordiv") # ok
    test_binary_op("mul") # ok
    test_binary_op("matmul") # ok
    return

    test_snn_pow() # ok
    test_binary_op("less") # ok
    test_binary_op("less_equal") # ok
    test_binary_op("not_equal") # ok
    test_binary_op("equal") # ok
    test_binary_op("greater") # ok
    test_binary_op("greater_equal") # ok

    ###### binary binary rh_is_const=True ops, mark [OK]
    print("\n\n-------------    test binary op with const rh_is_const=True  ----------")
    test_binary_op("add", False, True)
    test_binary_op("sub", False, True)
    test_binary_op("div", False, True)
    test_binary_op("mul", False, True)
    test_binary_op("matmul", False, True)
    test_binary_op("less", False, True)
    test_binary_op("less_equal", False, True)
    test_binary_op("not_equal", False, True)
    test_binary_op("equal", False, True)
    test_binary_op("greater", False, True)
    test_binary_op("greater_equal", False, True)

    ###### binary binary lh_is_const=True ops, mark [ok]
    print("\n\n-------------    test binary op with const lh_is_const=True  ----------")
    test_binary_op("add", True, False)
    test_binary_op("sub", True, False)
    test_binary_op("div", True, False)
    test_binary_op("mul", True, False)
    test_binary_op("matmul", True, False)
    test_binary_op("less", True, False)
    test_binary_op("less_equal", True, False)
    test_binary_op("not_equal", True, False)
    test_binary_op("equal", True, False)
    test_binary_op("greater", True, False)
    test_binary_op("greater_equal", True, False)

    ###### binary binary lh_is_const=True, rh_is_const=True ops, mark [todo]
    print("\n\n-------------    test binary op with const,const  ----------")
    test_binary_op("add", True, True, False)
    test_binary_op("sub", True, True, False)
    test_binary_op("div", True, True, False)
    test_binary_op("mul", True, True, False)
    test_binary_op("matmul", True, True, False)
    test_binary_op("less", True, True, False)
    test_binary_op("less_equal", True, True, False)
    test_binary_op("not_equal", True, True, False)
    test_binary_op("equal", True, True, False)
    test_binary_op("greater", True, True, False)
    test_binary_op("greater_equal", True, True, False)

    ###### unary ops
    test_unary_op("negative") # ok
    test_unary_op("square") # ok
    test_unary_op("abs") # ok
    test_unary_op("abs_prime") # ok
    test_unary_op("log")
    test_unary_op("log1p")

    test_reduce_op("reduce_max", 1) # reduction_indices=1, ok
    test_reduce_op("reduce_min", 1) # ok
    test_reduce_op("reduce_mean", 1) # ok
    test_reduce_op("reduce_sum", 1) # ok

    test_reduce_op("reduce_max", 0) # reduction_indices=1, ok
    test_reduce_op("reduce_min", 0) # ok
    test_reduce_op("reduce_mean", 0) # ok
    test_reduce_op("reduce_sum", 0) # ok

    test_reduce_op("reduce_max", -1) # reduction_indices=1, ok
    test_reduce_op("reduce_min", -1) # ok
    test_reduce_op("reduce_mean", -1) # ok
    test_reduce_op("reduce_sum", -1) # ok

    test_accumutive_op(rtt.SecureAddN, 2) # ok

    test_nn_op("relu") # ok
    test_nn_op("relu_prime") # ok
    test_nn_op("sigmoid_cross_entropy_with_logits", "with_logits") # ok
    test_nn_op("sigmoid") # ok

    # test io ops
    test_private_input_op("private_input")

    print("--------------------- end {}-protocol testing!  ----------------".format(prot))

# choose protocol
protocol="SecureNN" # Helix
import os
if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    print("***** secure_tests uses ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
    rtt.activate(protocol)
else:
    print("***** secure_tests uses default helix protocol ")

# SecureNN test
test_all_ops(protocol)

# # Helix test
#protocol = "Helix"
#test_all_ops(protocol)
rtt.deactivate()
print("----- ending ---- ")
