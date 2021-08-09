#!/usr/bin/python

import tensorflow as tf

import sys, os
import numpy as np
np.set_printoptions(suppress=True)

protocol="Helix"

if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
    import latticex.rosetta as rtt
    print("***** test_cases use ", os.environ["ROSETTA_TEST_PROTOCOL"])
    protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
    rtt.activate(protocol)
else:
    print("***** test_cases use default helix protocol ")

test_case_array = [
    # 0-d
    # [
    #     0.1
    # ],
    # 1-d: 1*1, 1*2, 1*3
    # [
    #     [1.1]
    # ],
    # [
    #     [1.1, 1.2]
    # ],
    # [
    #     [1.1, 1.2, 1.3]
    # ],
    # 2-d (1)
    [
        [[2.1]]
    ],
    [
        [[2.1, 2.2]]
    ],
    [
        [[2.1, 2.2, 2.3]]
    ],
    # 2-d (2)
    [
        [[2.1], [2.2]]
    ],
    [
        [[2.1], [2.2], [2.3]]
    ],
    [
        [[2.7, 2.2], [2.3, 2.4]]
    ],
    [
        [[2.1, 2.8], [2.7, 2.4], [2.5, 2.6]]
    ],
    # 2-d (3)
    [
        [[2.1, 2.7, 2.3], [2.4, 2.5, 2.6]]
    ],
    [
        [[2.9, 2.2, 3.3], [2.4, 2.8, 2.6], [2.7, 2.5, 3.4]]
    ],
]

test_case_array2 = [
    # 0-d
    [
        0.54
    ],
    # 1-d
    [
        [-1.64]
    ],
    [
        [1.62, 0.2364]
    ],
    [
        [-1.51, 1.082, 1.03]
    ],
    # 2-d (1)
    [
        [[2.16]]
    ],
    [
        [[2.199, -2.2292]]
    ],
    [
        [[-2.1, 2.002, 2.03]]
    ],
    # 2-d (2)
    [
        [[2.10231], [-1.2]]
    ],
    [
        [[-2.01], [-2.52], [2.13]]
    ],
    [
        [[2.881, -1.211], [-2.3, 0.854]]
    ],
    [
        [[0.1, -2.12], [2.33, 2.54], [-2.25, 0.6]]
    ],
    # 2-d (3)
    [
        [[1.1, 0.342, -2.63], [2.4, 0.555, 2.6]]
    ],
    [
        [[-2.1, 1.22, 2.2], [1.034, 1.2245, 2.6], [-0.669, 0.8, -2.9]]
    ],
]


def numpy_dims_test():
    size = len(test_case_array)
    idx = 0
    for i in range(size):
        a = test_case_array[i][0]
        for j in range(i, size):
            b = test_case_array[j][0]
            print(i, a)
            print(j, b)
            idx += 1

            try:
                c = np.array(a) * np.array(b)
                print("case ", idx, " NUMPY RES OK:", i, j,
                      np.array(a).shape, np.array(b).shape, np.array(c).shape)
            except Exception as e:
                print("case ", idx, " NUMPY RES NO:", i, j,
                      np.array(a).shape, np.array(b).shape)
                print("Exception :", i, j, e)


def __test_unary_op(op_functor, msg, xa):
    # log/log1p/...
    pass


def unary_op_test(op_functor, msg):
    size = len(test_case_array)
    for i in range(size):
        a = test_case_array[i][0]
        # print(i, a)
        xa = tf.Variable(a)

        try:
            __test_unary_op(op_functor, msg, xa)
            print("case ", i, " UNARY RES OK:", i, np.array(a).shape)
        except Exception as e:
            print("case ", i, " UNARY RES OK:", i, np.array(a).shape)
            print("Exception :", i,  e)


def __test_reduce_op(op_functor, msg, xa, axis=None):
    # max/mean/sum/...
    init = tf.compat.v1.global_variables_initializer()
    sess = tf.compat.v1.Session()
    sess.run(init)

    ###########
    # print("=========================== ", msg, " 1 beg")
    # xc = op_functor(xa, axis=axis)
    # xcc = sess.run(xc)
    # print(xcc)
    # print("=========================== ", msg, " 1 end")

    # axis=None,keep_dims=False,name=None,reduction_indices=None
    print("=========================== ", msg, "BEGIN")
    xc = op_functor(xa, axis=axis)
    #xd = op_functor(xa, axis=axis)
    #xe = op_functor(xa, axis=axis)
    xee = sess.run([xa, xc])
    result = []
    # to support both native TF OP and Rosetta OP test
    if 'latticex.rosetta' in sys.modules:
        import latticex.rosetta as rtt
        reveal_input = sess.run(rtt.SecureReveal(xee[0]))
        reveal_output = sess.run(rtt.SecureReveal(xee[1]))
        print("revealed input:", reveal_input)
        print("revealed output:", reveal_output)
        result = np.array(reveal_output, dtype = np.float)
    else:
        print("input:",xee[0])
        print("output :",xee[1])
        result = np.array(xee[1], dtype = np.float)
    print("=========================== ", msg, " END")
    return result

def reduce_op_test(op_functor, msg, axis=None):
    size = len(test_case_array)
    whole_result=np.array([])
    for i in range(size):
        a = test_case_array[i][0]
        print(i, a)
        xa = tf.Variable(a)

        try:
            this_res = __test_reduce_op(op_functor, msg, xa, axis)
            whole_result = np.append(whole_result, this_res.flatten())
            print("case ", i, " REDUCEOP RES OK:", i, np.array(a).shape)
        except Exception as e:
            print("case ", i, " REDUCEOP RES OK:", i, np.array(a).shape)
            print("Exception :", i,  e)
    return whole_result

def __test_binary_op(op_functor, msg, xa, xb):
    # Add/Sub/Mul/Div/Equal/Greater/Less/...
    init = tf.compat.v1.global_variables_initializer()
    sess = tf.compat.v1.Session()
    sess.run(init)

    print("=========================== ", msg, " 1 beg")
    xc = op_functor(xa, xb)
    xcc = sess.run(xc)
    print(xcc)
    print("=========================== ", msg, " 1 end")

    print("=========================== ", msg, " 2 beg")
    xc = op_functor(xa, xb)
    xd = op_functor(xa, xc)
    xe = op_functor(xa, xd)
    xee = sess.run([xa, xb, xc, xd, xe])
    print(xee)
    print("=========================== ", msg, " 2 end")

    sess.close()


def binary_op_test(op_functor, msg):
    size = len(test_case_array)
    idx = 0
    for i in range(size):
        a = test_case_array[i][0]
        for j in range(i, size):
            b = test_case_array[j][0]
            shape_a = np.array(a).shape
            shape_b = np.array(b).shape
            shape_single = (1, 1)
            if (shape_a != shape_b and shape_a != shape_single and shape_b != shape_single):
                continue
            print(i, a)
            print(j, b)
            xa = tf.Variable(a)
            xb = tf.Variable(b)
            idx += 1

            try:
                __test_binary_op(op_functor, msg, xa, xb)
                print("case ", idx, " BINARYOP RES OK:", i,
                      j, np.array(a).shape, np.array(b).shape)
            except Exception as e:
                print("case ", idx, " BINARYOP RES NO:", i,
                      j, np.array(a).shape, np.array(b).shape)
                print("Exception :", i, j, e)


def print_inputs():
    for i in range(len(test_case_array)):
        a = test_case_array[i][0]
        print("a:", a)


if __name__ == "__main__":
    print_inputs()
    numpy_dims_test()
