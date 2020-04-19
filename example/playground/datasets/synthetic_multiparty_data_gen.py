#import matplotlib.pyplot as plt
from functools import reduce
import csv
import math
import os
import random
import numpy as np
np.set_printoptions(suppress=True)

## common const configuration
FLOAT_POINT = 13 # about 4 significant decimal digits.
BIG_NUM = 2 ** 62
PN = [1, -1]

# the absolute lower and upper bound of each attribute value
L = 0.01
U = 10
FIXED_BIAS = 0.3
#linear model random generation
# num of samples
SAMPLES_NUM = 100
# num of variables X, ie. the Dimension of attribute dataset generated.
VAR_NUM = 6
# the Weights
weight_list = np.random.uniform(0.1, 1, VAR_NUM)
weight_list = [float(random.sample(PN, 1)[0]) * v for v in weight_list]
#print("weights: ",  weight_list)
# hardness to learn
# the nearest and farest distance to the classifying line
DIS_L = 0.1
DIS_U = 10

###### Some Functionalities to convert to Multi-Party 'cipher' data #######
def float_to_mpc_type(fi):
    int_p = int(fi)
    decimal_p = fi - int_p
    res = int(int_p * ( 1 << FLOAT_POINT)) + int(decimal_p * (1 << FLOAT_POINT))
    return int(res)

def mpc_to_float(mi):
    fi = mi / (1 << FLOAT_POINT)
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

###############Generate plaintext Linear separable dataset ####################### 
def data_gen_main():
    global VAR_NUM
    global SAMPLES_NUM
    global FIXED_BIAS
    global L, U, DIS_L, DIS_U, PN, weight_list
    dir_name ="./" + str(VAR_NUM) + str("D/")
    if(not os.path.exists(dir_name)): 
        os.mkdir(dir_name)
    attr_f = dir_name + str("/") + str(VAR_NUM) + "d_attr"
    label_f = dir_name + str("/") + str(VAR_NUM) + "d_label"
    Ndim_plain = []
    Ndim_plain_label = []
    
    if (VAR_NUM <= 0 or SAMPLES_NUM <= 0):
        print("Illegal papramter, please check it!")
    curr_num = 0
    while curr_num < SAMPLES_NUM:
        curr_row = []
        curr_label = -1
        sum_v = 0
        for i in range(0, VAR_NUM - 1, 1):
            curr_v = random.sample(PN, 1) * np.random.uniform(L, U, 1)
            while(abs(curr_v) < 0.001):
                curr_v = random.sample(PN, 1) * np.random.uniform(L, U, 1) 
            curr_row.append(curr_v[0])
            sum_v += curr_v * weight_list[i]
        
        online_v = (FIXED_BIAS - sum_v)/(weight_list[VAR_NUM - 1])
        #print("onlineV:", online_v)
        curr_v = np.random.uniform(DIS_L, DIS_U, 1)
        curr_v = random.sample(PN, 1) * curr_v
        if(curr_v <= 0):
            curr_label = 0
        else:
            curr_label = 1
        curr_v += online_v            
        try_num = 0
        # a little relaxtion the restriction!
        while(abs(curr_v) < 0.0003 or abs(curr_v) > 2 ** 20):
            curr_v = np.random.uniform(DIS_L, DIS_U, 1)
            curr_v = random.sample(PN, 1) * curr_v
            curr_v += online_v
            if(curr_v <= 0):
                curr_label = 0
            else:
                curr_label = 1
            curr_v += online_v 
            if(try_num > 10):
                print("Fail! try again!")
                return
            continue
        curr_row.append(curr_v[0])
        Ndim_plain.append(curr_row)
        Ndim_plain_label.append([curr_label])
        curr_num = curr_num + 1

    share_and_output(Ndim_plain, attr_f)
    share_and_output(Ndim_plain_label, label_f)
    with open(attr_f[:-4] + "model", 'w') as f:
        f.write("Weights:\n")
        w_str = reduce(lambda x, y: str(x) + "\n" + str(y), weight_list)
        #f.write(" " * 8 + str(weight_list) + "\n")
        f.write(w_str + "\n\n")
        f.write("BIAS:\n")
        f.write(str(-FIXED_BIAS))

    #test_read_files(attr_f)
    #test_read_files(label_f)

if __name__ == "__main__":
    data_gen_main()
 
