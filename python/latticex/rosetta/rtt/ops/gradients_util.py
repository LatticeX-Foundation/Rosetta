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
from tensorflow.python.ops import gradients_util, array_ops
from tensorflow.python.framework import ops
from tensorflow.python.framework.ops import convert_n_to_tensor_or_indexed_slices
from tensorflow.python.framework.ops import Tensor
from tensorflow.python.framework.ops import IndexedSlices
from tensorflow.python.framework import dtypes
from tensorflow import constant
from tensorflow.python.framework import constant_op



def RttDefaultGradYs(grad_ys,
                   ys,
                   colocate_gradients_with_ops,
                   gradient_uid="__unsupported__"):
    if len(grad_ys) != len(ys):
        raise ValueError("Passed %d grad_ys for %d ys" % (len(grad_ys), len(ys)))
    grad_ys = convert_n_to_tensor_or_indexed_slices(grad_ys, name="grad_y")
    new_grad_ys = []
    for i in gradients_util.xrange(len(grad_ys)):
        grad_y = grad_ys[i]
        y = ys[i]
        with gradients_util._maybe_colocate_with(y.op, gradient_uid, colocate_gradients_with_ops):
            if grad_y is None:
                if y.dtype.is_complex:
                    raise TypeError(
                        "Gradients of complex tensors must set grad_ys (y.dtype = %r)" %
                        y.dtype)
                # NOTE(George): We set the default value as string "1.0".
                new_grad_ys.append(
                    array_ops.fill(
                        array_ops.shape(y),
                        constant_op.constant("1.0", dtype=y.dtype, name="grad_ys_%d" % i)))
                continue
            if y.dtype.is_floating or y.dtype.is_integer:
                if not grad_y.dtype.is_floating and not grad_y.dtype.is_integer:
                    raise TypeError(
                        "Gradient type %s generated for real or "
                        "integer-valued tensor %s with type %s must be "
                        "real or integer" % (dtypes.as_dtype(grad_y.dtype).name, y,
                                             dtypes.as_dtype(y.dtype).name))
            elif y.dtype.is_complex:
                if not grad_y.dtype.is_complex:
                    raise TypeError(
                        "Gradient type %s generated for complex-valued "
                        "tensor %s with type %s must be real" % (dtypes.as_dtype(
                            grad_y.dtype).name, y, dtypes.as_dtype(y.dtype).name))
            elif y.dtype == dtypes.variant:
                if grad_y.dtype != dtypes.variant:
                    raise TypeError(
                        "Gradient type %s generated for variant "
                        "tensor %s with type %s must be variant" % (dtypes.as_dtype(
                            grad_y.dtype).name, y, dtypes.as_dtype(y.dtype).name))
            elif y.dtype == dtypes.resource:
                # We assume y is the handle of a ResourceVariable. The gradient of a
                # ResourceVariable should be a numeric value, not another resource.
                if grad_y.dtype == dtypes.resource:
                    raise TypeError("Input gradient %s for resource tensor %s should not "
                                    "be a resource" % (grad_y, y))
            else:
                raise TypeError(
                    "Tensor %s with type %s must be numeric "
                    "to obtain a default gradient" % (y, dtypes.as_dtype(y.dtype).name))
            # Create a grad_y tensor in the name scope of the gradient.
            # Required for TensorArrays to identify which gradient call a
            # grad_y value is coming from.
            if isinstance(grad_y, IndexedSlices):
                new_grad_ys.append(
                    IndexedSlices(
                        indices=(array_ops.identity(
                            grad_y.indices, name="grad_ys_%d_indices" % i)
                                 if isinstance(grad_y.indices, Tensor) else
                                 grad_y.indices),
                        values=(array_ops.identity(
                            grad_y.values, name="grad_ys_%d_values" % i) if isinstance(
                            grad_y.values, Tensor) else grad_y.values),
                        dense_shape=(array_ops.identity(
                            grad_y.dense_shape, name="grad_ys_%d_shape" % i)
                                     if isinstance(grad_y.dense_shape, Tensor) else
                                     grad_y.dense_shape)))
            else:
                new_grad_ys.append(array_ops.identity(grad_y, name="grad_ys_%d" % i))

    return new_grad_ys


def RttIsBackpropagatable(tensor):
    # NOTE(George): We make 'string' legal for backpropagating. 
    if tensor.dtype == dtypes.string:
        return True

    if gradients_util.IsTrainable(tensor):
        return True

    dtype = dtypes.as_dtype(tensor.dtype)
    return dtype.base_dtype == dtypes.bfloat16


def RttIsTrainable(tensor_or_dtype):
  if isinstance(tensor_or_dtype, ops.Tensor):
    dtype = tensor_or_dtype.dtype
  else:
    dtype = tensor_or_dtype
  dtype = dtypes.as_dtype(dtype)
  # NOTE(George): `dtypes.string` is appended at last.
  return dtype.base_dtype in (dtypes.float16, dtypes.float32, dtypes.float64,
                              dtypes.complex64, dtypes.complex128,
                              dtypes.resource, dtypes.variant, dtypes.string)


# override tensorflow _DefaultGradYs with RttDefaultGradYs
gradients_util._DefaultGradYs = RttDefaultGradYs

# override tensorflow _IsBackpropagatable with RttIsBackpropagatable
gradients_util._IsBackpropagatable = RttIsBackpropagatable

# override tensorflow IsTrainable with RttIsTrainable
gradients_util.IsTrainable = RttIsTrainable


