import tensorflow as tf 
import sys
sys.path.append("./")
sys.path.append("./../")
sys.path.append("./../../")
sys.path.append("./../../framework")
import rtt_tensor
import rtt_math_ops
import rtt_utils_for_test


def unary_op(x, op_type):
    if op_type.lower() == "neg":
        z1 = -x
        z2 = tf.negative(x)
    elif op_type.lower() == "square":
        z2 = tf.square(x)
    elif op_type.lower() == "log":
        z2 = tf.log(x)
    elif op_type.lower() == "log1p":
        z2 = tf.log1p(x)
    elif op_type.lower() == "sigmoid":
        z2 = tf.sigmoid(x)
    elif op_type.lower() == "relu":
        z2 = tf.nn.relu(x)
    elif op_type.lower() == "abs":
        z1 = abs(x)
        z2 = tf.abs(x)
    else:
        raise ValueError( "unkown the unary operation '{}'".format(op_type))


def test_unary(op_type):
    # 1. var
    x1 = tf.Variable([1.], name='x1')
    unary_op(x1, op_type)

    # 2. constant
    x2 = tf.constant([1.], name='x2')
    unary_op(x2, op_type)

    # 3. placeholder
    x3 = tf.placeholder(dtype=tf.float64, shape=(1), name="x3")
    unary_op(x3, op_type)

    # 4. scalar
    unary_op(4, op_type)


def binary_op(x, y, op_type):
    if op_type.lower() == "add":
        z1 = x + y
        z2 = tf.add(x, y)
    elif op_type.lower() == "sub":
        z1 = x - y
        z2 = tf.subtract(x, y)
    elif op_type.lower() == "mul":
        z1 = x * y
        z2 = tf.multiply(x, y)
    elif op_type.lower() == "div":
        z1 = x / y
        z2 = tf.div(x, y)
    elif op_type.lower() == "floordiv":
        z1 = x // y
        z2 = tf.floor_div(x, y)
    elif op_type.lower() == "truediv":
        z1 = x / y
        z2 = tf.true_div(x, y)
    elif op_type.lower() == "realdiv":
        z2 = tf.real_div(x, y)
    elif op_type.lower() == "equal":
        z1 = (x == y)
        z2 = tf.equal(x, y)
    elif op_type.lower() == "notequal":
        z1 = (x != y)
        z2 = tf.not_equal(x, y)
    elif op_type.lower() == "greater":
        z1 = (x > y)
        z2 = tf.greater(x, y)
    elif op_type.lower() == "greaterequal":
        z1 = (x >= y)
        z2 = tf.greater_equal(x, y)
    elif op_type.lower() == "less":
        z1 = (x < y)
        z2 = tf.less(x, y)
    elif op_type.lower() == "lessequal":
        z1 = (x <= y)
        z2 = tf.less_equal(x, y)
    elif op_type.lower() == "matmul":
        z1 = x @ y
        z2 = tf.matmul(x, y)
    elif op_type.lower() == "pow":
        z1 = x ** y
        z2 = tf.pow(x, y)
    
    else:
        raise ValueError( "unkown the binary operation '{}'".format(op_type))


def test_binary(op_type):
    # 1. var vs var
    x1 = tf.Variable([[1., 1.], [1., 1.]], name='x1')
    y1 = tf.Variable([[2., 2.], [2., 2.]], name='y1')
    binary_op(x1, y1, op_type)

    # 2. var vs constant
    # 3. constant vs var
    x2 = tf.Variable([[1., 1.], [1., 1.]], name='x2')
    y2 = tf.constant([[2., 2.], [2., 2.]], name='y2')
    binary_op(x2, y2, op_type)
    binary_op(y2, x2, op_type)

    # 4. var vs placeholder
    # 5. placeholder vs var
    x4 = tf.Variable([[1., 1.], [1., 1.]], name='x4')
    y4 = tf.placeholder(dtype=tf.float64, shape=(2, 2), name="y4")
    binary_op(x4, y4, op_type)
    binary_op(y4, x4, op_type)

    # 6. var vs scalar
    # 7. scalar vs var
    x6 = tf.Variable([[1., 1.], [1., 1.]], name='x6')
    binary_op(x6, [[6, 6], [6, 6]], op_type)
    binary_op([[6, 6], [6, 6]], x6, op_type)


def reduce_op(x, axis=None, op_type=None):
    if op_type.lower() == "min":
        z1 = tf.reduce_min(x, axis)
    elif op_type.lower() == "max":
        z1 = tf.reduce_max(x, axis)
    elif op_type.lower() == "sum":
        z1 = tf.reduce_sum(x, axis)
    elif op_type.lower() == "mean":
        z1 = tf.reduce_mean(x, axis)
    else:
        raise ValueError("unkown the reduce operation '{}'".format(op_type))


def test_reduce(op_type):
    x1 = tf.Variable([[1., 1.], [1., 1.]], name='x1')
    reduce_op(x1, op_type=op_type)
    reduce_op(x1, axis=0, op_type=op_type)
    reduce_op(x1, axis=1, op_type=op_type)
    reduce_op(x1, axis=[0,1], op_type=op_type)


#-------------------
# test for unary op
#-------------------
test_unary("neg")
test_unary("square")
test_unary("log")
test_unary("log1p")
test_unary("sigmoid")
test_unary("relu")
test_unary("abs")


#-------------------
# test for binary op
#-------------------
test_binary("add")
test_binary("sub")
test_binary("mul")
test_binary("div")
test_binary("floordiv")
test_binary("truediv")
test_binary("realdiv")
test_binary("equal")
test_binary("notequal")
test_binary("greater")
test_binary("greaterequal")
test_binary("less")
test_binary("lessequal")
test_binary("matmul")
test_binary("pow")


#-------------------
# test for reduce op
#-------------------
test_reduce("min")
test_reduce("max")
test_reduce("sum")
test_reduce("mean")



Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()


