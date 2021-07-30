#!/usr/bin/python3
# coding: UTF-8
# do privacy matmul with Latticex-Rosetta

import tensorflow as tf
import latticex.rosetta as rtt
import numpy as np

import time

file_x = "./x.csv"
file_y = "./y.csv"
batch_size = 2
iter_num = 2

rtt.activate("SecureNN")
rtt.set_backend_loglevel(0)

party_id = rtt.get_party_id()

def decode_p0(line):
    fields = tf.string_split([line], ',').values
    fields = rtt.PrivateInput(fields, data_owner=0)
    # tf.print(fields)
    return fields

def decode_p1(line):
    fields = tf.string_split([line], ',').values
    fields = rtt.PrivateInput(fields, data_owner=1)
    # tf.print(fields)
    return fields

dataset_x = rtt.PrivateTextLineDataset(file_x, data_owner=0) # owner is p0
dataset_y = rtt.PrivateTextLineDataset(file_y, data_owner=1) # owner is p1

dataset_x = dataset_x\
    .map(decode_p0)\
    .batch(batch_size)

dataset_y = dataset_y\
    .map(decode_p1)\
    .batch(batch_size)

iter_x = dataset_x.make_initializable_iterator()
iter_y = dataset_y.make_initializable_iterator()

v = tf.Variable(rtt.private_input(0, np.ones([4,1])))

batch_x = iter_x.get_next()
batch_y = iter_y.get_next()

data_vertical = tf.concat([batch_x, batch_y], axis=1)

data_v_reveal = rtt.SecureReveal(data_vertical)

result = rtt.SecureReveal(tf.matmul(data_vertical, v))
result_reveal = rtt.SecureReveal(result)

init = tf.global_variables_initializer()
batch = 0
with tf.compat.v1.Session() as sess:
  sess.run(init)
  sess.run([iter_x.initializer, iter_y.initializer])
  start = time.time()
  try:
    while True:
      print("batch: ", batch, "result: ", sess.run(result_reveal))
      batch += 1
  except tf.errors.OutOfRangeError:
      print("to next_epoch...")
  
  print("cost: ", time.time() - start)

