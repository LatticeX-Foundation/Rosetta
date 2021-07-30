import concurrent.futures
import numpy as np


PRECISION = 8.0 / 100
def check_mpc_results(real_val, expect_val):
    real_val = real_val.astype(float) # convert string to float value
    real_val = np.asarray(real_val)
    real_val = real_val.reshape(-1)
    expect_val = np.asarray(expect_val)
    expect_val = expect_val.reshape(-1)
    print("real val:{}".format(real_val))
    print("except val:{}".format(expect_val))
    limit_precision = np.full(np.array(expect_val).shape, PRECISION, np.float)
    for i in range(len(expect_val)):
        lh = np.asarray(real_val[i]).reshape(-1)
        rh = np.asarray(expect_val[i]).reshape(-1)
        print(lh)
        print(rh)
        for j in range(len(rh)):
            print("{0} cmp {1}".format(lh[j], float(rh[j])))
            if (abs(lh[j] - float(rh[j])) > PRECISION):
                return False
    return True


def mt_unit_test_fw(funcs):
    task_id = len(funcs)
    with concurrent.futures.ThreadPoolExecutor() as executor:
        futures = []
        for unit_func in funcs:
            futures.append(executor.submit(unit_func, str(task_id)))
            task_id -= 1

        for future in concurrent.futures.as_completed(futures):
            print(future.result())
            pass

