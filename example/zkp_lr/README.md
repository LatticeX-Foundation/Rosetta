## Private Inference on Logistic Regression Model

### Overview

This example code is in `example/zkp_lr`.  It is based on the `MNIST` dataset, and  in this sample demo we just train a logistic regression model, a simple binary classifier, to learn whether one hand-written digital image should be labeled as number `0`.  

**It is highly recommended that you run this simple demo to verify that you have installed the Rosetta with ZKP OK.**

You can run all the examples just with `./run.sh`. And you will see the logs in `log` subdirectory.

 And finally, you can see the mean relative errors between the plaintext TensorFlow inference results and ZKP-powered private inference results on MNIST test dataset. 

For example,  you may see:

```
run tf logistic regression   train mnist. ...

run tf logistic regression predict mnist. ...

run zk logistic regression predict mnist. ...

run zk logistic regression predict mnist. model loaded as public ...

run zk logistic regression predict mnist. input is public ...
np.mean(prediction probability relative_error): 3.225990394471199e-05
error_in_bits: 16.45493854813948
```


### Demo Description

We first train a plaintext LR model with the script `tf-logistic_regression_train.py`, and this model will be saved as a checkpoint in subdirectory `./log/ckp0`.  The log file for this script is saved as `./log/mnist_logistic_regression_train.log`. 

And then we test on the test dataset directly with this learned model in another native TF script `tf-logistic_regression_predict.py`, and the log file for this script will be `./log/mnist_logistic_regression_predict.log`.  You may see the testing metrics as follows:


```json
{
 "tag": "tf",
 "score_auc": 0.9841388318716179,
 "score_ks": 0.9522298936368726,
 "threshold_opt": 0.6165876159789364,
 "score_accuracy": 0.9921875,
 "score_precision": 0.9957081545064378,
 "score_recall": 0.9957081545064378,
 "score_f1": 0.9957081545064378
}
```

This script will also output the detailed predication value for each testing sample in file `./log/preds_tf_mnist.csv`, and this is the baseline for comparing later with the private-preserving result to check the precision of ZKP.


And then, we can try our ZKP protocol to secure this inference process with Rosetta. 

First of all, we evaluate on the case where both the testing data and the trained model parameters are kept private for Prover. The following two lines in `run.sh` is for this case.

```bash
python3 rtt-logistic_regression_predict.py --party_id=1 >log/mnist_logistic_regression_predict-1.log 2>&1 &
python3 rtt-logistic_regression_predict.py --party_id=0 >log/mnist_logistic_regression_predict-0.log 2>&1
```

You can see from the script `rtt-logistic_regression_predict.py` that we just `private_input` the testing sample data and restore the plaintext model from checkpoint for Prover.

To check the result, we can reveal and get the testing metrics as follows in `log/mnist_logistic_regression_predict-0.log`:


```json
{
 "tag": "zk",
 "score_auc": 0.9841388318716179,
 "score_ks": 0.9522298936368726,
 "threshold_opt": 0.616577,
 "score_accuracy": 0.9921875,
 "score_precision": 0.9957081545064378,
 "score_recall": 0.9957081545064378,
 "score_f1": 0.9957081545064378
}
```


For later comparison, we also reveal the detailed prediction value in file `log/preds_zk_mnist.csv`.



We also evaluate on the cases when the testing dataset can be public and when the model parameters can be public.  This can be done by running:

```bash
echo "run zk logistic regression predict mnist. Model loaded as public ..."
kill_prog
python3 rtt-logistic_regression_predict.py --party_id=1 --model_public --cfgfile=CONFIG-public.json >log/mnist_logistic_regression_predict-model_public-1.log 2>&1 &
python3 rtt-logistic_regression_predict.py --party_id=0 --model_public --cfgfile=CONFIG-public.json >log/mnist_logistic_regression_predict-model_public-0.log 2>&1

echo "run zk logistic regression predict mnist. Test dataset as public ..."
kill_prog
python3 rtt-logistic_regression_predict.py --party_id=1 --input_public >log/mnist_logistic_regression_predict-input_public-1.log 2>&1 &
python3 rtt-logistic_regression_predict.py --party_id=0 --input_public >log/mnist_logistic_regression_predict-input_public-0.log 2>&1
```

The results are similar.



As said above, we can also compare the mean prediction error for all test dataset. By running `python relative_error.py` in the end, we can get something like:

```
np.mean(prediction probability relative_error): 2.0313663485231745e-05
```

This means that comparing to native TensorFlow inference, The ZKP-backed secure inference has virtually no loss of accuracy.
