import tensorflow as tf
from numpy import *
import latticex.rosetta as rst


#define pre-var
training_epochs = 20
learning_rate = 0.01

#create model
X = tf.placeholder('float64')
Y = tf.placeholder('float64')

W = tf.Variable(tf.random_normal([1], dtype=tf.float64), name='weight')
b = tf.Variable(tf.zeros([1], dtype=tf.float64), name='bias')
Y_hat = tf.sigmoid(tf.multiply(X, W) + b)
#Loss = tf.nn.sigmoid_cross_entropy_with_logits(logits=Y_hat, labels=Y)
Cross = -Y*tf.log(Y_hat) - (1-Y)*tf.log(1-Y_hat)
Loss = tf.reduce_mean(Cross)

try:
    train_step = tf.train.GradientDescentOptimizer(learning_rate).minimize(Loss)
    print("Pass")
except Exception:
    print("Fail")


#write log
Writer = tf.summary.FileWriter("log/logistic", tf.get_default_graph())
Writer.close()
rst.deactivate()