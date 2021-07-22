import os
import csv
import tensorflow as tf
import numpy as np
from util import read_dataset
import argparse

np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)


# ################################################## Args
parser = argparse.ArgumentParser(description="Rosetta Linear Regression")
parser.add_argument('--dims', type=int, help="dims", default=5)
parser.add_argument('--epochs', type=int, help="epochs", default=10)
args, unparsed = parser.parse_known_args()
# ##################################################
EPOCHES = args.epochs
DIM_NUM = args.dims

learning_rate = 0.0002
BATCH_SIZE = 5
DIS_STEP = 5


# real data
# ######################################## difference from mpc
prefix = '../playground/datasets/'
file_name_prefix = prefix + str(DIM_NUM) + "D/" + str(DIM_NUM) + "d"
real_X = read_dataset(file_name_prefix + "_attr_plain.csv")
real_Y = read_dataset(file_name_prefix + "_label_plain.csv")
# ######################################## difference from mpc

real_X = np.array(real_X)
real_Y = np.array(real_Y)
print(real_X)
print(real_Y)


model = tf.keras.Sequential()
model.add(tf.keras.layers.Dense(units=1, input_shape=(DIM_NUM,),))
print(model)

# model.compile(loss='mse', optimizer=tf.keras.optimizers.Adam(learning_rate))
model.compile(loss='mse', optimizer=tf.keras.optimizers.SGD(learning_rate))

# train
history = model.fit(real_X, real_Y, epochs=EPOCHES, batch_size=32,
                    verbose=1, steps_per_epoch=2)

lost = model.evaluate(real_X, real_Y, batch_size=32, verbose=1)
print('lost:', lost)

W, b = model.layers[0].get_weights()
print('Weights=', W, '\nbiases=', b)

Y_pred = model.predict(real_X)
print('Y_pred:', Y_pred)
