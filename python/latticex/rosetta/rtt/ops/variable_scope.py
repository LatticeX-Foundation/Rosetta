# ==============================================================================
# Copyright 2020 The LatticeX Foundation
# This file is part of the Rosetta library.
#
# The Rosetta library is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Rosetta library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
# =============================================================================="
import tensorflow as tf
import numpy as np
from tensorflow.python.ops import resource_variable_ops
from tensorflow.python.ops import variables
from tensorflow.python.ops import variable_scope
from tensorflow.python.eager import context
from tensorflow.python.framework import ops
from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts


def convert_init_value_to_string(initial_value, dtype):
    if callable(initial_value):
        initial_value = initial_value()
        
    if isinstance(initial_value, ops.Tensor):
        dtype = initial_value.dtype

    if (dtype != tf.string and dtype != None):
        initial_value = tf.as_string(initial_value)
    elif isinstance(initial_value, np.ndarray):
        if initial_value.dtype.type != np.str and initial_value.dtype.type != np.object_ and initial_value.dtype.type != np.bytes_:
            initial_value = tf.as_string(initial_value)
    elif isinstance(initial_value, (list, tuple)):
        if (type("") != type(initial_value[0])):
            initial_value = tf.as_string(initial_value)
    elif isinstance(initial_value, (float, int)):
        initial_value = tf.as_string(initial_value)
    
    return initial_value


def Rtt_default_variable_creator(next_creator=None, **kwargs):
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
    
    initial_value = convert_init_value_to_string(initial_value, dtype)

    if use_resource is None:
        use_resource = variable_scope.get_variable_scope().use_resource
    if use_resource is None:
        use_resource = variable_scope._DEFAULT_USE_RESOURCE
    use_resource = use_resource or context.executing_eagerly()
    if use_resource:
        distribute_strategy = kwargs.get("distribute_strategy", None)
        return rtt_ts.convert_to_rtttensor(resource_variable_ops.ResourceVariable(
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


def Rtt_default_variable_creator_v2(next_creator=None, **kwargs):
    """Default variable creator."""
    assert next_creator is None
    initial_value = kwargs.get("initial_value", None)
    trainable = kwargs.get("trainable", None)
    validate_shape = kwargs.get("validate_shape", True)
    caching_device = kwargs.get("caching_device", None)
    name = kwargs.get("name", None)
    variable_def = kwargs.get("variable_def", None)
    dtype = kwargs.get("dtype", None)
    import_scope = kwargs.get("import_scope", None)
    constraint = kwargs.get("constraint", None)
    distribute_strategy = kwargs.get("distribute_strategy", None)
    synchronization = kwargs.get("synchronization", None)
    aggregation = kwargs.get("aggregation", None)
    shape = kwargs.get("shape", None)

    initial_value = convert_init_value_to_string(initial_value, dtype)

    return rtt_ts.convert_to_rtttensor(resource_variable_ops.ResourceVariable(
        initial_value=initial_value,
        trainable=trainable,
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


# override default_variable_creator RefVariable & ResourceVariable for RTT
variables.default_variable_creator = Rtt_default_variable_creator
variables.default_variable_creator_v2 = Rtt_default_variable_creator_v2
