import tensorflow as tf
import sys
sys.path.append('./../../grads_ops/')
sys.path.append('./../../')
sys.path.append('./..')

import latticex.rosetta as rst

# linear model
X = tf.Variable(1.0, name='x')
W = tf.Variable(2.0, name='weight')
b = tf.Variable(3.0, name='bias')
Loss = tf.multiply(X, W) + b

try:
    train_step = tf.train.FtrlOptimizer(0.01).minimize(Loss)
    print("Pass")
except Exception:
    print("Fail")

