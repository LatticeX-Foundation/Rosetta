from __future__ import print_function

import numpy as np
import warnings
from keras.layers import merge, Input
from keras.layers import Dense, Activation, Flatten
from keras.layers import Convolution2D, MaxPooling2D, ZeroPadding2D, AveragePooling2D
from keras.layers import BatchNormalization

import time
import tensorflow as tf
import latticex.rosetta as rtt
import os
import sys
sys.setrecursionlimit(3000)

TF_WEIGHTS_PATH = 'https://github.com/fchollet/deep-learning-models/releases/download/v0.1/resnet50_weights_tf_dim_ordering_tf_kernels.h5'
TF_WEIGHTS_PATH_NO_TOP = 'https://github.com/fchollet/deep-learning-models/releases/download/v0.1/resnet50_weights_tf_dim_ordering_tf_kernels_notop.h5'


def identity_block(input_tensor, kernel_size, filters, stage, block):
    '''The identity_block is the block that has no conv layer at shortcut

    # Arguments
        input_tensor: input tensor
        kernel_size: defualt 3, the kernel size of middle conv layer at main path
        filters: list of integers, the nb_filters of 3 conv layer at main path
        stage: integer, current stage label, used for generating layer names
        block: 'a','b'..., current block label, used for generating layer names
    '''
    nb_filter1, nb_filter2, nb_filter3 = filters
    bn_axis = 3

    conv_name_base = 'res' + str(stage) + block + '_branch'
    bn_name_base = 'bn' + str(stage) + block + '_branch'

    x = Convolution2D(nb_filter1, 1, 1, name=conv_name_base + '2a', trainable=False)(input_tensor)
    x = BatchNormalization(axis=bn_axis, name=bn_name_base + '2a', trainable=False)(x, training=False)
    x = Activation('relu')(x)

    x = Convolution2D(nb_filter2, kernel_size, kernel_size,
                      border_mode='same', name=conv_name_base + '2b', trainable=False)(x)
    x = BatchNormalization(axis=bn_axis, name=bn_name_base + '2b', trainable=False)(x, training=False)
    x = Activation('relu')(x)

    x = Convolution2D(nb_filter3, 1, 1, name=conv_name_base + '2c', trainable=False)(x)
    x = BatchNormalization(axis=bn_axis, name=bn_name_base + '2c', trainable=False)(x, training=False)

    x = x + input_tensor
    x = Activation('relu')(x)
    return x


def conv_block(input_tensor, kernel_size, filters, stage, block, strides=(2, 2)):
    '''conv_block is the block that has a conv layer at shortcut

    # Arguments
        input_tensor: input tensor
        kernel_size: defualt 3, the kernel size of middle conv layer at main path
        filters: list of integers, the nb_filters of 3 conv layer at main path
        stage: integer, current stage label, used for generating layer names
        block: 'a','b'..., current block label, used for generating layer names

    Note that from stage 3, the first conv layer at main path is with subsample=(2,2)
    And the shortcut should have subsample=(2,2) as well
    '''
    nb_filter1, nb_filter2, nb_filter3 = filters
    bn_axis = 3

    conv_name_base = 'res' + str(stage) + block + '_branch'
    bn_name_base = 'bn' + str(stage) + block + '_branch'

    x = Convolution2D(nb_filter1, 1, 1, subsample=strides,
                      name=conv_name_base + '2a', trainable=False)(input_tensor)
    x = BatchNormalization(axis=bn_axis, name=bn_name_base + '2a', trainable=False)(x, training=False)
    x = Activation('relu')(x)

    x = Convolution2D(nb_filter2, kernel_size, kernel_size, border_mode='same',
                      name=conv_name_base + '2b', trainable=False)(x)
    x = BatchNormalization(axis=bn_axis, name=bn_name_base + '2b', trainable=False)(x, training=False)
    x = Activation('relu')(x)

    x = Convolution2D(nb_filter3, 1, 1, name=conv_name_base + '2c', trainable=False)(x)
    x = BatchNormalization(axis=bn_axis, name=bn_name_base + '2c', trainable=False)(x, training=False)

    shortcut = Convolution2D(nb_filter3, 1, 1, subsample=strides,
                             name=conv_name_base + '1', trainable=False)(input_tensor)
    shortcut = BatchNormalization(axis=bn_axis, name=bn_name_base + '1', trainable=False)(shortcut, training=False)

    x = x + shortcut
    x = Activation('relu')(x)
    return x


def ResNet50(include_top=True, input_tensor=None, classes=1000):
    '''Instantiate the ResNet50 architecture,
    optionally loading weights pre-trained
    on ImageNet. Note that when using TensorFlow,
    for best performance you should set
    `image_dim_ordering="tf"` in your Keras config
    at ~/.keras/keras.json.

    The model and the weights are compatible with both
    TensorFlow and Theano. The dimension ordering
    convention used by the model is the one
    specified in your Keras config file.

    # Arguments
        include_top: whether to include the 3 fully-connected
            layers at the top of the network.
        input_tensor: optional Keras tensor (i.e. xput of `layers.Input()`)
            to use as image input for the model.

    # Returns
        A Keras model instance.
    '''

    bn_axis = 3

    # build model
    x = input_tensor
    # x = ZeroPadding2D((3, 3))(input_tensor)
    x = Convolution2D(64, 7, 7, subsample=(2, 2), name='conv1', trainable=False)(x)
    x = BatchNormalization(axis=bn_axis, name='bn_conv1', trainable=False)(x, training=False)
    x = Activation('relu')(x)
    x = MaxPooling2D((3, 3), strides=(2, 2))(x)

    x = conv_block(x, 3, [64, 64, 256], stage=2, block='a', strides=(1, 1))
    x = identity_block(x, 3, [64, 64, 256], stage=2, block='b')
    x = identity_block(x, 3, [64, 64, 256], stage=2, block='c')

    x = conv_block(x, 3, [128, 128, 512], stage=3, block='a')
    x = identity_block(x, 3, [128, 128, 512], stage=3, block='b')
    x = identity_block(x, 3, [128, 128, 512], stage=3, block='c')
    x = identity_block(x, 3, [128, 128, 512], stage=3, block='d')

    x = conv_block(x, 3, [256, 256, 1024], stage=4, block='a')
    x = identity_block(x, 3, [256, 256, 1024], stage=4, block='b')
    x = identity_block(x, 3, [256, 256, 1024], stage=4, block='c')
    x = identity_block(x, 3, [256, 256, 1024], stage=4, block='d')
    x = identity_block(x, 3, [256, 256, 1024], stage=4, block='e')
    x = identity_block(x, 3, [256, 256, 1024], stage=4, block='f')

    x = conv_block(x, 3, [512, 512, 2048], stage=5, block='a')
    x = identity_block(x, 3, [512, 512, 2048], stage=5, block='b')
    x = identity_block(x, 3, [512, 512, 2048], stage=5, block='c')

    x = AveragePooling2D((7, 7), name='avg_pool')(x)

    if include_top:
        x = Flatten()(x)
        x = Dense(1000, activation='softmax', name='fc1000', trainable=False)(x)
        # rtt.RttKerasDense(classes, name='fc1000')

    return x



##################################################################################
# Show variable
##################################################################################
import tensorflow.contrib.slim as slim
def show_all_variables():
    model_vars = tf.trainable_variables()
    slim.model_analyzer.analyze_vars(model_vars, print_info=True)


##################################################################################
# Load Model
##################################################################################
def load(sess, saver, checkpoint_dir):
    print(" [*] Reading checkpoints...")
    ckpt = tf.train.get_checkpoint_state(checkpoint_dir)
    if ckpt and ckpt.model_checkpoint_path:
        ckpt_name = os.path.basename(ckpt.model_checkpoint_path)
        saver.restore(sess, os.path.join(checkpoint_dir, ckpt_name))
        return True
    else:
        print(" [*] Failed to find a checkpoint")
        return False


test_inptus = tf.placeholder(tf.float32, [1, 224, 224, 3], name='train_inputs')
train_logits = ResNet50(include_top=True, input_tensor=test_inptus)
show_all_variables()

#-------------------------------------------
start_time = time.time()
protocol = "Wolverine"
rtt.activate(protocol)
mpc_player_id = rtt.py_protocol_handler.get_party_id()
rtt.set_backend_loglevel(2)
print("pystats activate elapse:{0} s".format(time.time() - start_time))
#-------------------------------------------

saver = tf.train.Saver(name='v2')
with tf.Session() as sess:
    tf.global_variables_initializer().run()
    could_load = load(sess, saver, 'ckp/')
    if could_load:
        print(" [*] Load SUCCESS", flush=True)
    else:
        print(" [!] Load failed...")


# saver = tf.train.Saver(name='v2')
# with tf.Session() as sess:
#     tf.global_variables_initializer().run()
#     could_load = load(sess, saver, 'ckp/')
#     if could_load:
#         print(" [*] Load SUCCESS")
#     else:
#         print(" [!] Load failed...")


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


# save secure graph
Writer = tf.summary.FileWriter("log/resnet50", tf.get_default_graph())
Writer.close()
rtt.deactivate()