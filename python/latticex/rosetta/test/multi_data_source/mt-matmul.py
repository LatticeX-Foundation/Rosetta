#!/usr/bin/env python3
import latticex.rosetta as rtt  # difference from tensorflow
import math
import os
import csv
import tensorflow as tf
import numpy as np
#from util import read_dataset
from test_fw import mt_unit_test_fw
import time

def test(task_id):
  rtt.py_protocol_handler.set_loglevel(0)
  np.set_printoptions(suppress=True)
  
  
  rtt.activate("SecureNN", task_id = task_id)
  print('begin get io wrapper', task_id)
  node_id = rtt.get_current_node_id(task_id = task_id)
  print('end get io wrapper', task_id)
  dg = tf.Graph()
  with dg.as_default():
    # Get private data from Alice (input x), Bob (input y)
    w = tf.Variable(rtt.private_input(0, [[1, 2], [2, 3]], task_id = task_id))
    x = tf.Variable(rtt.private_input(1, [[1, 2], [2, 3]], task_id = task_id))
    y = tf.Variable(rtt.private_input(2, [[1, 2], [2, 3]], task_id = task_id))
    
    # Define matmul operation
    res = tf.matmul(tf.matmul(w, x), y)
    init = tf.global_variables_initializer()
    config = tf.ConfigProto(inter_op_parallelism_threads = 16, intra_op_parallelism_threads = 16) 
    
    with tf.Session(task_id = task_id, config = config) as sess:
        sess.run(init)
        #rW, rb = sess.run([reveal_W, reveal_b])
        #print("init weight:{} \nbias:{}".format(rW, rb))
    
        #Y_pred = sess.run(reveal_Y, feed_dict={X: real_X, Y: real_Y})
        #print("Y_pred:", Y_pred)
        sess.run(res)
    
    print(rtt.get_perf_stats(pretty = True, task_id = task_id))
    rtt.deactivate(task_id = task_id)
for j in range(1):
  task_list = []
  for i in range(1000):
      task_list.append(test)
  mt_unit_test_fw(task_list)
  time.sleep(3)
