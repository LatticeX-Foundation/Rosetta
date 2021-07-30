import numpy as np

####################   sample datas ##################
# samples for unary or reduce ops
sample_test_data_array = [
    # 0-d
    [
        0.1
    ],
    # 1-d
    [
        [1.1]
    ],
    [
        [1.1, 1.2, 1.3]
    ],
    # 2-d (1)
    [
        [[2.1]]
    ],
    [
        [[2.1], [2.2], [2.3]]
    ],
    [
        [[2.1, 2.2, 2.3]]
    ],
    # 2-d (2+)
    [
        [[2.1, 2.2], [2.3, 2.4], [3.2, 3.4]]
    ],
    # 2-d (3+)
    [
        [[2.1, 2.2, 2.3], [2.4, 2.5, 2.6], [2.4, 2.8, -2.6]]
    ],
    [
        [[2.1, 2.2, 2.3], [2.4, 2.5, 2.6], [2.7, 2.8, 2.9]]
    ],
    [
        [[2.1, 2.2], [2.3, 2.4], [2.5, 2.6], [2.2, 2.8]]
    ],
]

# samples for binary ops, should have two elems
sample_test_data_array2 = [
    #p0 (a0, b0), p1(a1, b1)
    [
        [-1.64, 1.23]
    ],
    # 2-d (1)
    [
        [[2.16], [-2.9]]
    ],
    [
        [[2.199, -2.2292], [2.199, -2.2292]]
    ],
    [
        [[-2.1, 2.002, 2.03], [-2.3, 2.4, 2.03]]
    ],
    # 2*3 matrix
    [
        [[[-2.1, 1.22, 2.2], [1.034, 1.2245, 2.6], [2, 5, 6]],
            [[1, 2, 3], [1, 2, 3], [1, 2, 3]]]
    ],
]

# samples for binary ops, should have two elems
sample_test_data_array_const2 = [
    #p0 (a0, b0), p1(a1, b1)
    [
        [2.5, 1]
    ],
    # 2-d (1)
    [
        [[2.16], [2]]
    ],
    [
        # a=[2.199, -2.2292], b=[1,2] (b is const)
        [[2.199, 2.42], [1.224, 2.1]]
    ],
    [
        [[-2.1, 2.002, 2.03], [1, 2, 3]]
    ],
    # 2*3 matrix
    [
        [[[-2.1, 1.22, 2.2], [1.034, 1.2245, 2.6], [2, 5, 6]],
         [[1, 2, 3],          [1, 2, 3],            [1, 2, 3]]]
    ],
]
####################################

def _is_round_equal(test_case, x, y, loss_precision=0.3):
    state = False
    if loss_precision == None:
        # absolute(`a` - `b`) <= (`atol` + `rtol` * absolute(`b`))
        state = np.allclose(x, y, rtol=0, atol=0.1)  # no more than 0.5
    else:
        state = np.allclose(x, y, rtol=0, atol=loss_precision)
    return state

######   sample test methods:  5 now for binary op test #######
def _test_binary_sample_call(test_case, index, loss_precision=0.1):
    if index < len(sample_test_data_array2) and len(sample_test_data_array2[index]) == 1:
        inputs = sample_test_data_array2[index]
        print("inputs =>: ", inputs)
        in0 = inputs[0]
        in1 = np.zeros(shape=np.double(in0).shape).tolist(
        ) if isinstance(in0, list) else 0

        # start p0,p1, p2
        outputs = test_case.run_players(in0, in1, test_case.op_name)
        # 0: lh_isconst=True, 1: rh_isconst=True, -1: default and both False
        expect_outputs = test_case.run_local(in0, in1, test_case.op_name)

        print("expect-outputs: {}, type: {}".format(expect_outputs, type(expect_outputs)))
        print("mpc-outputs: {}, type: {}".format(outputs, type(outputs)))

        #test_case.assertEqual(outputs.tolist(), expect_outputs.tolist())
        if loss_precision != None:
            if _is_round_equal(test_case, outputs, expect_outputs, loss_precision):
                print("round equal: {} ~= {}".format(
                    outputs, expect_outputs))
            else:
                test_case.assertEqual(
                    outputs, expect_outputs)
        else:
            test_case.assertEqual(outputs, expect_outputs)
        print("--------- test {}: sample {} done".format(test_case.op_name, inputs))


def _test_binary_const_sample_call(test_case, index, lh_is_const=False, rh_is_const=True, loss_precision=0.1):
    if index < len(sample_test_data_array_const2) and len(sample_test_data_array_const2[index]) == 1:
        inputs = sample_test_data_array_const2[index]
        print("inputs =>: ", inputs)
        in0 = inputs[0]  # all remain the same

        in10 = in0[0] if lh_is_const else np.zeros(shape=np.double(
            in0[0]).shape).tolist() if isinstance(in0[0], list) else 0
        in11 = in0[1] if rh_is_const else np.zeros(shape=np.double(
            in0[1]).shape).tolist() if isinstance(in0[1], list) else 0
        in1 = in0  # [in10, in11]
        in_zeros = np.zeros(shape=np.double(in0[1]).shape).tolist()
        print("in0: {}".format(in0))
        print("in1: {}".format(in1))

        # start p0,p1, p2
        is_valid_const_pos = False
        const_pos = 0
        if lh_is_const == True and rh_is_const == False:
            is_valid_const_pos = True
            const_pos = 0
        elif lh_is_const == False and rh_is_const == True:
            is_valid_const_pos = True
            const_pos = 1
        else:
            assert is_valid_const_pos, "const utest required either pos_0 or pos_1 with const variable"
            sys.exit()

        outputs = test_case.run_players(
            in0, in1, test_case.op_name, const_pos=const_pos)
        # 0: lh_isconst=True, 1: rh_isconst=True, -1: default and both False
        expect_outputs = test_case.run_local(
            in0, in_zeros, test_case.op_name, const_pos=const_pos)

        if loss_precision != None:
            if _is_round_equal(test_case, outputs, expect_outputs, loss_precision):
                print("round equal: {} ~= {}".format(
                    outputs, expect_outputs))
            else:
                test_case.assertEqual(
                    outputs, expect_outputs)
        else:
            test_case.assertEqual(outputs, expect_outputs)
            # test_case.assertEqual(outputs, expect_outputs)

        print("expect-outputs: {}, type: {}".format(expect_outputs, type(expect_outputs)))
        print("mpc-outputs: {}, type: {}".format(outputs, type(outputs)))
        print("--------- test {}: sample {} done".format(test_case.op_name, inputs))


def test_binary_sample_0(test_case, loss_precision=0.1):
    _test_binary_sample_call(test_case, index=0, loss_precision=loss_precision)


def test_binary_sample_1(test_case, loss_precision=0.1):
    _test_binary_sample_call(test_case, index=1, loss_precision=loss_precision)


def test_binary_sample_2(test_case, loss_precision=0.1):
    _test_binary_sample_call(test_case, index=2, loss_precision=loss_precision)


def test_binary_sample_3(test_case, loss_precision=0.1):
    _test_binary_sample_call(test_case, index=3, loss_precision=loss_precision)


def test_binary_sample_4(test_case, loss_precision=0.1):
    _test_binary_sample_call(test_case, index=4, loss_precision=loss_precision)


def test_binary_const_sample_0(test_case, lh_is_const=False, rh_is_const=True, loss_precision=0.1):
    _test_binary_const_sample_call(test_case, index=0, lh_is_const=lh_is_const,
                                   rh_is_const=rh_is_const, loss_precision=loss_precision)


def test_binary_const_sample_1(test_case, lh_is_const=False, rh_is_const=True, loss_precision=0.1):
    _test_binary_const_sample_call(test_case, index=1, lh_is_const=lh_is_const,
                                   rh_is_const=rh_is_const, loss_precision=loss_precision)


def test_binary_const_sample_2(test_case, lh_is_const=False, rh_is_const=True, loss_precision=0.1):
    _test_binary_const_sample_call(test_case, index=2, lh_is_const=lh_is_const,
                                   rh_is_const=rh_is_const, loss_precision=loss_precision)


def test_binary_const_sample_3(test_case, lh_is_const=False, rh_is_const=True, loss_precision=0.1):
    _test_binary_const_sample_call(test_case, index=3, lh_is_const=lh_is_const,
                                   rh_is_const=rh_is_const, loss_precision=loss_precision)


def test_binary_const_sample_4(test_case, lh_is_const=False, rh_is_const=True, loss_precision=0.1):
    _test_binary_const_sample_call(test_case, index=4, lh_is_const=lh_is_const,
                                   rh_is_const=rh_is_const, loss_precision=loss_precision)

#####    sample test methods: 10 now for unary or reduce op test ####
def _test_unary_reduce_sample_call(test_case, index, is_reduce=False, loss_precision=0.1):
    if index < len(sample_test_data_array):
        inputs = sample_test_data_array[index]
        print("inputs =>: ", inputs)
        in0 = inputs[0]
        # np.zeros(shape=np.double(in0).shape).tolist() if isinstance(in0, list) else  0
        in1 = in0
        zeros_in1 = np.zeros(shape=np.double(
            in0).shape).tolist() if isinstance(in0, list) else 0
        # start p0,p1, p2
        outputs = test_case.run_players(in0, in1, test_case.op_name)
        expect_outputs = test_case.run_local(in0, zeros_in1, test_case.op_name)

        print("**local-outputs: {}, type: {}".format(expect_outputs, type(expect_outputs)))
        print("**mpc-outputs: {}, type: {}".format(outputs, type(outputs)))

        #test_case.assertEqual(outputs, expect_outputs)
        if _is_round_equal(test_case, outputs, expect_outputs, loss_precision):
            print("round equal: {} ~= {}".format(
                outputs, expect_outputs))
        else:
            test_case.assertEqual(
                outputs, expect_outputs)
        # else:
        #   # if (is_reduce and len())
        #   test_case.assertEqual(outputs, expect_outputs)

        print("--------- test {}: sample {} done".format(test_case.op_name, inputs))


def test_unary_reduce_sample_0(test_case, is_reduce=False, loss_precision=None):
    _test_unary_reduce_sample_call(
        test_case, index=0, is_reduce=is_reduce, loss_precision=loss_precision)


def test_unary_reduce_sample_1(test_case, is_reduce=False, loss_precision=None):
    _test_unary_reduce_sample_call(
        test_case, index=1, is_reduce=is_reduce, loss_precision=loss_precision)


def test_unary_reduce_sample_2(test_case, is_reduce=False, loss_precision=None):
    _test_unary_reduce_sample_call(
        test_case, index=2, is_reduce=is_reduce, loss_precision=loss_precision)


def test_unary_reduce_sample_3(test_case, is_reduce=False, loss_precision=None):
    _test_unary_reduce_sample_call(
        test_case, index=3, is_reduce=is_reduce, loss_precision=loss_precision)


def test_unary_reduce_sample_4(test_case, is_reduce=False, loss_precision=None):
    _test_unary_reduce_sample_call(
        test_case, index=4, is_reduce=is_reduce, loss_precision=loss_precision)


def test_unary_reduce_sample_5(test_case, is_reduce=False, loss_precision=None):
    _test_unary_reduce_sample_call(
        test_case, index=5, is_reduce=is_reduce, loss_precision=loss_precision)


def test_unary_reduce_sample_6(test_case, is_reduce=False, loss_precision=None):
    _test_unary_reduce_sample_call(
        test_case, index=6, is_reduce=is_reduce, loss_precision=loss_precision)


def test_unary_reduce_sample_7(test_case, is_reduce=False, loss_precision=None):
    _test_unary_reduce_sample_call(
        test_case, index=7, is_reduce=is_reduce, loss_precision=loss_precision)


def test_unary_reduce_sample_8(test_case, is_reduce=False, loss_precision=None):
    _test_unary_reduce_sample_call(
        test_case, index=8, is_reduce=is_reduce, loss_precision=loss_precision)


def test_unary_reduce_sample_9(test_case, is_reduce=False, loss_precision=None):
    _test_unary_reduce_sample_call(
        test_case, index=9, is_reduce=is_reduce, loss_precision=loss_precision)
