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

from tensorflow.python.training import saver
from tensorflow.core.protobuf import meta_graph_pb2
from tensorflow.core.protobuf import saver_pb2
from tensorflow.python.ops import io_ops
from tensorflow.python.framework import ops
from tensorflow.python.eager import context
from tensorflow.python.ops import variables
import time

import tensorflow as tf

#BaseSaverBuilder, BulkSaverBuilder, Saver
from latticex.rosetta.mpc import MpcSaveV2

class MpcBaseSaverBuilder(saver.BaseSaverBuilder):
    # override the base class to replace the save_v2 to MpcSaveV2
    def save_op(self, filename_tensor, saveables):
        #print("DEBUG: save_op")
        """Create an Op to save 'saveables'.

        This is intended to be overridden by subclasses that want to generate
        different Ops.

        Args:
        filename_tensor: String Tensor.
        saveables: A list of BaseSaverBuilder.SaveableObject objects.

        Returns:
        An Operation that save the variables.

        Raises:
        RuntimeError: (implementation detail) if "self._write_version" is an
            unexpected value.
        """
        # pylint: disable=protected-access
        tensor_names = []
        tensors = []
        tensor_slices = []
        for saveable in saveables:
            for spec in saveable.specs:
                tensor_names.append(spec.name)
                tensors.append(spec.tensor)
                tensor_slices.append(spec.slice_spec)
        if self._write_version == saver_pb2.SaverDef.V1:
            return io_ops._save(
                filename=filename_tensor,
                tensor_names=tensor_names,
                tensors=tensors,
                tensor_slices=tensor_slices)
        elif self._write_version == saver_pb2.SaverDef.V2:
            # "filename_tensor" is interpreted *NOT AS A FILENAME*, but as a prefix
            # of a V2 checkpoint: e.g. "/fs/train/ckpt-<step>/tmp/worker<i>-<step>".
            return MpcSaveV2(filename_tensor, tensor_names, tensor_slices,
                                tensors)
        else:
            raise RuntimeError("Unexpected write_version: " + self._write_version)

# we have to also redefine the 'MPC'-version for BulkSaverBuilder.
#   since this inherits from BaseSaverBuilder directly.
class MpcBulkSaverBuilder(MpcBaseSaverBuilder):
    """SaverBuilder with support for bulk restoring multiple saveables."""
    def bulk_restore(self, filename_tensor, saveables, preferred_shard,
                   restore_sequentially):
        # Ignored: bulk restore is internally sequential.
        del restore_sequentially
        restore_specs = []
        for saveable in saveables:
            for spec in saveable.specs:
                restore_specs.append((spec.name, spec.slice_spec, spec.dtype))

        names, slices, dtypes = zip(*restore_specs)
        # Load all tensors onto CPU 0 for compatibility with existing code.
        with ops.device("cpu:0"):
            return io_ops.restore_v2(filename_tensor, names, slices, dtypes)


#class MpcSaver(saver.Saver):
    # def _build(self, checkpoint_path, build_save, build_restore):
    #     if self._builder is None:
    #         self._builder = MpcBulkSaverBuilder(self._write_version)
    #     super(MPCSaver, self)._build(self, checkpoint_path, build_save, build_restore)


class MpcSaver(saver.Saver):
    #def __init__(self, *args, **kwargs):
    #      super(MPCRefVariable, self).__init__(*args, **kwargs)
    def _build(self, checkpoint_path, build_save, build_restore):
        """Builds saver_def."""
        if not context.executing_eagerly():
            if self._is_built:
                return
            self._is_built = True

        if not self.saver_def or context.executing_eagerly():
            if self._builder is None:
                ###Attention: this is our target!!
                self._builder = MpcBulkSaverBuilder(self._write_version)

            if self._var_list is None:
                # pylint: disable=protected-access
                self._var_list = variables._all_saveable_objects()
            if not self._var_list:
                if self._allow_empty:
                    self._is_empty = True
                    return
                else:
                    raise ValueError("No variables to save")
            self._is_empty = False

            self.saver_def = self._builder._build_internal(  # pylint: disable=protected-access
                    self._var_list,
                    reshape=self._reshape,
                    sharded=self._sharded,
                    max_to_keep=self._max_to_keep,
                    keep_checkpoint_every_n_hours=self._keep_checkpoint_every_n_hours,
                    name=self._name,
                    restore_sequentially=self._restore_sequentially,
                    filename=checkpoint_path,
                    build_save=build_save,
                    build_restore=build_restore)
        elif self.saver_def and self._name:
            # Since self._name is used as a name_scope by builder(), we are
            # overloading the use of this field to represent the "import_scope" as
            # well.
            self.saver_def.filename_tensor_name = ops.prepend_name_scope(
                    self.saver_def.filename_tensor_name, self._name)
            self.saver_def.save_tensor_name = ops.prepend_name_scope(
                    self.saver_def.save_tensor_name, self._name)
            self.saver_def.restore_op_name = ops.prepend_name_scope(
                    self.saver_def.restore_op_name, self._name)

        self._check_saver_def()
        if not context.executing_eagerly():
            # Updates next checkpoint time.
            # Set in __init__ when executing eagerly.
            self._next_checkpoint_time = (
                time.time() + self.saver_def.keep_checkpoint_every_n_hours * 3600)

tf.train.Saver = MpcSaver

