from sklearn.metrics import mean_absolute_error, mean_squared_log_error, median_absolute_error
from sklearn.metrics import roc_auc_score, roc_curve, precision_recall_curve, auc
from sklearn.metrics import precision_score, accuracy_score, recall_score, f1_score
from sklearn.metrics import mean_squared_error, explained_variance_score, r2_score
import csv
import pandas as pd
import numpy as np
import json


def read_dataset(file_name=None):
    if file_name is None:
        print("Error! No file name!")
        return
    res_data = []
    with open(file_name, 'r') as f:
        cr = csv.reader(f)
        for each_r in cr:
            curr_r = [np.array([v], dtype=np.float_)[0] for v in each_r]
            res_data.append(curr_r)
            # print(each_r)
    return res_data


def savecsv(file_name, tf_tensor):
    """
    only for numpy.narray
    """
    np.savetxt(file_name, tf_tensor, fmt="%.10f", delimiter=",")


def loadcsv(file_name):
    """
    only for numpy.narray
    """
    return np.loadtxt(file_name, delimiter=",")


def pretty(d):
    """ d is a dict"""
    return json.dumps(d, indent=2, ensure_ascii=False)


def score_logistic_regression(prediction_prob, target, n=0.005, ratio=True,
                              list_metrics=['tag', 'score_auc', 'score_ks',
                                            'threshold_opt', 'score_accuracy', 'score_precision', 'score_recall',
                                            'score_f1'], tag=''):
    fpr, tpr, thresholds = roc_curve(target, prediction_prob, pos_label=1)
    score_auc = auc(fpr, tpr)

    crossfreq = pd.crosstab(prediction_prob[:, 0], target[:, 0])
    crossdens = crossfreq.cumsum(axis=0) / crossfreq.sum()
    crossdens['gap'] = abs(crossdens[0] - crossdens[1])
    score_ks = crossdens[crossdens['gap'] ==
                         crossdens['gap'].max()]['gap'].iloc[0]

    threshold_opt = thresholds[np.argmax(np.abs(tpr - fpr))]
    prediction = (prediction_prob >= threshold_opt).astype('int64')

    score_accuracy = accuracy_score(target, prediction)
    score_precision = precision_score(target, prediction)
    score_recall = recall_score(target, prediction)
    score_f1 = f1_score(target, prediction)

    scope = locals()
    dict_evaluation_metrix = dict([(i, eval(i, scope)) for i in list_metrics])
    return dict_evaluation_metrix


def score_linear_regression(y_pred, y_true, list_metrics=['tag', 'mse', 'rmse', 'mae', 'evs', 'r2'], tag=''):
    mse = mean_squared_error(y_true, y_pred)
    rmse = np.sqrt(mean_squared_error(y_true, y_pred))
    mae = median_absolute_error(y_true, y_pred)
    evs = explained_variance_score(y_true, y_pred)
    r2 = r2_score(y_true, y_pred)

    scope = locals()
    dict_evaluation_metrix = dict([(i, eval(i, scope)) for i in list_metrics])
    return dict_evaluation_metrix
