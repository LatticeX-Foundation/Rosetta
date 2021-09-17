import tensorflow as tf
import tensorflow.contrib as tf_contrib


# Xavier : tf_contrib.layers.xavier_initializer()
# He : tf_contrib.layers.variance_scaling_initializer()
# Normal : tf.random_normal_initializer(mean=0.0, stddev=0.02)
# l2_decay : tf_contrib.layers.l2_regularizer(0.0001)

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
                            kernel_regularizer=weight_regularizer, use_bias=use_bias, name='fc1000')

        return x


def resblock_origial(x_init, channels, is_training=True, use_bias=True, downsample=False, scope='resblock'):
    with tf.variable_scope(scope):

        x = batch_norm(x_init, is_training, scope='batch_norm_0')
        x = relu(x)

        if downsample:
            x = conv(x, channels, kernel=3, stride=2,
                     use_bias=use_bias, scope='conv_0')
            x_init = conv(x_init, channels, kernel=1, stride=2,
                          use_bias=use_bias, scope='conv_init')

        else:
            x = conv(x, channels, kernel=3, stride=1,
                     use_bias=use_bias, scope='conv_0')

        x = batch_norm(x, is_training, scope='batch_norm_1')
        x = relu(x)
        x = conv(x, channels, kernel=3, stride=1,
                 use_bias=use_bias, scope='conv_1')

        return x + x_init


def resblock(x_init, channels, is_training=True, use_bias=True, downsample=False, scope='resblock', stridess=2):
    with tf.variable_scope(scope):

        x = batch_norm(x_init, is_training, scope='batch_norm_0')
        x = relu(x)

        if downsample:
            x = conv(x, channels, kernel=3, stride=2,
                     use_bias=use_bias, scope='conv_0')

        else:
            x = conv(x, channels, kernel=3, stride=1,
                     use_bias=use_bias, scope='conv_0')

        x = batch_norm(x, is_training, scope='batch_norm_1')
        x = relu(x)
        x = conv(x, channels, kernel=3, stride=1,
                 use_bias=use_bias, scope='conv_1')

        if downsample:
            x_init = conv(x_init, channels, kernel=1, stride=2,
                          use_bias=use_bias, scope='conv_init')

        return x + x_init


def bottle_resblock_original(x_init, channels, is_training=True, use_bias=True, downsample=False, scope='bottle_resblock'):
    with tf.variable_scope(scope):
        x = batch_norm(x_init, is_training, scope='batch_norm_1x1_front')
        shortcut = relu(x)

        x = conv(shortcut, channels, kernel=1, stride=1,
                 use_bias=use_bias, scope='conv_1x1_front')
        x = batch_norm(x, is_training, scope='batch_norm_3x3')
        x = relu(x)

        if downsample:
            x = conv(x, channels, kernel=3, stride=2,
                     use_bias=use_bias, scope='conv_0')
            shortcut = conv(shortcut, channels*4, kernel=1,
                            stride=2, use_bias=use_bias, scope='conv_init')

        else:
            x = conv(x, channels, kernel=3, stride=1,
                     use_bias=use_bias, scope='conv_0')
            shortcut = conv(shortcut, channels * 4, kernel=1,
                            stride=1, use_bias=use_bias, scope='conv_init')

        x = batch_norm(x, is_training, scope='batch_norm_1x1_back')
        x = relu(x)
        x = conv(x, channels*4, kernel=1, stride=1,
                 use_bias=use_bias, scope='conv_1x1_back')

        return x + shortcut


def bottle_resblock(x_init, channels, is_training=True, use_bias=True, downsample=False, scope='bottle_resblock'):
    with tf.variable_scope(scope):
        x = batch_norm(x_init, is_training, scope='batch_norm_1x1_front')
        shortcut = relu(x)

        x = conv(shortcut, channels, kernel=1, stride=1,
                 use_bias=use_bias, scope='conv_1x1_front')
        x = batch_norm(x, is_training, scope='batch_norm_3x3')
        x = relu(x)

        if downsample:
            x = conv(x, channels, kernel=3, stride=2,
                     use_bias=use_bias, scope='conv_0')
            shortcut = conv(shortcut, channels*4, kernel=1,
                            stride=2, use_bias=use_bias, scope='conv_init')

        else:
            x = conv(x, channels, kernel=3, stride=1,
                     use_bias=use_bias, scope='conv_0')
            shortcut = conv(shortcut, channels * 4, kernel=1,
                            stride=1, use_bias=use_bias, scope='conv_init')

        shortcut = batch_norm(shortcut, is_training, scope='batch_norm_3x3_2')

        x = batch_norm(x, is_training, scope='batch_norm_1x1_back')
        x = relu(x)
        x = conv(x, channels*4, kernel=1, stride=1,
                 use_bias=use_bias, scope='conv_1x1_back')

        return x + shortcut


def bottle_resblock2(x, channels, is_training=True, use_bias=True, downsample=False, scope='bottle_resblock', stridess=1):
    with tf.variable_scope(scope):
        # for shortcut
        orig_x = x

        #
        #
        # Conv 1x1, strides=2, out_channels
        x = conv(x, channels, kernel=1, stride=stridess,
                 use_bias=use_bias, scope='conv_1x1_1')
        x = batch_norm(x, is_training, scope='batch_norm_3x3_1')
        x = relu(x)

        #
        #
        # Conv 3x3, strides=1, out_channels
        x = conv(x, channels, kernel=3, stride=1,
                 use_bias=use_bias, scope='conv_3x3_2')
        x = batch_norm(x, is_training, scope='batch_norm_3x3_2')
        x = relu(x)

        #
        #
        # Conv 1x1, strides=1, out_channels*4
        x = conv(x, channels*4, kernel=1, stride=1,
                 use_bias=use_bias, scope='conv_1x1_3')
        x = batch_norm(x, is_training, scope='batch_norm_3x3_3')

        if downsample:
            orig_x = conv(orig_x, channels*4, kernel=1, stride=stridess,
                          use_bias=use_bias, scope='conv_initx')
            orig_x = batch_norm(orig_x, is_training, scope='batch_norm_3x3_3m')

        #
        #
        # Add
        z = x + orig_x
        z = relu(z)
        return z


def get_residual_layer(res_n):
    x = []

    if res_n == 18:
        x = [2, 2, 2, 2]

    if res_n == 34:
        x = [3, 4, 6, 3]

    if res_n == 50:
        x = [3, 4, 6, 3]

    if res_n == 101:
        x = [3, 4, 23, 3]

    if res_n == 152:
        x = [3, 8, 36, 3]

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
# Loss function
##################################################################################


def classification_loss(logit, label):

    # loss = tf.reduce_mean(
    #     tf.nn.softmax_cross_entropy_with_logits_v2(labels=label, logits=logit))
    #return None, logit
    return None, tf.nn.softmax(logit)

    prediction = tf.equal(tf.argmax(logit, -1), tf.argmax(label, -1))
    accuracy = tf.reduce_mean(tf.cast(prediction, tf.float32))
    return None, accuracy

