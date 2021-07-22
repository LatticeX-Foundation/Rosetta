import tensorflow as tf
from tensorflow.python.ops import variables
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import variable_scope
from tensorflow.python.eager import context
from tensorflow.python.framework import ops
from tensorflow.python.framework import constant_op
from tensorflow import dtypes, placeholder, zeros, ones, zeros_like, ones_like
import rtt_tensor as rtt_ts
import numpy as np
import pandas as pd


def get_var_from_rtt_tensor(rtt_tensor):
    op_name = rtt_tensor._raw.op.inputs[0].op.inputs[0].op.name
    for var in tf.global_variables():
        if (var.op.name == op_name):
            return var


def dtype_check_and_set(t):
    new_dtype = t
    if t in [None, float, dtypes.float16, dtypes.float32, dtypes.float64]:
        new_dtype = dtypes.string
    return new_dtype


def RttPlaceholder(dtype, shape=None, name=None):
    dtype = dtype_check_and_set(dtype)
    return rtt_ts.placeholder(array_ops.placeholder(dtype, shape, name))


def RttConstant(value, dtype=None, shape=None, name="Const"):
    if (dtype != tf.string and dtype != None):
        value = str(value)
    elif isinstance(value, (list, tuple)):
        if (type(str) != type(value[0])):
            value = str(value)

    dtype = dtype_check_and_set(dtype)
    return rtt_ts.constant(constant_op.constant(value, dtype, shape, name))


class RttRefVariable(variables.RefVariable):
    def __init__(self, *args, **kwargs):
        kwargs['dtype'] = dtype_check_and_set(kwargs.get('dtype', None))
        super(RttRefVariable, self).__init__(*args, **kwargs)


def Rtt_default_variable_creator(next_creator=None, **kwargs):
    # return rtt_ts.convert_to_tensor(variable_scope.default_variable_creator(next_creator, **kwargs))

    """Default variable creator."""
    assert next_creator is None
    initial_value = kwargs.get("initial_value", None)
    trainable = kwargs.get("trainable", None)
    collections = kwargs.get("collections", None)
    validate_shape = kwargs.get("validate_shape", True)
    caching_device = kwargs.get("caching_device", None)
    name = kwargs.get("name", None)
    variable_def = kwargs.get("variable_def", None)
    dtype = kwargs.get("dtype", None)
    expected_shape = kwargs.get("expected_shape", None)
    import_scope = kwargs.get("import_scope", None)
    constraint = kwargs.get("constraint", None)
    use_resource = kwargs.get("use_resource", None)
    synchronization = kwargs.get("synchronization", None)
    aggregation = kwargs.get("aggregation", None)
    shape = kwargs.get("shape", None)
    
    if isinstance(initial_value, ops.Tensor):
        dtype = initial_value.dtype
    if (dtype != tf.string and dtype != None):
        initial_value = tf.as_string(initial_value)
    elif isinstance(initial_value, (list, tuple)):
        if (type(str) != type(initial_value[0])):
            initial_value = tf.as_string(initial_value)


    if use_resource is None:
        use_resource = variable_scope.get_variable_scope().use_resource
    if use_resource is None:
        use_resource = variable_scope._DEFAULT_USE_RESOURCE
    use_resource = use_resource or context.executing_eagerly()
    if use_resource:
        distribute_strategy = kwargs.get("distribute_strategy", None)
        return rtt_ts.convert_to_rtttensor(resource_variable_ops.ResourceVariable(
        # return resource_variable_ops.ResourceVariable(
            initial_value=initial_value,
            trainable=trainable,
            collections=collections,
            validate_shape=validate_shape,
            caching_device=caching_device,
            name=name,
            dtype=dtype,
            constraint=constraint,
            variable_def=variable_def,
            import_scope=import_scope,
            distribute_strategy=distribute_strategy,
            synchronization=synchronization,
            aggregation=aggregation,
            shape=shape))
    else:
        return rtt_ts.convert_to_rtttensor(variables.RefVariable(
        # return variables.RefVariable(
            initial_value=initial_value,
            trainable=trainable,
            collections=collections,
            validate_shape=validate_shape,
            caching_device=caching_device,
            name=name,
            dtype=dtype,
            constraint=constraint,
            variable_def=variable_def,
            expected_shape=expected_shape,
            import_scope=import_scope,
            synchronization=synchronization,
            aggregation=aggregation,
            shape=shape))

# override RefVariable & ResourceVariable class for RTT
variables.RefVariable = RttRefVariable

# override default_variable_creator RefVariable for RTT
variables.default_variable_creator = Rtt_default_variable_creator

# override placeholder functions for RTT
tf.placeholder = RttPlaceholder

# override constant functions for RTT
tf.constant = RttConstant

