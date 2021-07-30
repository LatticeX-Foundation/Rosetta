#!/usr/bin/python
#coding: utf-8

import unittest
from multiprocessing import Process, Pipe
import os
import tensorflow as tf
import numpy as np
import sys
import re
import time
import functools

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

# set output buffer to zero
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

_float_precision = 13

def float2mytype(dv):
  return np.uint64(np.int64(np.double(dv))*(2**_float_precision) + np.int64((np.double(dv)-np.int64(np.double(dv)))*(2**_float_precision)))

def mytype2float(mpc_v):
  return np.double(np.int64(np.uint64(mpc_v))) / (2**_float_precision)


def set_float_precision(float_precision=13):
  _float_precision = float_precision


def _python_reset_stdout(rst):
  sys.stdout = sys.__stdout__  # _utest_origin_logger
  sys.stderr = sys.__stderr__  # _utest_origin_logger
  #_utest_logger.close()


def _core_redirect_stdout_to_file(rosetta, name, mpc_op_name):
  logfile = "log/{}-{}.log".format(mpc_op_name, name)
  rosetta.secure_player.redirect_stdout(logfile)

def _core_restore_stdout(rosetta):
  rosetta.secure_player.restore_stdout()

def _find_tf_local_ops(op_name):
  _plain_ops_support = {
    'SecureAdd': tf.add,
    'SecureSub': tf.subtract,
    'SecureMul': tf.multiply,
    'SecureDiv': tf.div,
    'SecureTruediv': tf.truediv,
    'SecureMatMul': tf.matmul,
    'SecureSigmoid': tf.sigmoid,
    'SecureRelu': tf.nn.relu,
    #'SecureReluPrime': SecureReluPrime,
    'SecureLog': tf.log,
    'SecurePow': tf.pow,
    'SecureMax': tf.reduce_max,
    'SecureMin': tf.reduce_min,
    'SecureMean': tf.reduce_mean,
    'SecureLog1p': tf.log1p,
    'SecureSum': tf.reduce_sum,
    'SecureAddN': tf.math.add_n,
    # 'SecureSaveV2',
    # 'SecureRestoreV2',
    # 'SecureApplyGradientDescent',
    # 'SecureSigmoidCrossEntropy',
    'SecureEqual': tf.equal,
    'SecureEqual': tf.not_equal,
    'SecureGreater': tf.greater,
    'SecureGreaterEqual': tf.greater_equal,
    'SecureLess': tf.less,
    'SecureLessEqual': tf.less_equal,
    'SecureReveal': tf.identity
  }

  if op_name in _plain_ops_support.keys():
    return _plain_ops_support[op_name]
  else:
    print('cannot find mpc op: {}'.format(op_name))
    sys.exit()


def _find_mpc_ops(name, op_name):
  import latticex.rosetta as rst  # should use delay load

  _mpc_ops_support = {
    'SecureAdd': rst.SecureAdd,
    'SecureSub': rst.SecureSub,
    'SecureMul': rst.SecureMul,
    'SecureDiv': rst.SecureDiv,
    'SecureTruediv': rst.SecureTruediv,
    'SecureMatMul': rst.SecureMatMul,
    'SecureSigmoid': rst.SecureSigmoid,
    'SecureRelu': rst.SecureRelu,
    'SecureReluPrime': rst.SecureReluPrime,
    'SecureLog': rst.SecureLog,
    'SecurePow': rst.SecurePow,
    'SecureMax': rst.SecureMax,
    'SecureMin': rst.SecureMin,
    'SecureMean': rst.SecureMean,
    'SecureLog1p': rst.SecureLog1p,
    'SecureSum': rst.SecureSum,
    'SecureAddN': rst.SecureAddN,
    # 'SecureSaveV2',
    # 'SecureRestoreV2',
    # 'SecureApplyGradientDescent',
    # 'SecureSigmoidCrossEntropy',
    'SecureEqual': rst.SecureEqual,
    'SecureNotEqual': rst.SecureNotEqual,
    'SecureGreater': rst.SecureGreater,
    'SecureGreaterEqual': rst.SecureGreaterEqual,
    'SecureLess': rst.SecureLess,
    'SecureLessEqual': rst.SecureLessEqual,
    'SecureReveal': rst.SecureReveal,
  }

  if op_name in _mpc_ops_support.keys():
      mpc_op = _mpc_ops_support[op_name]
  else:
      print('cannot find mpc op: {}'.format(op_name))
      sys.exit()

  return mpc_op, rst


def _find_plain_uint64_local_ops(op_name):
  # input are
  def _rulu_fun(x):
    a = np.double((x) > 0)
    b = a*(x)
    return (b)

  def _pow_fun(x, n):
    dx = (x)
    n = (n)
    if n == 0:
      return (np.ones(dx.shape) if isinstance(dx, np.ndarray) else 1)
    elif n == (1):
      return x
    elif n == (2):
      return (dx * dx)
    else:
      return (dx**n)

  _mpc_ops_support = {
    'SecureAdd': lambda x, y: np.double(x + y),
    'SecureSub': lambda x, y: np.double(x - y),
    'SecureMul': lambda x, y: np.double(x * y),
    # tf.floordiv
    'SecureDiv': lambda x, y: np.double(x / y),
    'SecureTruediv': lambda x, y: np.double(x / y),
    'SecureMatMul': lambda x, y: np.matmul(x, y),
    'SecureSigmoid': lambda x: np.double(1/(1+np.exp(-(x)))),
    'SecureRelu': _rulu_fun,
    'SecureReluPrime': lambda x: (np.double((x) >= 0)),
    'SecurePow': _pow_fun,
    'SecureMax': lambda x: (np.max(x)),
    'SecureMin': lambda x: (np.min(x)),
    'SecureMean': lambda x: (np.mean(x)),
    'SecureLog': lambda x: (np.log(x)),
    'SecureLog1p': lambda x: (np.log(x+1)),
    'SecureSum': lambda x: np.sum(x),
    'SecureAddN': lambda x: (functools.reduce(lambda a, b: (a)+(a), x)),
    # 'SecureSaveV2',
    # 'SecureRestoreV2',
    # 'SecureApplyGradientDescent',
    # 'SecureSigmoidCrossEntropy',
    'SecureEqual': lambda x, y: np.double((x) == (y)),
    'SecureNotEqual': lambda x, y: np.double((x) != (y)),
    'SecureGreater': lambda x, y: np.double((x) > (y)),
    'SecureGreaterEqual': lambda x, y: np.double((x) >= (y)),
    'SecureLess': lambda x, y: np.double((x) < (y)),
    'SecureLessEqual': lambda x, y: np.double((x) <= (y)),
    'SecureReveal': lambda x: np.double(x),
  }

  if op_name in _mpc_ops_support.keys():
    return _mpc_ops_support[op_name]
  else:
    print('cannot find mpc op: {}'.format(op_name))
    sys.exit()

def _plain_local_run_inputs(mpc_op_name, inputs):
  # print("{} running...".format(mpc_op_name))
  # print("{} sys.argv: {}".format(mpc_op_name, sys.argv))
  # print("============ {} input: {}".format(mpc_op_name, inputs))

  outputs = []

  # find the mpc operation
  plain_op = _find_plain_uint64_local_ops(mpc_op_name)
  print("test op: ", plain_op, ", name: ", mpc_op_name)

  tf_outputs = None
  input0 = None
  input1 = None
  inputs = np.double(inputs)
  # binary op test here
  if isinstance(inputs, np.ndarray) and len(inputs) == 2:  # binary operation test
    input0 = inputs[0]
    input1 = inputs[1]
    outputs = plain_op(input0, input1)
  else:
    # unary or reduce operation test
    input0 = inputs
    outputs = plain_op(input0)

  # if isinstance(input0, np.ndarray) and not isinstance(outputs, np.ndarray):
  #   outputs = [outputs]  # fix for single value array
  # else:
    # outputs = outputs.tolist()  # convert to list type
  outputs = outputs.tolist()  # convert to list type
  # print("local plain outputs: {}".format(outputs))
  return outputs

def _tf_local_run_inputs(mpc_op_name, inputs):
  # print("{} running...".format(name))
  # print("{} sys.argv: {}".format(name, sys.argv))
  # print("{} input: {}".format(name, inputs))

  outputs = []

  # find the mpc operation
  mpc_op = _find_tf_local_ops(mpc_op_name)
  tf_outputs = None

  # binary op test here
  if len(inputs) == 1:  # unary operation test
    input0 = (inputs[0])

    tf_input0 = tf.Variable(input0)
    tf_outputs = mpc_op(tf_input0)
  elif len(inputs) >= 2:  # binary operation test
    input0 = (inputs[0])
    input1 = (inputs[1])

    tf_input0 = tf.Variable(input0)
    tf_input1 = tf.Variable(input1)

    # init
    # print('dtypes, tensors:: ', tf_input0.dtype, tf_input1.dtype, tf_input0, tf_input1)
    tf_outputs = mpc_op(tf_input0, tf_input1)
  else:
    print("----!!!! only test unary and binary ops !!!!----")
    sys.exit()

  # session.run and get result
  init = tf.compat.v1.global_variables_initializer()
  sess = tf.compat.v1.Session()
  sess.run(init)

  player_out = sess.run(tf_outputs)
  sess.close()

  outs = (player_out)
  outputs = outs.tolist()  # convert to list type
  print("local tf outputs: {}".format(outputs))
  return outputs


def _tf_mpc_run_inputs(protocol, name, mpc_op_name, inputs, const_pos=-1):
  # print("{} running...".format(name))
  # print("{} sys.argv: {}".format(name, sys.argv))
  # print("{} input: {}".format(name, inputs))

  # redirect  python stdout
  #_python_redirect_stdout_to_file(name, mpc_op_name)
  outputs = []

  # find the mpc operation
  mpc_op, rtt = _find_mpc_ops(name, mpc_op_name)
  rtt.activate(protocol)
  # rtt.py_protocol_handler.set_loglevel(0)

  mpc_reveal = rtt.SecureReveal
  receive_party = 1  # 0 if protocol == "SecureNN" else 1

  tf_outputs = None
  tf_reveal_outputs = None

  # binary op test here
  if isinstance(inputs, np.ndarray) and len(inputs) == 2:  # binary operation test
    print("input0: {}, type: {}\n".format(inputs[0], type(inputs[0])))
    print("input1: {}, type: {}\n".format(inputs[1], type(inputs[1])))

    input0 = inputs[0]
    input1 = inputs[1]

    # TODO: why code blow will cause exception
    tf_input0 = tf.Variable(
        input0) if const_pos != 0 else tf.constant(input0)
    tf_input1 = tf.Variable(
        input1) if const_pos != 1 else tf.constant(input1)

    # init
    # print('+++++======  {}, tensors: {}, {} \n'.format(name, tf_input0, tf_input1))

    # some binary ops has lh_is_const or lh_is_const attribute
    lh_const = True if const_pos == 0 else False
    rh_const = True if const_pos == 1 else False

    if lh_const or rh_const:
      tf_outputs = mpc_op(tf_input0, tf_input1, lh_is_const=lh_const, rh_is_const=rh_const)
    else:
      tf_outputs = mpc_op(tf_input0, tf_input1)
    
    tf_reveal_outputs = mpc_reveal(tf_outputs, receive_party=receive_party)
  else:
    #len(inputs) == 1:# unary operation test
    input0 = inputs

    tf_input0 = tf.Variable(input0)
    tf_outputs = mpc_op(tf_input0)
    tf_reveal_outputs = mpc_reveal(tf_outputs, receive_party=receive_party)

  # session.run and get result
  init = tf.compat.v1.global_variables_initializer()
  sess = tf.compat.v1.Session()

  sess.run(init)

  # player_value = sess.run(tf_outputs)
  # tf_reveal_outputs = mpc_reveal(player_value)
  player_out = sess.run(tf_reveal_outputs)
  sess.close()
  rtt.deactivate()

  # force set p1 all zeros ?
  outputs = np.double(player_out).tolist()  # convert to list type
  # self.assertEqual(np.double(ret).tolist(), np.double(np.array(["1", "2"])).tolist())
  print("{}, tf mpc run output, float : {} ".format(name, outputs))

  return outputs


def _tf_secure_player_func(name, inputs, mpc_op_name, const_pos, pipe, protocol="SecureNN"):
    if name == "p0":
        sys.argv.append("--party_id=0")
    elif name == "p1":
        sys.argv.append("--party_id=1")
    elif name == "p2":
        sys.argv.append("--party_id=2")
    else:
        #sys.argv.append("--party_id=3")
        print("\n----- not support 4pc now!!!!----\n")
        exit()

    print('the player inputs: ', inputs)

    # run tensorflow test process
    player_output = _tf_mpc_run_inputs(protocol, name, mpc_op_name, inputs, const_pos)
    if pipe:
        pipe.send(player_output)

class AddPlayerCluster:
  def __init__(self, name, parties=3, protocol="SecureNN"):
    self.name = name
    self.parties = parties
    self.processes = []
    self.pipes_recv = []
    self.pipes_send = []
    #self.outputs = []
    self.inputs = []
    self.status = 0
    self.protocol = protocol

  def set_protocol(self, protocol):
    self.protocol = protocol

  def run_local(self, p0_in, p1_in, op_name, const_pos=-1):
    reveal_inputs = (p0_in)
    return _plain_local_run_inputs(op_name, reveal_inputs)

  def run_players(self, p0_in, p1_in, op_name, const_pos=-1):
    #p2_in = np.double(p1_in)
    p1_in = p0_in
    p2_in = p0_in
    self.inputs = [np.double(p0_in), np.double(p1_in), np.double(p2_in)]
    for i in range(self.parties):
      #for i in range(2, -1, -1):
      parent_conn, child_conn = None, None
      if i < 2:
        parent_conn, child_conn = Pipe()
        self.pipes_send.append(child_conn)
        self.pipes_recv.append(parent_conn)

      proc = Process(target=_tf_secure_player_func, 
                    args=('p{}'.format(i), self.inputs[i], op_name, const_pos, child_conn, self.protocol, ))
      proc.start()
      self.processes.append(proc)

    #for p in range(self.parties):
    outputs = []
    for p in range(2, -1, -1):
      if p < 2:  # 0,1
        player_output = self.pipes_recv[p].recv()
        self.pipes_recv[p].close()
        self.pipes_send[p].close()
        del self.pipes_recv[p]
        del self.pipes_send[p]
        outputs.append(player_output)
      self.processes[p].join()

    print("run_players outputs float (p1,p0): {}, mpc (p1,p0): {}".format(
        outputs, outputs))

    del self.pipes_recv[:]
    del self.pipes_send[:]
    del self.processes[:]

    outputs.reverse() # P0, P1
    return outputs

from decorator_utest_cases_ import test_binary_sample_0, test_binary_sample_1, test_binary_sample_2, test_binary_sample_3
from decorator_utest_cases_ import test_binary_sample_3, test_binary_sample_4, test_binary_const_sample_4

'''
binary op utest base class
'''
class MpcBinaryUnitTester(unittest.TestCase):
  def __init__(self, test_name):
    unittest.TestCase.__init__(self, test_name)
    self.cluster_player = None
    self.op_name = None
    self.test_name = test_name

  def setOpName(self, op_name):
    self.op_name = op_name

  def getOpName(self):
    return self.op_name

  def tearDown(self):
    del self.cluster_player
    self.cluster_player = None
    print('tearDown {}, test {} ...'.format(self.test_name, self.op_name))

  def setUp(self):
    protocol = "SecureNN"
    if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
      print("***** secure_tests uses ", os.environ["ROSETTA_TEST_PROTOCOL"])
      protocol = os.environ["ROSETTA_TEST_PROTOCOL"]

    self.cluster_player = AddPlayerCluster("3pc-run-test", 3, protocol)
    print('setUp {}, test {} ...'.format(self.test_name, self.op_name))

  def run_local(self, p0_in, p1_in, op_name=None, const_pos=-1):
    if op_name == None:
        op_name = self.op_name
    assert p0_in != None and p1_in != None and op_name != None, 'opname and input should be set'
    p0_p1_outputs = self.cluster_player.run_local(
        p0_in, p1_in, op_name, const_pos=const_pos)
    return p0_p1_outputs

  def run_players(self, p0_in, p1_in, op_name=None, const_pos=-1):
    if op_name == None:
        op_name = self.op_name
    assert p0_in != None and p1_in != None and op_name != None, 'opname and input should be set'

    p0_p1_outputs = self.cluster_player.run_players(p0_in, p1_in, op_name, const_pos=const_pos)
    # p0 = (p0_p1_outputs[0])
    # p1 = (p0_p1_outputs[1])
    return p0_p1_outputs[0]

  # def test_sample0(self):
  #   test_binary_sample_0(self)

'''
mpc unary op unittest base class (pow, log, somecase we can used with max, mean)
'''


class MpcUnaryUnitTester(unittest.TestCase):
  def __init__(self, test_name):
    unittest.TestCase.__init__(self, test_name)
    self.cluster_player = None
    self.op_name = None
    self.test_name = test_name

  def setOpName(self, op_name):
    self.op_name = op_name

  def getOpName(self):
    return self.op_name

  def tearDown(self):
    del self.cluster_player
    self.cluster_player = None
    print('tearDown {}, test {} ...'.format(self.test_name, self.op_name))

  def setUp(self):
    protocol = "SecureNN"
    if "ROSETTA_TEST_PROTOCOL" in os.environ.keys():
      print("***** secure_tests uses ",
            os.environ["ROSETTA_TEST_PROTOCOL"])
      protocol = os.environ["ROSETTA_TEST_PROTOCOL"]
    self.cluster_player = AddPlayerCluster("3pc-run-test", 3, protocol)
    print('setUp {}, test {} ...'.format(self.test_name, self.op_name))

  def run_local(self, p0_in, p1_in, op_name=None):
    if op_name == None:
        op_name = self.op_name
    #assert p0_in != None and p1_in != None and op_name != None
    p0_p1_outputs = self.cluster_player.run_local(p0_in, p1_in, op_name)
    return p0_p1_outputs

  def run_players(self, p0_in, p1_in, op_name=None):
    if op_name == None:
        op_name = self.op_name
    
    p0_p1_outputs = self.cluster_player.run_players(p0_in, p1_in, op_name)
    # p0 = (p0_p1_outputs[0])
    # p1 = (p0_p1_outputs[1])
    return p0_p1_outputs[0]  # only use output of `P0`


if __name__ == '__main__':
    unittest.main()
