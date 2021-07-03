from tensorflow.examples.tutorials.mnist import input_data
import os
import tensorflow as tf
mnist_home = os.path.join("/tmp/data/", 'mnist')
mnist = input_data.read_data_sets(mnist_home, one_hot=True)
#将数据分割为训练数据与测试数据
X_train = mnist.train.images
X_test = mnist.test.images
Y_train = mnist.train.labels
Y_test = mnist.test.labels
#构造数据迭代器
train_dataset = tf.data.Dataset.from_tensor_slices((X_train, Y_train))
train_dataset = train_dataset.batch(100).repeat()
test_dataset = tf.data.Dataset.from_tensor_slices((X_test, Y_test))
test_dataset = test_dataset.batch(100).repeat()
train_iterator = train_dataset.make_one_shot_iterator()
train_next_iterator = train_iterator.get_next()
test_iterator = test_dataset.make_one_shot_iterator()
test_next_iterator = test_iterator.get_next()

num_outputs = 10 
num_inputs = 784
w=[]
b=[]

def mlp(x, num_inputs, num_outputs, num_layers, num_neurons):
    w = []
    b = []
    for i in range(num_layers):
        # 权重
        w.append(tf.Variable(tf.random_normal(
            [num_inputs if i == 0 else num_neurons[i - 1],
             num_neurons[i]], seed = 1, dtype=tf.float64),
            name="w_{0:04d}".format(i), dtype=tf.float64
        ))
        # 偏差值
        b.append(tf.Variable(tf.random_normal(
            [num_neurons[i]], seed = 1, dtype=tf.float64),
            name="b_{0:04d}".format(i), dtype=tf.float64
        ))
    w.append(tf.Variable(tf.random_normal(
        [num_neurons[num_layers - 1] if num_layers > 0 else num_inputs,
         num_outputs], seed = 1, dtype=tf.float64), name="w_out", dtype=tf.float64))
    b.append(tf.Variable(tf.random_normal([num_outputs], seed = 1, dtype=tf.float64), name="b_out", dtype=tf.float64))

    # x是输入层
    layer = x
    # 添加隐藏层
    for i in range(num_layers):
        layer = tf.nn.relu(tf.matmul(layer, w[i]) + b[i])
    # 添加输出层
    layer = tf.matmul(layer, w[num_layers]) + b[num_layers]

    return layer

def mnist_batch_func(batch_size=100):
    X_batch, Y_batch = mnist.train.next_batch(batch_size)
    return [X_batch, Y_batch]
  
def tensorflow_classification(n_epochs, n_batches,
                              batch_size,
                              model, optimizer, loss, accuracy_function,
                              X_test, Y_test):
    with tf.Session() as tfs:
        tfs.run(tf.global_variables_initializer())
        for epoch in range(n_epochs):
            epoch_loss = 0.0
            for batch in range(n_batches):
                X_batch, Y_batch = tfs.run(train_next_iterator)
                feed_dict = {x: X_batch, y: Y_batch}
                _, batch_loss = tfs.run([optimizer, loss], feed_dict)
                epoch_loss += batch_loss
        
            average_loss = epoch_loss / n_batches
            print("epoch: {0:04d} loss = {1:0.6f}".format(
                epoch, average_loss))
        feed_dict = {x: X_test, y: Y_test}
        accuracy_score = tfs.run(accuracy_function, feed_dict=feed_dict)
        print("accuracy={0:.8f}".format(accuracy_score))
        
# 构造输入
x = tf.placeholder(dtype=tf.float64, name="x", 
                    shape=[None, num_inputs])
# 构造输出
y = tf.placeholder(dtype=tf.float64, name="y", 
                    shape=[None, 10])
#目前设置隐藏层为0，可自己修改
num_layers = 2
num_neurons = [128, 256]
learning_rate = 0.01
n_epochs = 30
batch_size = 100
n_batches = int(mnist.train.num_examples/batch_size)

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
accuracy_function = tf.reduce_mean(tf.cast(predictions_check, dtype=tf.float64))
#训练
tensorflow_classification(n_epochs=n_epochs, 
   n_batches=n_batches, 
   batch_size=batch_size, 
   model = model, 
   optimizer = optimizer, 
   loss = loss, 
   accuracy_function = accuracy_function, 
   X_test = X_test, 
   Y_test = Y_test
   )
