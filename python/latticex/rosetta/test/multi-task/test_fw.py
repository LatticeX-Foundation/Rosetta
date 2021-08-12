import concurrent.futures
import numpy as np
import tensorflow as tf
import latticex.rosetta as rtt
import time


PRECISION = 16. / 100
def check_mpc_results(real_val, expect_val, precision=PRECISION):
    if type(real_val) is np.ndarray:
        real_val = real_val.astype(float) # convert string to float value
    else:
        real_val = float(real_val)     
    real_val = np.asarray(real_val)
    real_val = real_val.reshape(-1)
    expect_val = np.asarray(expect_val)
    expect_val = expect_val.reshape(-1)
    print("real val:{}".format(real_val))
    print("expect val:{}".format(expect_val))
    limit_precision = np.full(np.array(expect_val).shape, precision, np.float)
    for i in range(len(expect_val)):
        lh = np.asarray(real_val[i]).reshape(-1)
        rh = np.asarray(expect_val[i]).reshape(-1)
        for j in range(len(rh)):
            print("{0} cmp {1}".format(lh[j], float(rh[j])))
            if (abs(lh[j] - float(rh[j])) > precision):
                print("lr:{0} cmp rh:{1} fail".format(lh[j], float(rh[j])))
                return False
    return True


task_id = 1
def mt_unit_test_fw(funcs):
    #task_id = 1
    global task_id
    all_task = []
    all_res = []

    try:
        with concurrent.futures.ThreadPoolExecutor() as executor: 
            for unit_func in funcs:
                all_task.append(executor.submit(unit_func, str(task_id)))
                task_id += 1
    
        concurrent.futures.wait(all_task, return_when=concurrent.futures.ALL_COMPLETED)

        for item in all_task:
            all_res.append(item.result())

        print("all task has finish, the results:{}".format(all_res))
        if (len(all_res) == 0 or len(all_res) != len(funcs)):
            print("Fail")

        if (False in all_res or None in all_res):
            print("Fail")
        else:
            print("Pass")
        time.sleep(1)
    except Exception as e:
        print(str(e))
        print("Fail")


def bin_op_test(protocol, task_id, tf_op, x_init, y_init, expect_val):
    Result = True
    local_g = tf.Graph()
    with local_g.as_default():
        X = tf.Variable(x_init)
        Y = tf.Variable(y_init)
        Z = tf_op(X, Y)
        rv_Z = rtt.SecureReveal(Z)
        init = tf.compat.v1.global_variables_initializer()

        try:
            rtt.activate(protocol, task_id=task_id)
            config = tf.ConfigProto(inter_op_parallelism_threads = 16, intra_op_parallelism_threads = 16)
            with tf.Session(task_id=task_id, config=config) as sess:
                sess.run(init)        
                real_Z = sess.run(rv_Z)
                res = check_mpc_results(real_Z, expect_val)
                if (res == False):
                    Result = False
            rtt.deactivate(task_id=task_id)
        except Exception as e:
            print(str(e))
            Result = False
    
    return Result


def bin_op_rh_const_test(protocol, task_id, tf_op, x_init, y_init, expect_val):
    Result = True
    local_g = tf.Graph()
    with local_g.as_default():
        X = tf.Variable(x_init)
        Z = tf_op(X, y_init)
        rv_Z = rtt.SecureReveal(Z)
        init = tf.compat.v1.global_variables_initializer()

        try:
            rtt.activate(protocol, task_id=task_id)
            config = tf.ConfigProto(inter_op_parallelism_threads = 16, intra_op_parallelism_threads = 16)
            with tf.Session(task_id=task_id, config=config) as sess:
                sess.run(init)        
                real_Z = sess.run(rv_Z)
                res = check_mpc_results(real_Z, expect_val)
                if (res == False):
                    Result = False
            rtt.deactivate(task_id=task_id)
        except Exception as e:
            print(str(e))
            Result = False
    
    return Result


def unary_op_test(protocol, task_id, tf_op, x_init, expect_val, precision=PRECISION):
    Result = True
    local_g = tf.Graph()
    with local_g.as_default():
        X = tf.Variable(x_init)
        Z = tf_op(X)
        rv_Z = rtt.SecureReveal(Z)
        init = tf.compat.v1.global_variables_initializer()

        try:
            rtt.activate(protocol, task_id=task_id)
            config = tf.ConfigProto(inter_op_parallelism_threads = 16, intra_op_parallelism_threads = 16)
            with tf.Session(task_id=task_id, config=config) as sess:
                sess.run(init)        
                real_Z = sess.run(rv_Z)
                res = check_mpc_results(real_Z, expect_val, precision)
                if (res == False):
                    Result = False
            rtt.deactivate(task_id=task_id)
        except Exception as e:
            print(str(e))
            Result = False
    
    return Result


def reduce_op_test(protocol, task_id, tf_op, x_init, expect_val):
    Result = True
    local_g = tf.Graph()
    with local_g.as_default():
        X = tf.Variable(x_init)
        Z = tf_op(X)
        rv_Z = rtt.SecureReveal(Z)
        init = tf.compat.v1.global_variables_initializer()

        try:
            rtt.activate(protocol, task_id=task_id)
            config = tf.ConfigProto(inter_op_parallelism_threads = 16, intra_op_parallelism_threads = 16)
            with tf.Session(task_id=task_id, config=config) as sess:
                sess.run(init)        
                real_Z = sess.run(rv_Z)
                res = check_mpc_results(real_Z, expect_val)
                if (res == False):
                    Result = False
            rtt.deactivate(task_id=task_id)
        except Exception as e:
            print(str(e))
            Result = False
    
    return Result
