import csv
import numpy as np


def read_dataset(file_name=None):
    if file_name is None:
        print("Error! No file name!")
        return
    res_data = []
    with open(file_name, 'r') as f:
        cr = csv.reader(f)
        for each_r in cr:
            curr_r = [np.array([v], dtype=np.float_)[0] for v in each_r]
            res_data.append(curr_r)
            # print(each_r)
    return res_data


def savecsv(file_name, tf_tensor):
    """
    only for numpy.narray
    """
    np.savetxt(file_name, tf_tensor, fmt="%.10f", delimiter=",")


def loadcsv(file_name):
    """
    only for numpy.narray
    """
    return np.loadtxt(file_name, delimiter=",")
