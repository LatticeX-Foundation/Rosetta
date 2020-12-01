# Copyright 2017 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
"""Python wrappers for reader Datasets."""
from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from tensorflow.python.compat import compat
from tensorflow.python.data.ops import dataset_ops
from tensorflow.python.data.util import convert
from tensorflow.python.data.util import structure
from tensorflow.python.framework import dtypes
from tensorflow.python.framework import ops
from tensorflow.python.framework import tensor_shape
from tensorflow.python.ops import array_ops
from tensorflow.python.ops import gen_dataset_ops
from tensorflow.python.ops import gen_experimental_dataset_ops as ged_ops
from tensorflow.python.util.tf_export import tf_export

from latticex.rosetta.secure.decorator.secure_base_ import _secure_ops

# TODO(b/64974358): Increase default buffer size to 256 MB.
_DEFAULT_READER_BUFFER_SIZE_BYTES = 256 * 1024  # 256 KB


def _secure_create_or_validate_filenames_dataset(filenames):
  """Creates (or validates) a dataset of filenames.

  Args:
    filenames: Either a list or dataset of filenames. If it is a list, it is
      convert to a dataset. If it is a dataset, its type and shape is validated.

  Returns:
    A dataset of filenames.
  """
  if isinstance(filenames, dataset_ops.DatasetV2):
    if dataset_ops.get_legacy_output_types(filenames) != dtypes.string:
      raise TypeError(
          "`filenames` must be a `tf.data.Dataset` of `tf.string` elements.")
    if not dataset_ops.get_legacy_output_shapes(filenames).is_compatible_with(
        tensor_shape.scalar()):
      raise TypeError(
          "`filenames` must be a `tf.data.Dataset` of scalar `tf.string` "
          "elements.")
  else:
    filenames = ops.convert_to_tensor(filenames, dtype=dtypes.string)
    filenames = array_ops.reshape(filenames, [-1], name="flat_filenames")
    filenames = dataset_ops.DatasetV2.from_tensor_slices(filenames)

  return filenames


def _secure_create_dataset_reader(dataset_creator, filenames, num_parallel_reads=None):
  """Creates a dataset that reads the given files using the given reader.

  Args:
    dataset_creator: A function that takes in a single file name and returns a
      dataset.
    filenames: A `tf.data.Dataset` containing one or more filenames.
    num_parallel_reads: The number of parallel reads we should do.

  Returns:
    A `Dataset` that reads data from `filenames`.
  """
  def read_one_file(filename):
    filename = ops.convert_to_tensor(filename, dtypes.string, name="filename")
    return dataset_creator(filename)

  if num_parallel_reads is None:
    return filenames.flat_map(read_one_file)
  else:
    raise ValueError(
        "not support parallel interleave dataset for PrivateTextLineDataset")

def _secure_text_line_dataset(filenames, compression_type, buffer_size, data_owner, name=None):
  r"""Creates a dataset that emits the lines of one or more text files.

  Args:
    filenames: A `Tensor` of type `string`.
      A scalar or a vector containing the name(s) of the file(s) to be
      read.
    compression_type: A `Tensor` of type `string`.
      A scalar containing either (i) the empty string (no
      compression), (ii) "ZLIB", or (iii) "GZIP".
    buffer_size: A `Tensor` of type `int64`.
      A scalar containing the number of bytes to buffer.
    data_owner: The owner of dataset in MPC, eg. 0: p0, 1: p1, 2: p2.
    name: A name for the operation (optional).

  Returns:
    A `Tensor` of type `variant`.
  """
  # Add nodes to the TensorFlow graph.
  return _secure_ops.private_text_line_dataset(filenames=filenames,
                           compression_type=compression_type,
                           buffer_size=buffer_size, data_owner=data_owner, name=name)


class _PrivateTextLineDataset(dataset_ops.DatasetSource):
  """A `Dataset` comprising records from one or more text files."""

  def __init__(self, filenames, compression_type=None, buffer_size=None, data_owner=None):
    """Creates a `_PrivateTextLineDataset`.

    Args:
      filenames: A `tf.string` tensor containing one or more filenames.
      compression_type: (Optional.) A `tf.string` scalar evaluating to one of
        `""` (no compression), `"ZLIB"`, or `"GZIP"`.
      buffer_size: (Optional.) A `tf.int64` scalar denoting the number of bytes
        to buffer. A value of 0 results in the default buffering values chosen
        based on the compression type.
      data_owner: The owner of dataset in MPC, eg. 0: p0, 1: p1, 2: p2.
    """
    self._filenames = filenames
    self._compression_type = convert.optional_param_to_tensor(
        "compression_type",
        compression_type,
        argument_default="",
        argument_dtype=dtypes.string)
    self._buffer_size = convert.optional_param_to_tensor(
        "buffer_size",
        buffer_size,
        argument_default=_DEFAULT_READER_BUFFER_SIZE_BYTES)
    self._data_owner = convert.optional_param_to_tensor(
        "data_owner",
        data_owner,
        argument_default=0)
    variant_tensor = _secure_text_line_dataset(
        self._filenames, self._compression_type, self._buffer_size, self._data_owner)
    super(_PrivateTextLineDataset, self).__init__(variant_tensor)

  @property
  def _element_structure(self):
    return structure.TensorStructure(dtypes.string, [])


# @tf_export("data.TextLineDataset", v1=[])
class PrivateTextLineDatasetV2(dataset_ops.DatasetSource):
  """A `Dataset` comprising lines from one or more text files."""

  def __init__(self, filenames, compression_type=None, buffer_size=None, data_owner=None,
               num_parallel_reads=None):
    """Creates a `PrivateTextLineDataset`.

    Args:
      filenames: A `tf.string` tensor or `tf.data.Dataset` containing one or
        more filenames.
      compression_type: (Optional.) A `tf.string` scalar evaluating to one of
        `""` (no compression), `"ZLIB"`, or `"GZIP"`.
      buffer_size: (Optional.) A `tf.int64` scalar denoting the number of bytes
        to buffer. A value of 0 results in the default buffering values chosen
        based on the compression type.
      data_owner: The owner of dataset in MPC, eg. 0: p0, 1: p1, 2: p2.
      num_parallel_reads: (Optional.) A `tf.int64` scalar representing the
        number of files to read in parallel. If greater than one, the records of
        files read in parallel are outputted in an interleaved order. If your
        input pipeline is I/O bottlenecked, consider setting this parameter to a
        value greater than one to parallelize the I/O. If `None`, files will be
        read sequentially.
    """
    filenames = _secure_create_or_validate_filenames_dataset(filenames)
    self._filenames = filenames
    self._compression_type = compression_type
    self._buffer_size = buffer_size
    self._data_owner = data_owner

    def creator_fn(filename):
      return _PrivateTextLineDataset(filename, compression_type, buffer_size, data_owner)

    self._impl = _secure_create_dataset_reader(creator_fn, filenames)
    variant_tensor = self._impl._variant_tensor  # pylint: disable=protected-access

    super(PrivateTextLineDatasetV2, self).__init__(variant_tensor)

  @property
  def _element_structure(self):
    return structure.TensorStructure(dtypes.string, [])


# @tf_export(v1=["data.PrivateTextLineDataset"])
class PrivateTextLineDatasetV1(dataset_ops.DatasetV1Adapter):
  """A `Dataset` comprising lines from one or more text files."""

  def __init__(self, filenames, compression_type=None, buffer_size=None, data_owner=None, 
               num_parallel_reads=None):
    wrapped = PrivateTextLineDatasetV2(filenames, compression_type, buffer_size, data_owner, 
                                num_parallel_reads)
    super(PrivateTextLineDatasetV1, self).__init__(wrapped)
  __init__.__doc__ = PrivateTextLineDatasetV2.__init__.__doc__

  @property
  def _filenames(self):
    return self._dataset._filenames  # pylint: disable=protected-access

  @_filenames.setter
  def _filenames(self, value):
    self._dataset._filenames = value  # pylint: disable=protected-access



# TODO(b/119044825): Until all `tf.data` unit tests are converted to V2, keep
# these aliases in place.
PrivateTextLineDataset = PrivateTextLineDatasetV1
