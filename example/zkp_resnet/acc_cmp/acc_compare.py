from utils import *
import numpy as np


def score_zk(f_tf_preds, f_zk_preds,  tag):
    print('\ntag:', tag)
    arr_tf = np.loadtxt(f_tf_preds)
    arr_zk = np.loadtxt(f_zk_preds)

    print('tf shape:', arr_tf.shape)
    print('zk shape:', arr_zk.shape)
    rows = arr_tf.shape[0]
    cols = arr_tf.shape[1]

    res = arr_tf - arr_zk
    res = np.square(res)
    res = np.sum(res, axis=1)
    res = np.sqrt(res)
    np.savetxt('diff-'+tag+'.csv', res, fmt='%.10f')
    res = np.sum(res)
    res = np.sum(res)/rows
    print('L2:', res)

    prediction = np.equal(np.argmax(arr_tf, -1), np.argmax(arr_zk, -1))
    # print(prediction)
    x = np.where(prediction == False)
    # print('diff size:', len(x))
    # print(x)

    labels = load_cifar10()
    labels = labels[:rows, :]
    prediction = np.equal(np.argmax(arr_tf, -1), np.argmax(labels, -1))
    acc_ok = 0
    for i in range(len(prediction)):
        if prediction[i]:
            acc_ok += 1
    print('tf acc: %.4f' % (acc_ok/len(prediction)))
    # x = np.where(prediction == False)
    # print('tf diff size:', len(x))
    # print(x)

    prediction = np.equal(np.argmax(arr_zk, -1), np.argmax(labels, -1))
    acc_ok = 0
    for i in range(len(prediction)):
        if prediction[i]:
            acc_ok += 1
    print('zk acc: %.4f' % (acc_ok/len(prediction)))
    # x = np.where(prediction == False)
    # print('zk diff size:', len(x))
    # print(x)


f_tf = './tf-preds-ResNet50_cifar10.csv'
f_zk = './rtt-preds-ResNet50_cifar10.csv'
score_zk(f_tf, f_zk, 'ResNet50_cifar10')
f_tf = './tf-preds-ResNet101_cifar10.csv'
f_zk = './rtt-preds-ResNet101_cifar10.csv'
score_zk(f_tf, f_zk, 'ResNet101_cifar10')
