import os
import tensorflow as tf
import latticex.rosetta as rtt
import csv
import numpy as np

rtt.set_backend_loglevel(1)
np.set_printoptions(suppress=True)
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '2'
np.random.seed(0)

rtt.activate("SecureNN")
mpc_player_id = rtt.py_protocol_handler.get_party_id()
BATCH_SIZE = 100
ROW_NUM = 5500

# real data
# ######################################## difference from tensorflow
file_x = '../dsets/P' + str(mpc_player_id) + "/mnist_train_x.csv"
file_y = '../dsets/P' + str(mpc_player_id) + "/mnist_train_y.csv"
X_train_0 = rtt.PrivateTextLineDataset(file_x, data_owner=0)
X_train_1 = rtt.PrivateTextLineDataset(file_x, data_owner=1)
Y_train = rtt.PrivateTextLineDataset(file_y, data_owner=1)
# ######################################## difference from tensorflow

cache_dir = "./temp{}".format(mpc_player_id)
if not os.path.exists(cache_dir):
    os.makedirs(cache_dir, exist_ok=True)
else:
    # fix TF1.14 cache file bug
    import shutil
    shutil.rmtree(cache_dir)
    os.makedirs(cache_dir, exist_ok=True)

# dataset decode
def decode_p0(line):
    fields = tf.string_split([line], ',').values
    fields = rtt.PrivateInput(fields, data_owner=0)
    return fields
def decode_p1(line):
    fields = tf.string_split([line], ',').values
    fields = rtt.PrivateInput(fields, data_owner=1)
    return fields

# dataset pipeline
X_train_0 = X_train_0.map(decode_p0).cache(f"{cache_dir}/cache_p0_x0").batch(BATCH_SIZE).repeat()
X_train_1 = X_train_1.map(decode_p1).cache(f"{cache_dir}/cache_p1_x1").batch(BATCH_SIZE).repeat()
Y_train = Y_train.map(decode_p1).cache(f"{cache_dir}/cache_p1_y").batch(BATCH_SIZE).repeat()

# make iterator
iter_x0 = X_train_0.make_initializable_iterator()
X0 = iter_x0.get_next()

iter_x1 = X_train_1.make_initializable_iterator()
X1 = iter_x1.get_next()

iter_y = Y_train.make_initializable_iterator()
Y = iter_y.get_next()
# Join input X of P0 and P1, features splitted dataset
X = tf.concat([X0, X1], axis=1)

num_outputs = 10 
num_inputs = 784 
w=[]
b=[]

def mlp(x, num_inputs, num_outputs, num_layers, num_neurons):
    w = []
    b = []
    for i in range(num_layers):
        # weights
        w.append(tf.Variable(tf.random_normal(
            [num_inputs if i == 0 else num_neurons[i - 1],
             num_neurons[i]], seed = 1, dtype=tf.float64),
            name="w_{0:04d}".format(i), dtype=tf.float64
        ))
        # biases
        b.append(tf.Variable(tf.random_normal(
            [num_neurons[i]], seed = 1, dtype=tf.float64),
            name="b_{0:04d}".format(i), dtype=tf.float64
        ))
    w.append(tf.Variable(tf.random_normal(
        [num_neurons[num_layers - 1] if num_layers > 0 else num_inputs,
         num_outputs], seed = 1, dtype=tf.float64), name="w_out", dtype=tf.float64))
    b.append(tf.Variable(tf.random_normal([num_outputs], seed = 1, dtype=tf.float64), name="b_out", dtype=tf.float64))

    # x is input layer
    layer = x
    # add hidden layers
    for i in range(num_layers):
        layer = tf.nn.relu(tf.matmul(layer, w[i]) + b[i])
    # add output layer
    layer = tf.matmul(layer, w[num_layers]) + b[num_layers]

    return layer


def tensorflow_classification(n_epochs, n_batches,
                              batch_size,
                              model, optimizer, loss
                              ):
    with tf.Session() as tfs:
        tfs.run(tf.global_variables_initializer())
        tfs.run([iter_x0.initializer, iter_x1.initializer, iter_y.initializer])
        for epoch in range(n_epochs):
            epoch_loss = 0.0
            for i in range(n_batches):
                tfs.run([optimizer, loss])
        saver.save(tfs, './log/ckpt'+str(mpc_player_id)+'/model')

if __name__ == "__main__":
    num_layers = 0
    num_neurons = []
    learning_rate = 0.01
    n_epochs = 20
    batch_size = 100
    n_batches = int(ROW_NUM/batch_size)

    model = mlp(x=X,
                num_inputs=num_inputs,
                num_outputs=num_outputs,
                num_layers=num_layers,
                num_neurons=num_neurons)

    loss = tf.reduce_mean(
        tf.nn.sigmoid_cross_entropy_with_logits(logits=model, labels=Y))
    optimizer = tf.train.GradientDescentOptimizer(
        learning_rate=learning_rate).minimize(loss)

    # save
    saver = tf.train.Saver(var_list=None, max_to_keep=5, name='v2')
    os.makedirs("./log/ckpt"+str(mpc_player_id), exist_ok=True)

    tensorflow_classification(n_epochs=n_epochs, 
    n_batches=n_batches, 
    batch_size=batch_size, 
    model = model, 
    optimizer = optimizer, 
    loss = loss
    )
    print("model saved!")

    print(rtt.get_perf_stats(True))
    rtt.deactivate()
