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



def network(x0, is_training=True, reuse=False):
    with tf.variable_scope("network", reuse=reuse):
        ch = 32  # paper is 64
        x0 = conv(x0, channels=ch, kernel=3, stride=1, scope='conv_0')
        # x0._raw.set_shape([32, 28, 28, ch])
        x0 = batch_norm(x0, is_training, scope='batch_norm_0')
        x0 = tf.nn.relu(x0)
        # x0 = avg_pooling(x0)
        x0 = max_pooling(x0)
        x0 = fully_conneted(x0, 10, scope='logit')
        # x0 = tf.argmax(x0, -1)
        x0 = tf.nn.softmax(x0)
        return x0



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

    PassObj = StaticReplacePass()
    if isinstance(train_logits, rtt_ts.RttTensor):
        train_logits = PassObj.run(train_logits._raw)
    else:
        train_logits = PassObj.run(train_logits)
    #------------------------------------------------------------------------------



try:
    build_model()
    print("Pass")
except Exception as e:
    print(str(e))
    print("Fail")




Writer = tf.summary.FileWriter("log/conv2d", tf.get_default_graph())
Writer.close()
rtt.deactivate()
