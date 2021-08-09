import tensorflow as tf
import tensorflow.contrib as tf_contrib
import latticex.rosetta as rtt
import sys
sys.setrecursionlimit(3000)

#-----------------------------------------------------------------------------
# user code start
#-----------------------------------------------------------------------------
weight_init = tf_contrib.layers.variance_scaling_initializer()
weight_regularizer = tf_contrib.layers.l2_regularizer(0.0001)


##################################################################################
# Layer
##################################################################################
def conv(x, channels, kernel=4, stride=2, padding='SAME', use_bias=True, scope='conv_0'):
    with tf.variable_scope(scope):
        x = tf.layers.conv2d(inputs=x, filters=channels,
                             kernel_size=kernel, kernel_initializer=weight_init,
                             kernel_regularizer=weight_regularizer,
                             strides=stride, use_bias=use_bias, padding=padding, reuse=tf.AUTO_REUSE)

        return x


def fully_conneted(x, units, use_bias=True, scope='fully_0'):
    with tf.variable_scope(scope):
        x = flatten(x)
        x = tf.layers.dense(x, units=units, kernel_initializer=weight_init,
                            kernel_regularizer=weight_regularizer, use_bias=use_bias, name='fc4096')

        return x


##################################################################################
# Sampling
##################################################################################

def flatten(x):
    return tf.layers.flatten(x)


def global_avg_pooling(x):
    gap = tf.reduce_mean(x, axis=[1, 2], keepdims=True)
    return gap


def avg_pooling(x):
    return tf.layers.average_pooling2d(x, pool_size=2, strides=2, padding='SAME')

##################################################################################
# Activation function
##################################################################################


def relu(x):
    return tf.nn.relu(x)


##################################################################################
# Normalization function
##################################################################################

def batch_norm(x, is_training=True, scope='batch_norm'):
    return tf_contrib.layers.batch_norm(x,
                                        decay=0.9, epsilon=1e-04,
                                        center=True, scale=True, updates_collections=None,
                                        is_training=is_training, scope=scope)



##################################################################################
# VGG Block
##################################################################################
def vgg_block_1(x_init, channels, is_training=True, use_bias=True, scope='vgg_block_1'):
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv1_3x3_block_1')
    x = batch_norm(x, is_training, scope='bn1_3x3_block_1')
    x = relu(x)
    x = conv(x_init, channels*4, kernel=3, stride=1, use_bias=use_bias, scope='conv2_3x3_block_1')
    x = batch_norm(x, is_training, scope='bn2_3x3_block_1')
    x = relu(x)
    x = tf.nn.max_pool(x, ksize=(1, 2, 2, 1), strides=(1, 2, 2, 1), padding='VALID')
    return x


def vgg_block_2(x_init, channels, is_training=True, use_bias=True, scope='vgg_block_2'):
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv1_3x3_block_2')
    x = batch_norm(x, is_training, scope='bn1_3x3_block_2')
    x = relu(x)
    x = conv(x_init, channels*2, kernel=3, stride=1, use_bias=use_bias, scope='conv2_3x3_block_2')
    x = batch_norm(x, is_training, scope='bn2_3x3_block_2')
    x = relu(x)
    x = tf.nn.max_pool(x, ksize=(1, 2, 2, 1), strides=(1, 2, 2, 1), padding='VALID')
    return x


def vgg_block_3(x_init, channels, is_training=True, use_bias=True, scope='vgg_block_3'):
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv1_3x3_block_3')
    x = batch_norm(x, is_training, scope='bn1_3x3_block_3')
    x = relu(x)
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv2_3x3_block_3')
    x = batch_norm(x, is_training, scope='bn2_3x3_block_3')
    x = relu(x)
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv3_3x3_block_3')
    x = batch_norm(x, is_training, scope='bn3_3x3_block_3')
    x = relu(x)
    x = conv(x_init, channels*2, kernel=3, stride=1, use_bias=use_bias, scope='conv4_3x3_block_3')
    x = batch_norm(x, is_training, scope='bn4_3x3_block_3')
    x = relu(x)
    x = tf.nn.max_pool(x, ksize=(1, 2, 2, 1), strides=(1, 2, 2, 1), padding='VALID')
    return x


def vgg_block_4(x_init, channels, is_training=True, use_bias=True, scope='vgg_block_4'):
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv1_3x3_block_4')
    x = batch_norm(x, is_training, scope='bn1_3x3_block_4')
    x = relu(x)
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv2_3x3_block_4')
    x = batch_norm(x, is_training, scope='bn2_3x3_block_4')
    x = relu(x)
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv3_3x3_block_4')
    x = batch_norm(x, is_training, scope='bn3_3x3_block_4')
    x = relu(x)
    x = conv(x_init, channels*2, kernel=3, stride=1, use_bias=use_bias, scope='conv4_3x3_block_4')
    x = batch_norm(x, is_training, scope='bn4_3x3_block_4')
    x = relu(x)
    x = tf.nn.max_pool(x, ksize=(1, 2, 2, 1), strides=(1, 2, 2, 1), padding='VALID')
    return x


def vgg_block_5(x_init, channels, is_training=True, use_bias=True, scope='vgg_block_5'):
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv1_3x3_block_5')
    x = batch_norm(x, is_training, scope='bn1_3x3_block_5')
    x = relu(x)
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv2_3x3_block_5')
    x = batch_norm(x, is_training, scope='bn2_3x3_block_5')
    x = relu(x)
    x = conv(x_init, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv3_3x3_block_5')
    x = batch_norm(x, is_training, scope='bn3_3x3_block_5')
    x = relu(x)
    x = conv(x_init, channels*2, kernel=3, stride=1, use_bias=use_bias, scope='conv4_3x3_block_5')
    x = batch_norm(x, is_training, scope='bn4_3x3_block_5')
    x = relu(x)
    x = tf.nn.max_pool(x, ksize=(1, 2, 2, 1), strides=(1, 2, 2, 1), padding='VALID')
    return x


##################################################################################
# Generator resnet 
##################################################################################
def network(x, is_training=True, reuse=False):
        ch = 64

        with tf.variable_scope("network", reuse=reuse):   
            # Block 1
            x = vgg_block_1(x, channels=ch, is_training=is_training, scope="block1")

            # Block 2
            x = vgg_block_2(x, channels=ch*2, is_training=is_training, scope="block2")

            # Block 3
            x = vgg_block_3(x, channels=ch*2, is_training=is_training, scope="block3")

            # Block 4
            x = vgg_block_4(x, channels=ch*2, is_training=is_training, scope="block4")

            # Block 5
            x = vgg_block_5(x, channels=ch*2, is_training=is_training, scope="block5")

            # FC & Relu
            x = fully_conneted(x, units=4096, scope='FC_1')
            x = relu(x)

            # FC & Relu
            x = fully_conneted(x, units=4096, scope='FC_2')
            x = relu(x)

            # FC
            x = fully_conneted(x, units=self.label_dim, scope='logit')

            return x



##################################################################################
# Build Model
##################################################################################
def build_model():
    test_inputs = tf.placeholder(tf.float32, [32, 32, 32, 1], name='test_inputs')
    # test_inputs.set_shape([32, 28, 28, 1])
    train_logits = network(test_inputs, is_training=False, reuse=False)

    #-----------------------------------------------------------------------------
    # for test
    # from latticex.rosetta.secure import StaticReplacePass
    # from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
    # import time

    # start_time = time.time()

    # PassObj = StaticReplacePass()
    # if isinstance(train_logits, rtt_ts.RttTensor):
    #     train_logits = PassObj.run(train_logits._raw)
    # else:
    #     train_logits = PassObj.run(train_logits)

    # spass_use_time = time.time()-start_time
    # print("spass_use_time:{}s".format(spass_use_time))
    #------------------------------------------------------------------------------


try:
    build_model()
    print("Pass")
except Exception as e:
    print(e)
    print("Fail")




Writer = tf.summary.FileWriter("log/vgg", tf.get_default_graph())
Writer.close()
rtt.deactivate()
