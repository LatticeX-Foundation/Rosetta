import time, sys
import numpy as np
np.set_printoptions(suppress=True)

import tensorflow as tf
# import rosetta
import latticex.rosetta as rtt
assert rtt

# sys.stdout zero buffer
class ZeroBufferOut(object):
    def __init__(self, stream):
        self.stream = stream

    def write(self, data):
        self.stream.write(data)
        self.stream.flush()

    def writelines(self, datas):
        self.stream.writelines(datas)
        self.stream.flush()

    def __getattr__(self, attr):
        return getattr(self.stream, attr)

sys.stdout = ZeroBufferOut(sys.stdout)

def create_run_session(target):
    print("-----  create_init_protocol_player ---")
    print("target: ", target, ", type:", type(target))
    init = tf.global_variables_initializer()
    with tf.Session() as sess:
        sess.run(init)
        result = sess.run(target)

    return result

input0 = [[-2.1, 1.22, 2.2],
       [ 1.034, 1.2245, 2.6],
       [ 2, 5, 6]]

input1 = [
  [-2.1, 1.22, 2.2],
  [ 1.034, 1.2245, 2.6],
  [ 2.,  5., 6.]
]

if __name__ == "__main__":
  rtt.activate("SecureNN")
  const_pos = 1
  tf_input0 = tf.Variable(input0) if const_pos != 0 else tf.constant(input0)
  tf_input1 = tf.Variable(input1) if const_pos != 1 else tf.constant(input1)
  ret = tf.add(tf_input0, tf_input1)
  
  result = create_run_session(ret)
  print("the result : ", result)