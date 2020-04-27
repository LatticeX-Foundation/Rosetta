import tensorflow as tf
from tensorflow.python.training import training_ops
from tensorflow.python.eager import context as _context
from tensorflow.python.eager import execute as _execute
#from tensorflow.tools.docs import doc_controls as _doc_controls
#from tensorflow.python.util.tf_export import kwarg_only as _kwarg_only

from latticex.rosetta.mpc import MpcApplyGradientDescent
#for overriding the auto-generated in native TensorFlow
def mpc_apply_gradient_descent(var, alpha, delta, use_locking=False, name=None):
  r"""Update '*var' by subtracting 'alpha' * 'delta' from it.

  Args:
    var: A mutable `Tensor`. Must be one of the following types: `float32`, `float64`, `int32`, `uint8`, `int16`, `int8`, `complex64`, `int64`, `qint8`, `quint8`, `qint32`, `bfloat16`, `uint16`, `complex128`, `half`, `uint32`, `uint64`.
      Should be from a Variable().
    alpha: A `Tensor`. Must have the same type as `var`.
      Scaling factor. Must be a scalar.
    delta: A `Tensor`. Must have the same type as `var`. The change.
    use_locking: An optional `bool`. Defaults to `False`.
      If `True`, the subtraction will be protected by a lock;
      otherwise the behavior is undefined, but may exhibit less contention.
    name: A name for the operation (optional).

  Returns:
    A mutable `Tensor`. Has the same type as `var`.
  """
  #print("DEBUG: static mpc_apply_gradient_descent")
  _ctx = _context._context or _context.context()
  if _ctx is not None and _ctx._thread_local_data.is_eager:
    raise RuntimeError("apply_gradient_descent op does not support eager execution. Arg 'out' is a ref.")
  # Add nodes to the TensorFlow graph.
  if use_locking is None:
    use_locking = False
  use_locking = _execute.make_bool(use_locking, "use_locking")
  ##### MODIFICATION ###############
  #_, _, _op = _op_def_lib._apply_op_helper(
  #      "ApplyGradientDescent", var=var, alpha=alpha, delta=delta,
  #                              use_locking=use_locking, name=name)
  _op = MpcApplyGradientDescent(var=var, alpha=alpha, delta=delta,
                                use_locking=use_locking, name=name).op
  _result = _op.outputs[:]
  _inputs_flat = _op.inputs
  _attrs = ("T", _op.get_attr("T"), "use_locking",
            _op.get_attr("use_locking"))
  _execute.record_gradient(
      "MpcApplyGradientDescent", _inputs_flat, _attrs, _result, name)
  _result, = _result
  return _result

# def MpcApplyGradientDescent(var, alpha, delta, use_locking=False, name=None):
#   return mpc_apply_gradient_descent(var=var, alpha=alpha, delta=delta, use_locking=use_locking, name=name)
# MpcApplyGradientDescent.__doc__ = mpc_apply_gradient_descent.__doc__
# MpcApplyGradientDescent = _doc_controls.do_not_generate_docs(_kwarg_only(MpcApplyGradientDescent))
# #tf_export("raw_ops.ApplyGradientDescent")(MPCApplyGradientDescent)

#NOTE: DO the static replacement
training_ops.apply_gradient_descent = mpc_apply_gradient_descent
#tf.training_ops.apply_gradient_descent = mpc_apply_gradient_descent 
