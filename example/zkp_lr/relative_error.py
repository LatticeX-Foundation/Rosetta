#!/usr/bin/python3
import numpy as np

zk_res = np.loadtxt('./log/preds_zk_mnist.csv')
tf_res = np.loadtxt('./log/preds_tf_mnist.csv')
relative_error = np.abs((np.array(zk_res)-np.array(tf_res))/np.array(zk_res))
#print('relative_error:', relative_error)
print('np.mean(prediction probability relative_error):', np.mean(relative_error))
error_in_bits = -np.mean(np.log2(relative_error))
print('error_in_bits:', error_in_bits)
