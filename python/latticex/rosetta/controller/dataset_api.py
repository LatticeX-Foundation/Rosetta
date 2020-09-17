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

import numpy as np
import pandas as pd
from enum import Enum, unique
from latticex.rosetta.controller.controller_base_ import _rtt
from latticex.rosetta.controller.common_util import rtt_get_logger
import warnings


class DatasetType(Enum):
    """
    SampleAligned - The user or ID is exactly the same, and the features are totally different.
    FeatureAligned - The features are exactly the same, and the users are totally different.

    eg. SampleAligned.
        Party 0 owns data and label: (ID, means the id of the sample, F* means the features)
        ID  F1   F2   F3    LABEL
        1   1.1  1.2  1.3   0
        2   2.1  2.2  2.3   1
        3   3.1  3.2  3.3   1
        Party 1 owns data: (the same sample's id, the different features from Party 0)
        ID  F4   F5   F6   F7 
        1   1.4  1.5  1.6  1.7
        2   2.4  2.5  2.6  2.7
        3   3.4  3.5  3.6  3.7

    eg. FeatureAligned.
        Party 0 owns data and label: (ID, means the id of the sample, F* means the features)
        ID  F1   F2   F3    LABEL
        1   1.1  1.2  1.3   0
        2   2.1  2.2  2.3   1
        3   3.1  3.2  3.3   1
        Party 1 owns data and label: (the same features, the different sample from Party 0)
        ID  F1   F2   F3    LABEL
        4   4.1  4.2  4.3   0
        5   5.1  5.2  5.3   0
        6   6.1  6.2  6.3   1
    """
    SampleAligned = 1
    FeatureAligned = 2


class PrivateDataset(object):
    """
    Args:
        data_owner : tuple
            Indicating which participants have the private data.
            Note: at least one party has data.
        label_owner : int
            Indicating which party has the label(target) value
            default is -1, means no any party has label data.
            if dataset_type is DatasetType.SampleAligned, only one party has the label.
            elif dataset_type is DatasetType.FeatureAligned, all parties in data_owner has the label, ignore this argument.
            elif label_owner is -1, means no any party has label data.
            else, means no any party has label data.
        dataset_type : DatasetType
            default is SampleAligned.
            @see DatasetType

        Note: all above arguments must be the same for all parties.

    Return:

    Usages:
        if dataset_type == DatasetType.SampleAligned. (default dataset_type is SampleAligned)
        assuming P0, P1 owns private data, P1 owns label, then all the parties do respectively:
        >>> XX, YY = get private data from its own data source, set None if no private data
        >>> X,y = PrivateDataset(data_owner=(0,1), label_owner=1).load_data(XX, YY)

        if dataset_type == DatasetType.FeatureAligned, label_owner is useless
        assuming P0,P1 owns private data and label, then all the parties do respectively:
        >>> XX, YY = get private data from its own data source, set None if no private data
        >>> X,y = PrivateDataset(data_owner=(0,1),
                dataset_type=DatasetType.FeatureAligned).load_data(XX, YY)
    """

    def __init__(self, data_owner: tuple or list, label_owner: int = -1,
                 dataset_type: DatasetType = DatasetType.SampleAligned):
        self.data_owner_ = list(data_owner)
        self.label_owner_ = label_owner
        self.dataset_type_ = dataset_type
        if self.dataset_type_ == DatasetType.FeatureAligned:
            self.label_owner_ = 0  # reset to a valid value

        self.dataset_ = _rtt.dataset.DataSet(
            self.data_owner_, self.label_owner_, self.dataset_type_.value)

    def __get_numpy(self, file, *args, **kwargs):
        if not isinstance(file, str):
            return file

        arr = None
        try:
            df = pd.read_csv(file, *args, **kwargs)
            arr = df.to_numpy()
        except Exception:
            pass
            # print(e)
        return arr

    def load_X(self, X: str or np.ndarray or None, *args, **kwargs):
        """
        Args:
            X : str or np.ndarray
                data file path (.csv) or numpy array (2d) or None (if no private data)
                X.shape = (n_samples, n_features)
        """
        inp_X = self.__get_numpy(X, *args, **kwargs)
        res_X = self.dataset_.private_input_x(inp_X)
        # return res_X if res_X is not None else None
        return res_X

    def load_y(self, y: str or np.ndarray or None, *args, **kwargs):
        """
        Args:
            y : str or np.ndarray
                label file path (.csv) or numpy array (2d) or None (if no private data)
                y.shape = (n_samples, 1)
        """
        inp_y = self.__get_numpy(y, *args, **kwargs)
        res_y = self.dataset_.private_input_y(inp_y)
        return res_y

    def load_data(self, X: str or np.ndarray or None, y: str or np.ndarray or None, *args, **kwargs):
        res_X = self.load_X(X, *args, **kwargs)
        res_y = self.load_y(y, *args, **kwargs)
        return res_X, res_y


# This class will be removed
class SecureDataSet(object):
    """
    Note: only supports 2d-numpy.ndarray at present.

    Args:
        p2_owns_data : bool
            flag indicating whether P2 has its own private data.
            Note: P0 and P1 must have its own private data.
        label_owner : int
            indicating who owns the label value (0,1,2)
        dataset_type : DatasetType
    """

    def __init__(self, p2_owns_data: bool = False,
                 label_owner: int = 0,
                 dataset_type: DatasetType = DatasetType.SampleAligned):

        warnings.warn(
            "This class will be removed in the version 0.3.0", DeprecationWarning)

        self.p2_owns_data_ = p2_owns_data
        self.label_owner_ = label_owner
        self.data_owner_ = (0, 1)
        if self.p2_owns_data_:
            self.data_owner_ = (0, 1, 2)

        self.dataset_ = PrivateDataset(
            self.data_owner_, self.label_owner_, dataset_type)

    def load_X(self, file: str, *args, **kwargs):
        """ Load and 'secret-shared' private ATTRIBUTE values from local dataset files.

        Since only SampleAligned is supported now, different party can have different number of attributes 
        while having the same number of samples.

        For example, if dataset of P0 is N * d0, dataset of P1 is N * d1, and P2 has no data at all.
        Then the resulting 'ciphertext' local dataset returned for each party is of the shape N * (d0+d1).

        Return:
            The shared 'ciphertext' local dataset. 
            [of string type with format specific to your current activated protocol].
        """
        return self.dataset_.load_X(file, *args, **kwargs)

    def load_Y(self, file: str, *args, **kwargs):
        """ Load and 'secret-shared' private LABEL values from local dataset files.

        Only the input file of 'label_owner'-party will be processed.
        Return:
            The shared 'ciphertext' local label dataset.
            [of string type with format specific to your current activated protocol].
        """
        return self.dataset_.load_y(file, *args, **kwargs)

    def load_XY(self, fileX: str, fileY: str = '', *args, **kwargs):
        return self.dataset_.load_data(fileX, fileY, *args, **kwargs)

    def private_input_x(self, inp: np.ndarray):
        return self.dataset_.load_X(inp)

    def private_input_y(self, inp: np.ndarray):
        return self.dataset_.load_y(inp)


