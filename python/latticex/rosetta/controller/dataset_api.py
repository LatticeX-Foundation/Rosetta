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
import latticex._rosetta as _rtt
import numpy as np
import pandas as pd
from enum import Enum, unique
from latticex.rosetta.controller.common_util import rtt_get_logger

class SecureDatasetType(Enum):
    """
    COMMON_N_V_SPLIT - The user or ID is exactly the same,
        and the features are totally different.
    COMMON_F_H_SPLIT - The features are exactly the same,
        and the users are totally different.
    N - Samples
    F - Features
    H - Horizontal
    V - Vertical
    """
    COMMON_N_V_SPLIT = 1
    COMMON_F_H_SPLIT = 2


class SecureDataSet(object):
    """
    Note: only supports 2d-nump.ndarry at present.

    Args:
        p2_owns_data : bool
            flag indicating whether P2 has its own private data.
            Note: P0 and P1 must have its own private data.
        label_owner : int
            indicating who owns the label value (0,1,2)
        dataset_type : SecureDatasetType
    """

    def __init__(self, p2_owns_data: bool = False,
                 label_owner: int = 0,
                 dataset_type: SecureDatasetType = SecureDatasetType.COMMON_N_V_SPLIT):
        self.p2_owns_data_ = p2_owns_data
        self.label_owner_ = label_owner
        self.dataset = _rtt.dataset.DataSet(
            p2_owns_data, label_owner, dataset_type.value)

        if dataset_type != SecureDatasetType.COMMON_N_V_SPLIT:
            raise Exception("only supports COMMON_N_V_SPLIT")

    def __get_numpy(self, file: str, is_x: bool, *args, **kwargs):
        arr = None
        try:
            df = pd.read_csv(file, *args, **kwargs)
            arr = df.to_numpy()
        except FileNotFoundError as e:
            pass
            rtt_get_logger().error(str(e))
        except Exception as e:
            pass
            # print(e)
        return arr

    def load_X(self, file: str, *args, **kwargs):
        """ Load and 'secret-shared' private ATTRIBUTE values from local dataset files.
        
        Since only COMMON_N_V_SPLIT is supported now, different party can have different number of attributes 
        while having the same number of samples.

        For example, if dataset of P0 is N * d0, dataset of P1 is N * d1, and P2 has no data at all.
        Then the resulting 'ciphertext' local dataset returned for each party is of the shape N * (d0+d1).

        Return:
            The shared 'ciphertext' local dataset. 
            [of string type with format specific to your current activated protocol].
        """
        inp = self.__get_numpy(file, is_x=True, *args, **kwargs)
        return self.private_input_x(inp)

    def load_Y(self, file: str, *args, **kwargs):
        """ Load and 'secret-shared' private LABEL values from local dataset files.
        
        Only the input file of 'label_owner'-party will be processed.
        Return:
            The shared 'ciphertext' local label dataset.
            [of string type with format specific to your current activated protocol].
        """
        inp = self.__get_numpy(file, is_x=False, *args, **kwargs)
        return self.private_input_y(inp)

    def load_XY(self, fileX: str, fileY: str = '', *args, **kwargs):
        x = self.load_X(fileX, *args, **kwargs)
        y = self.load_Y(fileY, *args, **kwargs)
        return x, y

    def private_input_x(self, inp: np.ndarray):
        return self.dataset.private_input_x(inp)

    def private_input_y(self, inp: np.ndarray):
        return self.dataset.private_input_y(inp)


dataset = SecureDataSet()
