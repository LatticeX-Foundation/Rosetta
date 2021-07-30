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
# Normalization function
##################################################################################
def batch_norm(x, is_training=True, scope='batch_norm'):
    return tf_contrib.layers.batch_norm(x,
                                        decay=0.9, epsilon=1e-04,
                                        center=True, scale=True, updates_collections=None,
                                        is_training=is_training, scope=scope)


##################################################################################
# Conv2D Layer
##################################################################################
def conv(x, channels, kernel=4, stride=2, padding='SAME', use_bias=True, scope='conv_0'):
    with tf.variable_scope(scope):
        x = tf.layers.conv2d(inputs=x, filters=channels,
                             kernel_size=kernel, kernel_initializer=weight_init,
                             kernel_regularizer=weight_regularizer,
                             strides=stride, use_bias=use_bias, padding=padding)
        return x


##################################################################################
# Dense Layer
##################################################################################
def fully_conneted(x, units, use_bias=True, scope='fully_0'):
    with tf.variable_scope(scope):
        x = tf.layers.flatten(x)
        x = tf.layers.dense(x, units=units, kernel_initializer=weight_init, kernel_regularizer=weight_regularizer, use_bias=use_bias)

        return x

##################################################################################
# Sample Layer
##################################################################################
def avg_pooling(x) :
    return tf.layers.average_pooling2d(x, pool_size=2, strides=2, padding='SAME')

def max_pooling(x) :
    return tf.layers.max_pooling2d(x, pool_size=2, strides=2, padding='SAME')

##################################################################################
# resblock
##################################################################################
def resblock(x_init, channels, is_training=True, use_bias=True, downsample=False, scope='resblock') :
    with tf.variable_scope(scope) :

        x = batch_norm(x_init, is_training, scope='batch_norm_0')
        x = tf.nn.relu(x)

        if downsample :
            x = conv(x, channels, kernel=3, stride=2, use_bias=use_bias, scope='conv_0')
            x_init = conv(x_init, channels, kernel=1, stride=2, use_bias=use_bias, scope='conv_init')

        else :
            x = conv(x, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv_0')

        x = batch_norm(x, is_training, scope='batch_norm_1')
        x = tf.nn.relu(x)
        x = conv(x, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv_1')

        return x + x_init

##################################################################################
# bottle_resblock
##################################################################################
def bottle_resblock(x_init, channels, is_training=True, use_bias=True, downsample=False, scope='bottle_resblock') :
    with tf.variable_scope(scope) :
        x = batch_norm(x_init, is_training, scope='batch_norm_1x1_front')
        shortcut = tf.nn.relu(x)

        x = conv(shortcut, channels, kernel=1, stride=1, use_bias=use_bias, scope='conv_1x1_front')
        x = batch_norm(x, is_training, scope='batch_norm_3x3')
        x = tf.nn.relu(x)

        if downsample :
            x = conv(x, channels, kernel=3, stride=2, use_bias=use_bias, scope='conv_0')
            shortcut = conv(shortcut, channels*4, kernel=1, stride=2, use_bias=use_bias, scope='conv_init')

        else :
            x = conv(x, channels, kernel=3, stride=1, use_bias=use_bias, scope='conv_0')
            shortcut = conv(shortcut, channels * 4, kernel=1, stride=1, use_bias=use_bias, scope='conv_init')

        x = batch_norm(x, is_training, scope='batch_norm_1x1_back')
        x = tf.nn.relu(x)
        x = conv(x, channels*4, kernel=1, stride=1, use_bias=use_bias, scope='conv_1x1_back')

        return x + shortcut

def get_residual_layer(res_n) :
    x = []

    if res_n == 18 :
        x = [2, 2, 2, 2]

    if res_n == 34 :
        x = [3, 4, 6, 3]

    if res_n == 50 :
        x = [3, 4, 6, 3]

    if res_n == 101 :
        x = [3, 4, 23, 3]

    if res_n == 152 :
        x = [3, 8, 36, 3]

    return x


##################################################################################
# Generator resnet 
##################################################################################
def network(x, is_training=True, reuse=False):
    with tf.variable_scope("network", reuse=reuse):

        resnet_n = 50
        if (resnet_n < 50):
            residual_block = resblock
        else:
            residual_block = bottle_resblock

        residual_list = get_residual_layer(resnet_n)

        ch = 32  # paper is 64
        x = conv(x, channels=ch, kernel=3, stride=1, scope='conv')

        for i in range(residual_list[0]):
            x = residual_block(x, channels=ch, is_training=is_training, downsample=False,
                                scope='resblock0_' + str(i))

        ########################################################################################################
        x = residual_block(x, channels=ch * 2, is_training=is_training, downsample=True, scope='resblock1_0')

        for i in range(1, residual_list[1]):
            x = residual_block(x, channels=ch * 2, is_training=is_training, downsample=False,
                                scope='resblock1_' + str(i))

        ########################################################################################################
        x = residual_block(x, channels=ch * 4, is_training=is_training, downsample=True, scope='resblock2_0')

        for i in range(1, residual_list[2]):
            x = residual_block(x, channels=ch * 4, is_training=is_training, downsample=False,
                                scope='resblock2_' + str(i))

        ########################################################################################################
        x = residual_block(x, channels=ch * 8, is_training=is_training, downsample=True, scope='resblock_3_0')

        for i in range(1, residual_list[3]):
            x = residual_block(x, channels=ch * 8, is_training=is_training, downsample=False,
                                scope='resblock_3_' + str(i))

        ########################################################################################################

        x = batch_norm(x, is_training, scope='batch_norm')
        x = tf.nn.relu(x)

        x = avg_pooling(x)
        x = fully_conneted(x, units=10, scope='logit')
        x = tf.argmax(x, -1)

        return x



##################################################################################
# Build Model
##################################################################################
def build_model():
    test_inputs = tf.placeholder(tf.float32, [32, 28, 28, 1], name='test_inputs')
    # test_inputs.set_shape([32, 28, 28, 1])
    train_logits = network(test_inputs, is_training=False, reuse=False)

    #-----------------------------------------------------------------------------
    # for test
    from latticex.rosetta.secure import StaticReplacePass
    from latticex.rosetta.rtt.framework import rtt_tensor as rtt_ts
    import time

    start_time = time.time()

    PassObj = StaticReplacePass()
    if isinstance(train_logits, rtt_ts.RttTensor):
        train_logits = PassObj.run(train_logits._raw)
    else:
        train_logits = PassObj.run(train_logits)

    spass_use_time = time.time()-start_time
    print("spass_use_time:{}s".format(spass_use_time))
    #------------------------------------------------------------------------------


try:
    build_model()
    print("Pass")
except Exception as e:
    print(str(e))
    print("Fail")



Writer = tf.summary.FileWriter("log/resnetX", tf.get_default_graph())
Writer.close()
