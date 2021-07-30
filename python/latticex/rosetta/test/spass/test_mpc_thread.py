import threading
import latticex.rosetta as rtt  # difference from tensorflow
import math
import os
import sys
import csv
import tensorflow as tf
import pandas as pd
import numpy as np
import time
import argparse

np.set_printoptions(suppress=True)
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
np.random.seed(0)


# rtt.activate("Helix")
rtt.activate("SecureNN")
rtt.backend_log_to_stdout(True)
rtt.set_backend_loglevel(2)
mpc_player_id = rtt.py_protocol_handler.get_party_id()


X = tf.Variable([[1., 2.], [3., 4.]])
W = tf.Variable(tf.zeros([2, 1], dtype=tf.float64))
b = tf.Variable(tf.zeros([1], dtype=tf.float64))
graph = tf.get_default_graph()


class SessionThread(threading.Thread):
    def __init__(self, tid, name, sess):
        threading.Thread.__init__(self)
        self.tid = tid
        self.name = name
        self.sess = sess

    def run(self):
        with graph.as_default():
            # print("thread:", self.tid, self.name, self.sess)
            # for ii in range(2):
            pred_Y = tf.sigmoid(tf.matmul(X, W) + b)
            reaveal_Y = rtt.SecureReveal(pred_Y)
            res = sess.run(reaveal_Y)

try:
    with tf.Session() as sess:
        sess.run(tf.global_variables_initializer())

        # pred_Y = tf.sigmoid(tf.matmul(X, W) + b)
        # reaveal_Y = rtt.SecureReveal(pred_Y)
        # res = sess.run(reaveal_Y)

        start_time = time.time()
        thread1 = SessionThread(1, "Thread-1", sess)
        thread2 = SessionThread(2, "Thread-2", sess)

        thread1.start()
        thread2.start()

        thread1.join()
        thread2.join()

        training_use_time = time.time()-start_time
        print("training_elapsed(s):{}".format(training_use_time))
        print("Pass")
except Exception:
        print("Fail")

rtt.deactivate()



Writer = tf.summary.FileWriter("log/mpc_thread", tf.get_default_graph())
Writer.close()
