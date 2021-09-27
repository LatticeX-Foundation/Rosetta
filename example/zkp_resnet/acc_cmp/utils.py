import tensorflow as tf
import tensorflow.contrib.slim as slim
import os
from keras.datasets import cifar10, cifar100, mnist, fashion_mnist
from keras.utils import to_categorical
import numpy as np
import random
from scipy import misc


def load_cifar10():
    (train_data, train_labels), (test_data, test_labels) = cifar10.load_data()
    test_labels = to_categorical(test_labels, 10)
    test_labels = test_labels[:10000, :]

    return test_labels
