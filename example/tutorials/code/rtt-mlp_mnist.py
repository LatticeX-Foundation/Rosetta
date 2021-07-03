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

# real data
# ######################################## difference from tensorflow
file_x = '../dsets/P' + str(mpc_player_id) + "/mnist_train_x.csv"
file_y = '../dsets/P' + str(mpc_player_id) + "/mnist_train_y.csv"
X_train = rtt.PrivateDataset(data_owner=(0, 1), label_owner=1).load_X(file_x, header=None)
Y_train = rtt.PrivateDataset(data_owner=[1], label_owner=1).load_X(file_y, header=None)
# ######################################## difference from tensorflow

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
        for epoch in range(n_epochs):
            epoch_loss = 0.0
            for i in range(n_batches):
                X_batch = X_train[(i * batch_size):(i + 1) * batch_size]
                Y_batch = Y_train[(i * batch_size):(i + 1) * batch_size]
                feed_dict = {x: X_batch, y: Y_batch}
                tfs.run([optimizer, loss], feed_dict)
        saver.save(tfs, './log/ckpt'+str(mpc_player_id)+'/model')

if __name__ == "__main__":
    # input images
    x = tf.placeholder(dtype=tf.float64, name="x", 
                        shape=[None, num_inputs]) 
    # target output
    y = tf.placeholder(dtype=tf.float64, name="y", 
                        shape=[None, num_outputs])
    num_layers = 0
    num_neurons = []
    learning_rate = 0.01
    n_epochs = 20
    batch_size = 100
    n_batches = int(len(X_train)/batch_size)

    model = mlp(x=x,
                num_inputs=num_inputs,
                num_outputs=num_outputs,
                num_layers=num_layers,
                num_neurons=num_neurons)

    loss = tf.reduce_mean(
        tf.nn.sigmoid_cross_entropy_with_logits(logits=model, labels=y))
    optimizer = tf.train.GradientDescentOptimizer(
        learning_rate=learning_rate).minimize(loss)

    predictions_check = tf.equal(tf.argmax(model, 1), tf.argmax(y, 1))
    accuracy_function = tf.reduce_mean(tf.cast(predictions_check, tf.float64))

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
    print("模型已保存")

    print(rtt.get_perf_stats(True))
    rtt.deactivate()
