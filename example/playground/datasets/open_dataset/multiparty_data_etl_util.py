#import matplotlib.pyplot as plt
from functools import reduce
import csv
import math
import os
import random
import numpy as np
np.set_printoptions(suppress=True)

import pandas as pd
from pandas import Series,DataFrame

## common const configuration
FLOAT_POINT = 13 # about 4 significant decimal digits.
BIG_NUM = 2 ** 62

###### Some Functionalities to convert to Multi-Party 'cipher' data #######
def float_to_mpc_type(fi):
    int_p = int(fi)
    decimal_p = fi - int_p
    res = int(int_p * ( 2 << FLOAT_POINT)) + int(decimal_p * (2 << FLOAT_POINT))
    return int(res)

def mpc_to_float(mi):
    fi = mi / (2 << FLOAT_POINT)
    #print(fi)
    return fi

# print(float_to_mpc_type(1.0))
# print(float_to_mpc_type(1.1))
# print(float_to_mpc_type(0.5))
# print(float_to_mpc_type(1.5))
# print(mpc_to_float(float_to_mpc_type(1.0)))
# print(mpc_to_float(float_to_mpc_type(1.1)))
# print(mpc_to_float(float_to_mpc_type(0.5)))
# print(mpc_to_float(float_to_mpc_type(1.5)))

def reconst(shared_a0, shared_a1, is_inner = False):
    """ 
        reconstruct the plain value from two shared value
        @arg: 
            is_inner: indicate whether the inputs are in ring or floats
    """
    if is_inner:
        cipher_all = shared_a0 + shared_a1 
        return mpc_to_float(cipher_all)
    else:
        cipher_all = float_to_mpc_type(shared_a0) + float_to_mpc_type(shared_a1)
        return mpc_to_float(cipher_all)

def share_dataset(plaintext_dataset, is_inner = False):
    """
    @args:
        plaintext_dataset: a list of samples, which is a list of
        common column_num data. eg:
            [[1, 11, 111],
            [2, 22, 222]]
        is_inner: are the values in outputs are big integer or float.
    @return: two shared dataset
    """
    res_a0 = []
    res_a1 = []
    for each_sam in plaintext_dataset:
        this_sample_a = []
        this_sample_b = []
        for each_v in each_sam:
            #cipher_a =  np.random.uniform(-BIG_NUM, BIG_NUM, 1)
            cipher_a =  int(np.random.uniform(-BIG_NUM, BIG_NUM, 1)[0])
            p0_v = cipher_a
            shared_all = float_to_mpc_type(each_v)
            p1_v = shared_all - p0_v
            if(not is_inner):
                p0_v = mpc_to_float(p0_v)
                p1_v = mpc_to_float(p1_v)
            #recs = mpc_to_float(reconst(p0_v, p1_v))
            #print("debug:", recs)
            this_sample_a.append(p0_v)
            this_sample_b.append(p1_v)
        res_a0.append(this_sample_a)
        res_a1.append(this_sample_b)
    return (res_a0, res_a1)

def reconst_dataset(shared_a0, shared_a1):
    if len(shared_a0) != len(shared_a1):
        print("ERROR 1!")
        return
    plain_data = []
    for row_i in range(0, len(shared_a0)):
        if(len(shared_a0[row_i]) != len(shared_a1[row_i])):
            print("ERROR 2!")
            return
        plain_row = []
        for col_j in range(0, len(shared_a0[row_i])):
            plain_row.append(reconst(shared_a0[row_i][col_j],
                                        shared_a1[row_i][col_j]))
        plain_data.append(plain_row)
    return plain_data

def print_ds(ds):
    """ print the content of a dataset pretty"""
    print("{")
    for row in ds:
        print("") 
        for col in row:
            #print(type(col))
            #print(col)
            if isinstance(col, float):
                col = np.array([col], dtype=np.float_)[0]
                #print(f'{float(col):20.4}', end=' ')
                print(f'{col:20}', end=' ')
            if isinstance(col, np.ndarray):
                #print(col.dtype)
                if(int(col) == col):
                   #print(f'{int(col):20}', end=' ')
                   print(f'{int(col):20}', end=' ')
                else:
                    #print(f'{col:20}', end=' ')
                    print(f'{col}', end =" ")
            if isinstance(col, int):
                print(f'{int(col):20}', end=' ')
    print("")
    print("}\n\n")

def test_shared_result(test_ds):
    """ show the procedure of converting plaintext dataset to its random multi-party shared data"""
    print("ori ds: ")
    print_ds(test_ds)
    test_share_0, test_share_1 = share_dataset(test_ds)
    print("P0 share: ")
    print_ds(test_share_0)
    print("P1 share: ")
    print_ds(test_share_1)
    print("reconst: ")
    rec_ds = reconst_dataset(test_share_0, test_share_1)
    print_ds(rec_ds)
# test_ds = [
#             [1.1, 10.01, 1100.101],
#             [0.2, 0.22, 0.222222],
#             [1000, 10000, 10010.1]
# ]
# test_shared_result(test_ds)

############# Output in specific dir ##############
def share_and_output(plain_ds, file_name = None):
    """ Interface!
        convert the plaintext dataset to its two shared datasets, and 
        then output them to 3 files separately 
    """
    if file_name is None:
        file_name = "./gen_data/tmp"
    file_plain = file_name + "_plain.csv"
    with open(file_plain, 'w') as f:
        writer = csv.writer(f)
        for each_row in plain_ds:
            writer.writerow(each_row)
    share_ds0, share_ds1 = share_dataset(plain_ds)
    file_shared = file_name + "_share_0.csv"
    with open(file_shared, 'w') as f:
        writer = csv.writer(f)
        for each_row in share_ds0:
            writer.writerow(each_row)
    file_shared = file_name + "_share_1.csv"
    with open(file_shared, 'w') as f:
        writer = csv.writer(f)
        for each_row in share_ds1:
            writer.writerow(each_row)

def test_read_files(file_name = None):
    """ read the plaintext file and test its shared files are correct"""
    file_plain = file_name + "_plain.csv"
    print("plain:")
    with open(file_plain, 'r') as f:
        cr = csv.reader(f)
        for each_r in cr:
            print(each_r)
    file_share = file_name + "_share_0.csv"
    share_ds_0 = []
    print("share 0:")
    with open(file_share, 'r') as f:
        cr = csv.reader(f)
        for each_r in cr:
            curr_r = [np.array([v], dtype=np.float_)[0] for v in each_r]
            share_ds_0.append(curr_r)
            print(each_r)  
    share_ds_1 = []
    print("share 1:")
    file_share = file_name + "_share_1.csv"
    with open(file_share, 'r') as f:
        cr = csv.reader(f)
        for each_r in cr:
            curr_r = [np.array([v], dtype=np.float_)[0] for v in each_r]
            share_ds_1.append(curr_r)
            print(each_r)
    recs_ds = reconst_dataset(share_ds_0, share_ds_1)
    print("recovery plain:")
    print_ds(recs_ds)

# THE PERCENTAGE OF THE TESTING SAMPLES
TEST_NUM_PERCENT = 0.2
#### convert ######
def wine_dataset_etl(is_regression = False):
    global TEST_NUM_PERCENT
    SRC_DATA = "raw/wine/wine.data"
    src_df = pd.read_csv(SRC_DATA, header=None, sep = ',')
    #print(src_df.head(5))
    label_v = []
    attr_v = []
    #for lv in src_df.loc[:, 0]:
    print(len(src_df))
    # to have random train and test data splitting
    shuffled = np.random.permutation(len(src_df))
    # print("shuffled:", shuffled) 
    for row_i in shuffled:
        # for classification
        lv = src_df.loc[row_i,0]
        if not is_regression:
            if lv == 1:
                lv = 0
            elif lv == 3:
                lv = 1
            else:
                continue
        tmp = []
        tmp.append(lv)
        label_v.append(tmp)
        tmp = list(src_df.loc[row_i,1:])
        attr_v.append(tmp)
    #print(list(src_df.loc[:,0]))
    # print("Label:", label_v)
    # print("Attr:", attr_v)

    DEST_DIR = "cooked/wine"
    if is_regression:
        DEST_DIR = DEST_DIR + "_reg"
    else:
        DEST_DIR = DEST_DIR + "_cls"
    train_dir = DEST_DIR + "_train"
    test_dir = DEST_DIR + "_test"
    if(not os.path.exists(train_dir)): 
        os.mkdir(train_dir)
    if(not os.path.exists(test_dir)): 
        os.mkdir(test_dir)
    
    total_num = len(label_v)
    train_num = int(total_num * (1-TEST_NUM_PERCENT))
    print("total instaces:", total_num)
    attr_f = train_dir + "/attr"
    label_f = train_dir + "/label"
    share_and_output(label_v[0:train_num], label_f)
    share_and_output(attr_v[0:train_num], attr_f)
    attr_f = test_dir + "/attr"
    label_f = test_dir + "/label"
    share_and_output(label_v[train_num:], label_f)
    share_and_output(attr_v[train_num:], attr_f)


def winequality_data_elt(is_regression = False):
    SRC_DATA = "raw/winequality/winequality-red.csv"
    src_df = pd.read_csv(SRC_DATA, sep = ';')
    print(src_df.head(5))
    label_v = []
    attr_v = []
    # to have random train and test data splitting
    shuffled = np.random.permutation(len(src_df))
    for row_i in shuffled:
        # for classification
        #lv = np.array([src_df.iloc[row_i:(row_i+1), -1]], dtype=np.float_)[0]
        lv = np.array(src_df.iloc[row_i:(row_i+1), -1], dtype=np.int16)[0]
        if not is_regression:
            if lv <= 5:
                lv = 0
            elif lv > 5:
                lv = 1
            else:
                continue
        tmp = []
        tmp.append(lv)
        label_v.append(tmp)
        #print("Attr:\n", src_df.iloc[row_i:(row_i+1), :-1])
        tmp =list(np.array(src_df.iloc[row_i:(row_i+1), :-1], dtype=np.float_))[0]
        #tmp = list(src_df.loc[row_i,1:])
        attr_v.append(tmp)

    DEST_DIR = "cooked/winequality"
    if is_regression:
        DEST_DIR = DEST_DIR + "_reg"
    else:
        DEST_DIR = DEST_DIR + "_cls"
    train_dir = DEST_DIR + "_train"
    test_dir = DEST_DIR + "_test"
    if(not os.path.exists(train_dir)): 
        os.mkdir(train_dir)
    if(not os.path.exists(test_dir)): 
        os.mkdir(test_dir)

    total_num = len(label_v)
    train_num = int(total_num * (1-TEST_NUM_PERCENT))
    print("total instaces:", total_num)
    attr_f = train_dir + "/attr"
    label_f = train_dir + "/label"
    share_and_output(label_v[0:train_num], label_f)
    share_and_output(attr_v[0:train_num], attr_f)
    attr_f = test_dir + "/attr"
    label_f = test_dir + "/label"
    share_and_output(label_v[train_num:], label_f)
    share_and_output(attr_v[train_num:], attr_f)

def mnist_dataset_etl(is_train = True, is_regression = False):
    if is_train:
        SRC_DATA = "raw/mnist/mnist_train.csv"
        DEST_DIR = "cooked/cls_mnist_train"
    else:
        SRC_DATA = "raw/mnist/mnist_test.csv"
        DEST_DIR = "cooked/cls_mnist_test"       
    # whether we are using it in regression task.
    # the default is false, which means the destination dataset is
    # just for classification tasks.
    src_csv = SRC_DATA
    attr_f = DEST_DIR + "/attr"
    label_f = DEST_DIR + "/label"
    src_df = pd.read_csv(src_csv, header=None, sep = ',')
    #print(src_df.head(5))
    label_v = []
    attr_v = []
    #for lv in src_df.loc[:, 0]:
    print(len(src_df))
    for row_i in range(len(src_df)):
        # for classification
        lv = src_df.loc[row_i,0]
        if not is_regression:
            if lv == 2:
                lv = 0
                print("2: ", row_i)
            elif lv == 6:
                lv = 1
                print("6: ", row_i)
            else:
                continue
        tmp = []
        tmp.append(lv)
        label_v.append(tmp)
        tmp = list(src_df.loc[row_i,1:])
        attr_v.append(tmp)

    DEST_DIR = ""
    if (is_train and not is_regression):
        DEST_DIR = "cooked/mnist_cls_train"
    elif (not is_train and not is_regression):
        DEST_DIR = "cooked/mnist_cls_test"
    elif (is_train and is_regression):
        DEST_DIR = "cooked/mnist_reg_train"
    elif (not is_train and is_regression):
        DEST_DIR = "cooked/mnist_reg_test"   
        
    if(not os.path.exists(DEST_DIR)): 
        os.mkdir(DEST_DIR)    
    attr_f = DEST_DIR + "/attr"
    label_f = DEST_DIR + "/label" 
    share_and_output(label_v, label_f)
    share_and_output(attr_v, attr_f)


if __name__ == "__main__":

    ## Note: please download the Mnist dataset in raw/mnist first!
    mnist_dataset_etl(is_train=True, is_regression=False)
    mnist_dataset_etl(is_train=False, is_regression=False)
    mnist_dataset_etl(is_train=True, is_regression=True)
    mnist_dataset_etl(is_train=False, is_regression=True)  

    #wine_dataset_etl(is_regression=False)
    #wine_dataset_etl(is_regression=True)
    #winequality_data_elt(is_regression = False)   
    #winequality_data_elt(is_regression = True)

