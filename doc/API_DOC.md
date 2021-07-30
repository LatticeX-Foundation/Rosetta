# Rosetta User API

- [Rosetta User API](#rosetta-user-api)
  - [Overview](#overview)
  - [Control API](#control-api)
    - [Protocol Management](#protocol-management)
      - [`activate(protocol_name=None, protocol_config_str=None, task_id=None)`](#activateprotocol_namenone-protocol_config_strnone-task_idnone)
      - [`deactivate(task_id=None)`](#deactivatetask_idnone)
      - [`get_supported_protocols()`](#get_supported_protocols)
      - [`get_default_protocol_name()`](#get_default_protocol_name)
      - [`get_protocol_name(task_id=None)`](#get_protocol_nametask_idnone)
      - [`get_party_id(task_id=None)`](#get_party_idtask_idnone)
    - [Input and Dataset Utils](#input-and-dataset-utils)
      - [`private_input(node_id: str, input_value: list, task_id=None)`](#private_inputnode_id-str-input_value-list-task_idnone)
      - [`private_console_input(node_id: str, task_id=None)`](#private_console_inputnode_id-str-task_idnone)
      - [`class PrivateDataSet`](#class-privatedataset)
      - [`class PrivateTextLineDataset`](#class-privatetextlinedataset)
    - [Misc API](#misc-api)
      - [`backend_log_to_stdout(flag: bool)`](#backend_log_to_stdoutflag-bool)
      - [`set_backend_logfile(logfile: str, task_id=None)`](#set_backend_logfilelogfile-str-task_idnone)
      - [`set_backend_logpattern(pattern: str)`](#set_backend_logpatternpattern-str)
      - [`set_backend_loglevel(loglevel: int)`](#set_backend_loglevelloglevel-int)
      - [`set_backend_logpattern(pattern: str)`](#set_backend_logpatternpattern-str-1)
      - [`set_float_precision(float_precision: int, task_id=None)`](#set_float_precisionfloat_precision-int-task_idnone)
      - [`set_saver_model(model_nodes: list, task_id=None)`](#set_saver_modelmodel_nodes-list-task_idnone)
      - [`set_restore_model(model_nodes: list, task_id=None)`](#set_restore_modelmodel_nodes-list-task_idnone)
      - [`get_float_precision(task_id=None)`](#get_float_precisiontask_idnone)
  - [Operation API](#operation-api)
    - [Terms and definition](#terms-and-definition)
    - [Common notes](#common-notes)
    - [Computational SecureOps](#computational-secureops)
      - [`SecureAdd(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#secureaddx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureSub(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securesubx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureMul(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securemulx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureFloorDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securefloordivx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securedivx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureDivide(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securedividex-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureTruediv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securetruedivx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureRealDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securerealdivx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureReciprocaldiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securereciprocaldivx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#secureequalx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureGreater(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securegreaterx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureGreaterEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securegreaterequalx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureLess(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securelessx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureLessEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securelessequalx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureLogicalAnd(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securelogicalandx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureLogicalOr(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securelogicalorx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureLogicalXor(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securelogicalxorx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureLogicalNot(x, name=None)`](#securelogicalnotx-namenone)
      - [`SecureMatMul(a, b, transpose_a=False, transpose_b=False, name=None)`](#securematmula-b-transpose_afalse-transpose_bfalse-namenone)
      - [`SecurePow(x, y, name=None, lh_is_const=False, rh_is_const=True)`](#securepowx-y-namenone-lh_is_constfalse-rh_is_consttrue)
      - [`SecureExp(x, name=None)`](#secureexpx-namenone)
      - [`SecureSqrt(x, name=None)`](#securesqrtx-namenone)
      - [`SecureRsqrt(x, name=None)`](#securersqrtx-namenone)
      - [`SecureLog(x, name=None)`](#securelogx-namenone)
      - [`SecureLog1p(x, name=None)`](#securelog1px-namenone)
      - [`SecureHLog(x, name=None)`](#securehlogx-namenone)
      - [`SecureSigmoid(x, name=None)`](#securesigmoidx-namenone)
      - [`SecureRelu(x, name=None)`](#securerelux-namenone)
      - [`SecureReluPrime(x, name=None)`](#securereluprimex-namenone)
      - [`SecureAbs(x, name=None)`](#secureabsx-namenone)
      - [`SecureAbsPrime(x, name=None)`](#secureabsprimex-namenone)
      - [`SecureMax(input_tensor, axis=None, name=None)`](#securemaxinput_tensor-axisnone-namenone)
      - [`SecureMin(input_tensor, axis=None, name=None)`](#securemininput_tensor-axisnone-namenone)
      - [`SecureMean(input_tensor, axis=None, name=None)`](#securemeaninput_tensor-axisnone-namenone)
      - [`SecureReveal(a, reveal_party=None)`](#securereveala-reveal_partynone)
    - [I/O SecureOps](#io-secureops)
      - [`SecureSaveV2(prefix, tensor_names, shape_and_slices,tensors, name=None)`](#securesavev2prefix-tensor_names-shape_and_slicestensors-namenone)
      - [`PrivateInput(x, data_owner, name=None)`](#privateinputx-data_owner-namenone)

## Overview

By using Rosetta framework, users can directly perform training or inference task on all of their respective dataset without leaking any privacy to others just after adding a single code `import latticex.rosetta`  at the header of your existing TensorFlow programs (see our [tutorial documentation](TUTORIALS.md) for more details) in the simplest case. Besides, to make it flexible we also provide some easy-to-use APIs, which we will describe in the following context.

The APIs can mainly be classified into two types: 'Control API', which should be sufficient for most of your tasks, and 'Operation API', a set of TensorFlow-style Operators called `SecureOps`. The first type provides you with the ability to control the context of backend cryptographic protocols, pre-process your datasets and so on. The second type is for advanced usage, and you can use them in the same way as the native `tf.Operation`, where TensorFlow executes them with a stream of ciphertext flowing through them.. 

Moreover, since `Rosetta v1.0.0` multi-threaded execution of multiple tasks has been adopted. The main changes are in 'Control API', as each running task is bind with a secure protocol instance, `task_id` argument is added to those APIs to reference the specified task. In addition, we have carefully handled interface compatibility issues so that the code still works correctly with `Rosetta v0.2.0` and newer versions.

## Control API

### Protocol Management

#### `activate(protocol_name=None, protocol_config_str=None, task_id=None)`

  Activate the protocol for the specific task to carry out your subsequent TensorFlow Operations.

  It is highly recommended that you use this interface explicitly to choose your backend protocol before calling `run` in the TensorFlow session.
  
  > Note: DO NOT call `activate` while your program is executing TensorFlow graph.

  **Args:**

  - **`protocol_name`**: Name of the protocol, which MUST be one of the supported protocols. If this parameter is not provided, the default protocol will be used.

  - **`protocol_config_str`**: The config JSON string that is compatible with your protocol. [deprecated since `v1.0.0`]
  
  - **`task_id`**: ID of the task.


#### `deactivate(task_id=None)`

  Deactivate backend protocol for the specific task.

  All the resources related, such as the network connections and local cache, will be released.

  **Args:**
  - **`task_id`**: ID of the task. 

  > Note: DO NOT call `deactivate` while your program is executing TensorFlow graph.

#### `get_supported_protocols()`

  Get list of all the backend cryptographic protocols.

  You can activate one of the protocols as your backend Ops.

  If you are a protocol developer, you can implement and register your own protocols in the backend, and you will see and use them here just as the native ones.

  **Returns:**
    
  The list of the names of the supported secure protocols.

#### `get_default_protocol_name()`

  Get the name of the default protocol that will be used if none is set.


#### `get_protocol_name(task_id=None)`

  Get the protocol name for the specific task.

#### `get_party_id(task_id=None)`

  Get your party id of for specific task (0, 1, 2 for party `P0`, `P1`, `P2` respectively).

  **Args:**

  - **`task_id`**: ID of the task.


### Input and Dataset Utils

#### `private_input(node_id: str, input_value: list, task_id=None)`

  One party set its private input value to be shared among multi-parties.

  **Args:**

  - **`node_id`**: Node ID, Indicates which node's `input_value` will be shared.
  
  - **`input_value`**: Local input values. ONLY the inputs of the node that has the same node id as the `node_id` will be processed.
  
  - **`task_id`**: ID of the task. 

  **Returns:**
    
  The local shared part, ciphertext, of the real value. Note that each party will has different cipher value that returned.


#### `private_console_input(node_id: str, task_id=None)`

  Just the same as private_input while the values will be fetched from console.

  **Args:**

  - **`node_id`**: Node ID, indicates which node's `input_value` will be shared.
  
  - **`task_id`**: ID of the task. 

  **Returns:**

  The local shared part, ciphertext, of the real value. Note that each party will has different cipher value that returned.

#### `class PrivateDataSet`

  A wrapper class for multiparty to align and 'encrypt' or 'share' its private dataset from a local CSV file or `ndarray` target of numpy. 
  
  Every party's dataset should be either sample-aligned or feature-aligned already.

  When instantiation, for example, if dataset_type == DatasetType.SampleAligned (default dataset_type is SampleAligned),
  assuming node `P0`, node `P1` owns private data, `P1` owns label, then all the parties do respectively:
  ```python
  # get private data from its own data source, set None if no private data
  XX, YY = ...
  X, Y = PrivateDataset(data_owner=("P0","P1"), label_owner=1).load_data(XX, YY)
  ```
  Otherwise, if dataset_type == DatasetType.FeatureAligned, label_owner is useless.
  For example, assuming `P0`, `P1` owns private data and label, then all the parties do respectively:
  ```python
  # get private data from its own data source, set None if no private data
  XX, YY = ... 
  X,y = PrivateDataset(data_owner=("P0","P1"),
                      dataset_type=DatasetType.FeatureAligned).load_data(XX, YY)
  ```
  - **load_X(self, x: str or np.ndarray or None, task_id : str="", \*args, \*\*kwargs)**
    
    Load and 'secret-shared' private ATTRIBUTE values from local dataset files or `ndarray` object of `numpy` for task with ID `task_id`.

    For example, if dataset shape of P0 is N * d0, dataset shape of P1 is N * d1, and P2 has no data at all.
    Then the resulting 'ciphertext' local dataset returned for each party is of the shape N * (d0 + d1).

    Return:
      The shared 'ciphertext' local dataset, and its datatype is string with format specific to your current activated protocol.

  - **load_Y(self, y: str or np.ndarray or None, task_id : str="", \*args, \*\*kwargs)**
    
    Load and 'secret-shared' private LABEL values from local dataset files or `ndarray` object of `numpy` for task with ID `task_id`.
    
    Only the input file of 'label_owner'-party will be processed.
    Return:
      The shared 'ciphertext' local label dataset, and its datatype is string with format specific to your current activated protocol.
  - **load_XY(self, X: str or np.ndarray or None, y: str or np.ndarray or None, task_id : str="", \*args, \*\*kwargs)**

    The combination of the above two functions (`load_X`, `load_Y`).

#### `class PrivateTextLineDataset`

  Private text line dataset, either for one or multiple file inputs.

  It is used in the same way as TextLineDataset, but has one more parameter `data_owner` than TextLineDataset, which represents which party holds the private data.

  For example, assuming node with ID `P0` owns private data, it instantiates like this:
  ```python
  file_x = ...
  dataset_x0 = rtt.PrivateTextLineDataset(
                    file_x, data_owner='P0')  # P0 hold the file data
  ```

  For example, assuming node with ID `P1` owns private data, it instantiates like this:
  ```python
  file_x = ...
  dataset_x1 = rtt.PrivateTextLineDataset(
                    file_x, data_owner='P1')  # P1 hold the file data
  ```


### Misc API

#### `backend_log_to_stdout(flag: bool)`
  
  Enable or disable backend log output to stdout.

#### `set_backend_logfile(logfile: str, task_id=None)`
  
  Set backend log output file path.

  **Args:**
  
  - **`task_id`**: ID of the task.

#### `set_backend_logpattern(pattern: str)`
  Set backend log file path.

  **Args:**
  
  - **`task_id`**: ID of the task.

#### `set_backend_loglevel(loglevel: int)`
  Set backend log output level.

  **Args:**

  - **`loglevel`**: Log level, valid values are as follows:
    - 0: Trace(All log)
    - 1: Debug
    - 2: Audit
    - 3: Info
    - 4: Warn
    - 5: Error
    - 6: Fatal
    - 7: Disable
    
#### `set_backend_logpattern(pattern: str)`
  
  Set backend log output pattern.

  **Args:**
  
  - **`pattern`**: `spdlog` log pattern, Rosetta's logger implementation is `spdlog`, default pattern is `%Y-%m-%d %H:%M:%S.%e|%l|%v`, which indicates that the log output mode is: "year-month-day hour:minute:second.milliseconds|log level|log text|". For more detail refer to: [spdlog-pattern](https://spdlog.docsforge.com/master/3.custom-formatting/#pattern-flags).
  

#### `set_float_precision(float_precision: int, task_id=None)`

  Set floating point precision.

  **Args:**

  - **`float_precision`**: Number of floating point precision bits in fixed point representation, with default value 13.
  
  - **`task_id`**: ID of the task. 


#### `set_saver_model(model_nodes: list, task_id=None)`
  
  Specify which nodes act as model savers.

  **Args:**

  - **`model_nodes`**: Model saver nodes.
  
  - **`task_id`**: ID of the task. 


#### `set_restore_model(model_nodes: list, task_id=None)`
  
  Set nodes to restore model.

  **Args:**

  - **`model_nodes`**: Nodes to restore model.
  
  - **`task_id`**: ID of the task. 


#### `get_float_precision(task_id=None)`
  
  Get floating point precision. 

  **Args:**
  - **`task_id`**: ID of the task. 

  **Returns:** 
  Floating point precision for task with `task_id`.



## Operation API

The main magic component that supports these upper-level conveniences is our implementation of a new suites of privacy-preserving-enabled secure `Operation` based on TensorFlow's flexible extension mechanism for introducing new operation library. To distinguish it from the native TensorFlow API Operation (hereinafter referred to directly as `Ops`), we refer to these customized Operation as `SecureOps`.

Here we describe how to use the various `SecureOps` interfaces supported in the `Rosetta`. Most of interface signature of these `SecureOps` is consistent with the corresponding `Ops`' in TensorFlow, and only in a few cases have we extended the native one with more MPC-related functionality (e.g., the `SaveV2` operation, etc.).

If you need to build your own specific privacy protection model based on Rosetta's underlying API, or are interested in our extending our `SecureOps` set, this is the right place you should start with. In addition, unit tests in the source code can also help you to understand the usage of the various `SecureOps`.

### Terms and definition

We will try to represent each `SecureOp` interface in an clear and easy-to-understand way as far as we can. Occasionally, we will use some cryptographic terms for concision, which you can refer to [glossary document](GLOSSARY.md) for their definition if you are not sure.

### Common notes

1. Unlike the input and output `Tensor` of `Ops` in native TensorFlow, the parameters and return values of `SecureOps` are considered to be a **shared value** in a secret state, unless an explicit declaration is made that an input value is an explicit constant (see the related `SecureOps` interface declaration below). You may not use these 'garbled' values directly.

2. On data type (`dtype`) of  `SecureOps` input and output `Tensor`, **Rosetta's Python frontend will uniformly convert them as `tf.string' in the current version.**

3. For binary operators such as `SecureAdd`, The current `Rosetta v1.0.0` supports `Tensor`s of dimension up to 5, while for unary input operators such as `SecureRelu` the tensor shape is unrestricted (note: `Rosetta v0.2.0` does not support `Tensor`s of dimension more than 2).
  


### Computational SecureOps

#### `SecureAdd(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns `x + y` element-wise.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* broadcasting is supported for this SecureOp.
  
  


#### `SecureSub(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns `x - y` element-wise.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* broadcasting is supported for this SecureOp.
  

  
#### `SecureMul(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns `x * y` element-wise.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* Broadcasting is supported for this SecureOp.


#### `SecureFloorDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`  

  Integral floor division of `x` by `y` in shared status element-wise, rounding toward the most negative integer. For example, 6 / 4 = 1 and (-6) / 4 = -2.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* broadcasting is supported for this SecureOp.
  

#### `SecureDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Alias for `SecureFloorDiv`. Please refer to `SecureFloorDiv`. we recommend you to use `SecureFloorDiv` as you can.


#### `SecureDivide(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Computes Python style division of `x` by `y` element-wise.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.
  
  *NOTE:* 
    
  - broadcasting is supported for this SecureOp.
  - due to its intrinsic algorithm complexity in MPC style to meet the security guarantee, **this SecureOp is comparatively much more time-consuming. So you may aviod to use this SecureOp as possible as you can.**
  - this SecureOp is just the same as `SecureRealDiv` and `SecureTrueDiv`.
  

#### `SecureTruediv(x, y, name=None, lh_is_const=False, rh_is_const=False)`
  
  Alias for `SecureDivide`. Please refer to `SecureDivide`.

 
#### `SecureRealDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Alias for `SecureDivide`. Please refer to `SecureDivide`.


#### `SecureReciprocaldiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  On the whole, we need to calculate the reciprocal of the denominator to achieve division, and then calculate the reciprocal of the denominator times the numerator to get the quotient.

  **Args:**

   - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status.
   - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
   - **`name(optional)`**: A name for the operation, the default value of it is None.
   - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
   - **`rh_is_const(optional)`**: flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  A `Tensor`. Has the same type as `x`.

  
  *NOTE:*
  > The denominator can not be too large(smaller than 10000 at best), otherwise the process will be overflow or influence the precision.

  > Normally,the precision of output  can get close to 1e-4.

  > This SecureOp is just the same as `SecureRealDiv` and `SecureTrueDiv`, but in fact, the reciprocaldiv algorithm is 5 times faster than the `SecureTrueDiv`.


#### `SecureEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x == y)` element-wise, as `0.0` for false and `1.0` for true.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 
  
   * broadcasting is supported for this SecureOp.
  
   * The output values are still in the shared status, just like other `SecureOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**
  

#### `SecureGreater(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x > y)` element-wise, as `0.0` for false and `1.0` for true.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

  > broadcasting is supported for this SecureOp.
  > The output values are still in the shared status, just like other `SecureOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**



#### `SecureGreaterEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x >= y)` element-wise, as 0 for false and 1 for true.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

  > broadcasting is supported for this SecureOp.
  > The output values are still in the shared status, just like other `SecureOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause of your source code directly.**


#### `SecureLess(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x < y)` element-wise, as 0 for false and 1 for true.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

   > Broadcasting is supported for this SecureOp.

   > The output values are still in the shared status, just like other `SecureOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**


#### `SecureLessEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x <= y)` element-wise, as 0 for false and 1 for true.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

  > Broadcasting is supported for this SecureOp.
  > The output values are still in the shared status, just like other `SecureOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**


#### `SecureLogicalAnd(x, y, name=None, lh_is_const=False, rh_is_const=False)`
  
  Returns the truth value of `(x & y)` element-wise, as 0 for false and 1 for true.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

   > Broadcasting is supported for this SecureOp.

   > The output values are still in the shared status, just like other `SecureOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**


#### `SecureLogicalOr(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x | y)` element-wise, as 0 for false and 1 for true.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

  > Broadcasting is supported for this SecureOp.

  > The output values are still in the shared status, just like other `SecureOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**

#### `SecureLogicalXor(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x ^ y)` element-wise, as 0 for false and 1 for true.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

  > Broadcasting is supported for this SecureOp.

  > The output values are still in the shared status, just like other `SecureOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**

#### `SecureLogicalNot(x, name=None)`

  Returns the truth value of `(!x)` element-wise, as 0 for false and 1 for true.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

  > The output values are still in the shared status, just like other `SecureOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**


#### `SecureMatMul(a, b, transpose_a=False, transpose_b=False, name=None)`

  Multiplies matrix `a` by matrix `b`, producing `a` * `b`.

  **Args:**

  - **`a`**: A `Tensor` in TensorFlow, whose values are in shared status. 

  - **`b`**: A `Tensor` in TensorFlow, whose values are in shared status. Must be logically compatible with `a`.

  - **`name(optional)`**: A name for the operation (optional).

  - **`transpose_a(optional)`**: If `True`, `a` is transposed before multiplication. Its default value is `False`.

  - **`transpose_b(optional)`**: If `True`, `b` is transposed before multiplication.Its default value is `False`.

  **Returns:**

  ​	A `Tensor`. each value in it is the inner product of the corresponding vector in `a` and `b`.

  

  *NOTE:*  
  > Just like other binary `SecureOps`, **in the current version, only at most 2-dimension input `Tensor` is supported.**


#### `SecurePow(x, y, name=None, lh_is_const=False, rh_is_const=True)`

  Returns $x^ y$ element-wise. 

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow. **In the current version, every value must be a constant plaintext  integer, and all parties must have the same value.** Besides, `y` should have the same shape as `x`.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `True`. **In the current version, only `True` is supported. In the future version, we will support it to be `False`** 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.


#### `SecureExp(x, name=None)`

  Computes e^x of `x` element-wise. 

  **Args:**

   - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status.
   - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​ A `Tensor`. Has the same type as `x`.

*NOTE*: 
> The current accuracy of the operator is 0.1 ~ 0.01, we will optimize it in the next version.


#### `SecureSqrt(x, name=None)`

  Computes x^(0.5) of `x` element-wise.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status.
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​ A `Tensor`. Has the same type as `x`.


#### `SecureRsqrt(x, name=None)`

  Computes 1/(x^(0.5)) of `x` element-wise by using Newton Raphson method.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status.
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​ A `Tensor`. Has the same type as `x`.


#### `SecureLog(x, name=None)`

  Computes natural logarithm of `x` element-wise. This is optimized version for $x \in [0.0001, 10]$, so **DO NOT** use it for other `x`.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  
  *NOTE*: 
  > The inner implementation of this `SecureLog` is optimized for efficiency by polynomial interpolation. for  $x \in [0.0001, 10]$, which are often used in machine learning tasks. And its average absolute error  is less than  0.01. The detailed segmental polynomials we use is:
  > $$ln(x)=\begin{cases} 85873.96716*x^3 -8360.491679*x^2 + 284.0022382*x -6.805568387& x \in (0.0001, 0.05) \\ 3.404663323 * x^3 -8.668159044*x^2 + 8.253302766*x -3.0312942 & x \in [0.05, 1.2) \\ -0.022636005*x^2+0.463403306*x-0.147409486 & x \in [1.2, 10.0) \end{cases}$$

  > **If your want to use the general and high-precision mathematical natural logarithm, you should use `SecureHLog`.**


#### `SecureLog1p(x, name=None)`

  Computes natural logarithm of `x+1` element-wise.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  

  *NOTE*:  The inner implementation of this `SecureLog1p` is based on `SecureLog`, you may refer to `SecureLog` for more information.


#### `SecureHLog(x, name=None)`

  Computes natural logarithm of `x` element-wise.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  

  *NOTE*:  This is for computing general and high-precision mathematical natural logarithm for all $x$ in domain. **This `SecureOp` is much more time-consuming, so avoid to use this as you can.** 


#### `SecureSigmoid(x, name=None)`

  Computes the sigmoid function, which means  $\frac{1}{1+e^{-x}}$  element-wise.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:*  
  > The inner implementation of this `SecureSigmoid` is  optimized for efficiency by using polynomial interpolation. The piecewise linear polynomials we use is as follows: 
  > $$sigmoid(x)=\begin{cases} 0 & x \in (-\infty,-4] \\ 0.0484792 * x + 0.1998976 & x \in [-4, -2) \\ 0.1928931 * x + 0.4761351 & x \in [-2, 0) \\ 0.1928931 * x + 0.5238649 & x \in [0, 2) \\ 0.0484792 * x + 0.8001024 & x \in [2,4) \\ 1 & x \in [4, \infty) \end{cases}$$ .


#### `SecureRelu(x, name=None)`

  Computes rectified linear functionality, which means max(x, 0) element-wise. 

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.


#### `SecureReluPrime(x, name=None)`

  Computes derivation of rectified linear functionality, which means $1$ if $x >= 0$, otherwise $0$. 

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.


#### `SecureAbs(x, name=None)`

  Computes absolute value of each value in `x`. 

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.


#### `SecureAbsPrime(x, name=None)`

  Computes derivation of  absolute value of each value in `x`, which means $1$ if $x > 0$, otherwise $-1$.

  **Args:**

  - **`x`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  

#### `SecureMax(input_tensor, axis=None, name=None)`

  Computes the maximum of elements across dimensions of a tensor. Reduces `input_tensor` along the dimensions given in `axis`. 
  If `axis` is None, all dimensions are reduced, and a tensor with a single element is returned.

  **Args:**

  - **`input_tensor`**: The tensor to reduce. 

  - **`axis(optional)`**: The dimensions to reduce. If `None` (the default), reduces all dimensions. **In the current version it must be 0, 1 or None.**

  - **`name(optional)`**: A name for the operation (optional).

  **Returns:**

  The reduced `Tensor` with max outputs.

  *Note:*
  > From v1.0.0 or newer version, Rosetta supports up to 5-demension input `Tensor`, while for v0.2.0 or older version, only 2-dimension input `Tensor` is supported.


#### `SecureMin(input_tensor, axis=None, name=None)`

  Computes the minimum of elements across dimensions of a tensor. Reduces `input_tensor` along the dimensions given in `axis`. 
  If `axis` is None, all dimensions are reduced, and a tensor with a single element is returned.

  **Args:**

  - **`input_tensor`**: The tensor to reduce. 

  - **`axis(optional)`**: The dimensions to reduce. If `None` (the default), reduces all dimensions. **In the current version it must be 0, 1 or None.**

  - **`name(optional)`**: A name for the operation (optional).

  **Returns:**

  The reduced `Tensor` with minimum outputs.

  *Note:*
  > From v1.0.0 or newer version, Rosetta supports up to 5-demension input `Tensor`, while for v0.2.0 or older version, only 2-dimension input `Tensor` is supported.


#### `SecureMean(input_tensor, axis=None, name=None)`

  Computes the mean of elements across dimensions of a tensor.
  Reduces `input_tensor` along the dimensions given in `axis`. 

  If `axis` is None, all dimensions are reduced, and a tensor with a single element is returned.

  **Args:**

  - **`input_tensor`**: The tensor to reduce. 
  - **`axis(optional)`**: The dimensions to reduce. If `None` (the default), reduces all dimensions. **In the current version it must be 0, 1 or None.**
  - **`name(optional)`**: A name for the operation (optional).

  **Returns:**

  The reduced `Tensor` with min ouputs.

  *Note:*
  > From v1.0.0 or newer version, Rosetta supports up to 5-demension input `Tensor`, while for v0.2.0 or older version, only 2-dimension input `Tensor` is supported.

#### `SecureReveal(a, reveal_party=None)`

  This auxiliary `SecureOp` can reveal the plaintext value of the `Tensor` `a`. **Since the output of this interface can be plaintext, be cautious and reach a consensus among all the parties when using this in production environment.**  

  **Args:**

  - **`a`**: A `Tensor` in TensorFlow, whose values are in shared status. 
  - **`reveal_party(optional)`**: Configure result nodes who will get the plaintext output, recommand parameter value is result nodes list. **By default, it is set to None, meaning that all result nodes will get the plaintext output values.**
  

  **Returns:**
  
  A `tensor`, of which values can be in plaintext as configured.


### I/O SecureOps

#### `SecureSaveV2(prefix, tensor_names, shape_and_slices,tensors, name=None)`

  This is the underlying Operation to save tensors in V2 checkpoint format. In native TensorFlow, its corresponding operation, `save_v2`,  which has the identical interface signature, is also wrapped by the `tf.train.Saver()` interface, rather than being called directly.  **Unlike the native `Saver`, this `SecureOp` can save the plaintext tensors in files only in specific parties by configuration.** In Rosetta, by default, you have no need to use this `SecureOp` neither, because our `Static Pass` functionality (see its definition in [glossary](GLOSSARY.md)) can help you perform the correct replacement intrinsically.

  The configurational option in `Rosetta v0.2.0` is the `SAVER_MODE` keyword in `CONFIG.json`.  Its  value os parsed as a 3-bit Flag for $[P2\ P1 \ P0]$, so it can be one of the following:

  - $0$: every party saved the local shared value, rather than plaintext value. **This is the default action.**
  - $1$: the saved plaintext file will be stored in party `P0`.
  - $2$: the saved plaintext file will be stored in party `P1`.
  - $3$: the saved plaintext file will be stored both in party  `P0` and` P1`.
  - $4$: the saved plaintext file will be stored in party `P2`.
  - $5$: the saved plaintext file will be stored both in party `P0` and `P2`.
  - $6$: the saved plaintext file will be stored both in party `P1` and `P2`.
  - $7$: the saved plaintext file will be stored in all parties, i.e `P0`, `P1` and `P2`. 

  **Args:**
  
  - **`prefix`**: A `Tensor` of type `string`. Must have a single element. The prefix of the V2 checkpoint to which we write the tensors.
  - **`tensor_name`**: A `Tensor` of type `string`. shape ${N}$. The names of the tensors to be saved.
  - **`shape_and_slices`**: A `Tensor` of type `string`. shape ${N}$.  The slice specs of the tensors to be saved. Empty strings indicate that they are non-partitioned tensors.
  - **`tensors`**:A list of `Tensor` objects, which are in shared status. $N$ tensors to save.
  - **`name`**: A name for the operation (optional).
  
  **Returns:**
  
  The created Operation.
  
  *NOTE*: Every party must have the same configured value so that the system call perform the correct actions. **This Configuration is important due to its output values, which may be sensitive,  are in plaintext . So be cautious and reach a consensus among all the parties.**

#### `PrivateInput(x, data_owner, name=None)`

  Define a private input，this represents a `private` input owned by the specified player into the graph.

  **Args:**
  - **`x`**: The data from data owner, supported data types: int32, int64, float, double, string.
  - **`data_owner`**: The private data owner which is data node id (string type). For compatibility with older versions(earlier than `v1.0.0`), `data_owner` = 0 means computation node party `P0` hold the data, data_owner = 1 means computation node party `P1` hold the data, data_owner = 2 means computation node party `P2` hold the data.
  - **`name`**: A name for the operation (optional).

  **Return value:**
    A `Tensor`, of type string and the same shape as `x`.

  *Note*: 
  > The load data of the return value is encrypted or shared.


