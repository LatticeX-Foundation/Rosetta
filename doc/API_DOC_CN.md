# Rosetta 算子API文档

- [Rosetta 算子API文档](#rosetta-算子api文档)
  - [概述](#概述)
  - [控制类API](#控制类api)
    - [协议管理](#协议管理)
      - [`activate(protocol_name=None, protocol_config_str=None, task_id=None)`](#activateprotocol_namenone-protocol_config_strnone-task_idnone)
      - [`deactivate(task_id=None)`](#deactivatetask_idnone)
      - [`get_supported_protocols()`](#get_supported_protocols)
      - [`get_default_protocol_name()`](#get_default_protocol_name)
      - [`get_protocol_name(task_id=None)`](#get_protocol_nametask_idnone)
      - [`get_party_id(task_id=None)`](#get_party_idtask_idnone)
    - [输入和数据集工具类](#输入和数据集工具类)
      - [`private_input(node_id: str, input_value, task_id=None)`](#private_inputnode_id-str-input_value-task_idnone)
      - [`private_console_input(node_id: str, task_id=None)`](#private_console_inputnode_id-str-task_idnone)
      - [`class PrivateDataSet`](#class-privatedataset)
      - [`class PrivateTextLineDataset`](#class-privatetextlinedataset)
    - [杂项](#杂项)
      - [`backend_log_to_stdout(flag: bool)`.](#backend_log_to_stdoutflag-bool)
      - [`set_backend_logfile(logfile: str, task_id=None)`](#set_backend_logfilelogfile-str-task_idnone)
      - [`set_backend_logpattern(pattern: str)`](#set_backend_logpatternpattern-str)
      - [`set_backend_loglelevel(loglevel: int)`](#set_backend_loglelevelloglevel-int)
      - [`set_backend_logpattern(pattern: str)`](#set_backend_logpatternpattern-str-1)
      - [`set_float_precision(float_precision: int, task_id=None)`](#set_float_precisionfloat_precision-int-task_idnone)
      - [`set_saver_model(model_nodes: list, task_id=None)`](#set_saver_modelmodel_nodes-list-task_idnone)
      - [`set_restore_model(model_nodes: list, task_id=None)`](#set_restore_modelmodel_nodes-list-task_idnone)
      - [`get_float_precision(task_id=None)`](#get_float_precisiontask_idnone)
  - [算子类型API](#算子类型api)
    - [术语和定义](#术语和定义)
    - [常见说明](#常见说明)
    - [计算类型SecureOps](#计算类型secureops)
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
      - [`SecureSaveV2(prefix, tensor_names, shape_and_slices, tensors, name=None)`](#securesavev2prefix-tensor_names-shape_and_slices-tensors-namenone)
      - [`PrivateInput(x, data_owner, name=None)`](#privateinputx-data_owner-namenone)


## 概述

> 注意，此文档内容可能会有所滞后，最新的接口文档请参考[用户接口文档](API_DOC.md)。

在Rosetta框架中，用户只需添加`import latticex.rosetta`到程序头部，便可以将TensorFlow明文训练或推理程序转换为基于隐私计算的训练或推断（可以参考我们的[tutorial文档](TUTORIALS_CN.md)）。此外，为了使其具有灵活性，我们还提供了一些易于使用的API，我们将在下文中描述这些API 。

这些API主要可以分为两种类型，"控制类API"，它应该足以满足你的大部分任务，以及 "操作类API"，一组TensorFlow风格的操作符，称为 "安全算子"。第一种类型为你提供了控制后端加密协议的上下文的能力，预处理你的数据集等等。第二种类型是用于高级用法，你可以类似本地`tf.Operation`的方式使用它们，TensorFlow执行它们的时候，其中流动的是加密数据流。

此外，从`Rosetta v1.0.0`开始，"多任务-多线程执行"已经被采用。主要的变化在 "控制类API"，因为每个运行的任务都与安全协议实例绑定，`task_id'参数被添加到这些API中以引用指定的任务和协议实例。此外，我们细心地处理了接口兼容性问题，使用v0.2.0及更新版本的代码依然能够正确运行。


## 控制类API

### 协议管理

#### `activate(protocol_name=None, protocol_config_str=None, task_id=None)`

  激活特定任务的协议，以执行你的后续TensorFlow操作。

  强烈建议你在TensorFlow会话Session调用`run`之前明确地使用这个接口来选择你的后端协议。
  
  > 注意：当你的程序正在执行TensorFlow图时，请不要调用`activate`。

  **参数：**

  - **`protocol_name`**: 协议的名称，必须是支持的协议之一。如果没有提供这个参数，将使用默认协议。

  - **`protocol_config_str`**: 与你的协议兼容的配置JSON字符串。（***v1.0.0后该参数已废弃***）。
  
  - **`task_id`**: 任务ID。


#### `deactivate(task_id=None)`

  停用特定任务的后端协议。

  所有相关的资源，如网络连接和本地缓存，将被释放。

  **参数：**
  - **`task_id`**: 关联激活协议实例的任务ID。

  > 注意：当你的程序在执行TensorFlow图时，不要调用`deactivate`。


#### `get_supported_protocols()`

  获取所有后端加密协议的列表。

  你可以激活其中一个协议作为你的后端Ops。

  如果你是一个协议开发者，你可以在后端实现并注册你自己的协议，你将在这里看到并使用它们，就像本地协议一样。

  **返回：**
    
  支持的安全协议的名称列表。


#### `get_default_protocol_name()`

  获取默认协议的名称，如果没有设置，将使用该默认协议。


#### `get_protocol_name(task_id=None)`

  获取特定任务的协议名称。


#### `get_party_id(task_id=None)`

  获取特定任务的计算节点角色ID（0，1，2分别表示参与方`P0`, `P1`，`P2`）。

  **参数:**

  - **`task_id`**: 任务ID。



### 输入和数据集工具类

#### `private_input(node_id: str, input_value, task_id=None)`

  一方设置其私人输入值，在多方之间共享。

  **参数：**

  - **`node_id`**: 节点ID， 表示哪一方的节点的`输入值'将被共享（为了兼容整数0，1，2将分别表示为计算节点P0， P1， P0）。
  
  - **`input_value`**: 本地输入值。只有节点ID与"node_id "相同的的节点，其输入值才将被处理（输入）。
  
  - **`task_id`**: 任务ID。

  **返回值：**
    
  真实值的本地共享部分，即密码文本。注意，每一方都会有不同的密码值返回。


#### `private_console_input(node_id: str, task_id=None)`

  与private_input相同，但其值将从控制台获取。

  **参数：**

  - **`node_id`**: 节点ID， 表示节点的`input_value'将被共享。
  
  - **`task_id`**: 任务ID。

  **返回值：**

  真实值的本地共享部分，即加密文本。注意，每一方都会有不同的密码值返回。

#### `class PrivateDataSet`

  一个封装类，用于多方对齐数据隐私输入，载入"加密 "或 "共享 "的本地CSV文件或numpy的`ndarray`目标的私有数据集。
  
  每一方的数据集都应该已经被样本对齐或特征对齐。

  在实例化时，例如，如果dataset_type == DatasetType.SampleAligned（默认dataset_type是SampleAligned）。
  假设节点`P0`、节点`P1`拥有私有数据，`P1`拥有标签，那么所有各参与方分别执行如下代码：
  ```python
  # 从自己的数据源获取私有数据，如果没有私有数据则设置为None
  XX, YY = ...
  X, Y = PrivateDataset(data_owner=("P0", "P1"), label_owner=1).load_data(XX, YY)
  ```
  否则，如果dataset_type == DatasetType.FeatureAligned，label_owner是无用的。
  例如，假设`P0`、`P1`拥有私有数据和标签，那么所有各方都分别执行如下代码：
  ```python
  # 从自己的数据源获取私有数据，如果没有私有数据则设置为None
  XX, YY = ... 
  X,y = PrivateDataset(data_owner=("P0", "P1"),
                      dataset_type=DatasetType.FeatureAligned）.load_data(XX, YY)
  ```

  - **`load_X(self, x: str or np.ndarray or None, task_id : str="", *args, **kwargs)`**
    
    为ID为`task_id`的任务从本地数据集文件或`numpy`的`ndarray`对象中加载和'秘密共享'私人ATTRIBUTE值。

    例如，如果P0的数据集形状是N * d0，P1的数据集形状是N * d1，而P2没有数据。
    那么为每一方返回的"密文"本地数据集的形状是N * (d0 + d1)。

    返回值：
      "密文"本地标签数据集，其数据类型为指定协议的string类型。

  - **`load_Y(self, y: str or np.ndarray or None, task_id : str="", *args, **kwargs)`**
    
    从本地数据集文件或 "numpy "的 "ndarray "对象为ID为 "task_id "的任务加载和 "秘密共享 "的私有LABEL值。
    
    只有'label_owner'一方的输入文件会被处理。
    返回值：
      "密文"本地标签数据集，其数据类型为指定协议的string类型。

  - **`load_XY(self, X: str or np.ndarray or None, y: str or np.ndarray or None, task_id : str="", *args, **kwargs)`**

    功能为上述两个函数（`load_X`, `load_Y`）的组合。
    返回值：
      "密文"本地标签数据集，其数据类型为指定协议的string类型。


#### `class PrivateTextLineDataset`

  私有文本文件行数据集，可以为一个或多个文件输入。

  它的使用方式与TextLineDataset相同，但比TextLineDataset多了一个参数`data_owner`，代表哪一方持有私有数据。

  例如，假设ID为`P0`的节点拥有私有数据，它的实例化是这样的：
  ```python
  file_x = ...
  dataset_x0 = rtt.PrivateTextLineDataset(
                    file_x, data_owner='P0') # P0持有文件数据
  ```

  例如，假设ID为`P1`的节点拥有私有数据，它的实例化是这样的：
  ```python
  file_x = ...
  dataset_x1 = rtt.PrivateTextLineDataset(
                    file_x, data_owner='P1') # P1持有文件数据
  ```

### 杂项

#### `backend_log_to_stdout(flag: bool)`.
  
  启用或禁用后端日志输出到标准输出stdout。

#### `set_backend_logfile(logfile: str, task_id=None)`
  
  设置后端日志输出文件路径。

  **参数：**
  
  - **`task_id`**: 任务的ID。

#### `set_backend_logpattern(pattern: str)`

  设置后端日志文件路径。

  **参数：**
  
  - **`task_id`**: 任务的ID。

#### `set_backend_loglelevel(loglevel: int)`

  设置后端日志输出级别。

  **参数：**

  - **`loglevel`**: 日志级别，有效值如下。
    - 0: 跟踪(所有日志)
    - 1: 调试
    - 2: 审计
    - 3: 信息
    - 4: 警告
    - 5: 错误
    - 6: 致命
    - 7: 禁用
    
#### `set_backend_logpattern(pattern: str)`
  
  设置后端日志输出模式。

  **参数：**
  
  - **`pattern`**: 
  `spdlog`日志模式。Rosetta后端日志引使用`spdlog`，默认模式为`%Y-%m-%d %H:%M:%S.%e|%l|%v`（表示日志输出模式为：”年-月-日 小时：分钟：秒.毫秒 | 日志级别 | 日志文本 |“），更多细节请参考: [spdlog-pattern](https://spdlog.docsforge.com/master/3.custom-formatting/#pattern-flags)。
  

#### `set_float_precision(float_precision: int, task_id=None)`

  设置浮点精度。

  **参数：**

  - **`float_precision`**: 定点表示法中的浮点精度位数，默认值为13。
  
  - **`task_id`**: 任务ID。


#### `set_saver_model(model_nodes: list, task_id=None)`
  
  指定哪些节点作为模型保存者。

  **参数：**

  - **`model_nodes`**: 模型保存节点列表。
  
  - **`task_id`**: 任务ID。


#### `set_restore_model(model_nodes: list, task_id=None)`
  
  设置节点以恢复模型。

  **参数：**

  - **`model_nodes`**: 要恢复模型的节点列表。
  
  - **`task_id`**: 任务ID。


#### `get_float_precision(task_id=None)`
  
  获取定点表示法中的浮点精度位数。

  **参数：**
  - **`task_id`**: 任务ID。

  **返回值：** 
  指定任务ID索引的协议使用的浮点精度位数（定点表示法）。
  


## 算子类型API

支持这些上层便利的隐私推理或训练，主要依赖的魔法组件是，基于TensorFlow用于引入新的操作库的灵活扩展机制实现的一套新的隐私保护安全算子（`Operation`）。为了区别于原生的TensorFlow API操作（以下直接称为 "Ops"），我们将这些定制的操作称为安全算子或隐私算子 "SecureOps"。

这里我们描述如何使用 "Rosetta v0.2.0 "版本中支持的各种 "SecureOps "接口。这些`SecureOps'的大部分接口签名与TensorFlow中相应的`Ops'一致，只有在少数情况下，我们用更多的MPC相关功能扩展了本地的`Ops'（例如，`SaveV2'操作，等等）。

如果你需要基于Rosetta的底层API建立你自己的特定隐私保护模型，或者对我们的`SecureOps`实现感兴趣，这是你应该开始的地方。此外，源代码中的单元测试也可以帮助你了解各种`SecureOps`的用法。

### 术语和定义

我们将尽可能地用清晰易懂的方式来表示每个`SecureOps`接口。有时候为了简洁起见，我们会使用一些密码学术语，如果你不确定，可以参考[词汇表文件](GLOSSARY.md)了解其定义。

### 常见说明

1. 与原生TensorFlow中`Ops`的输入和输出`Tensor`不同，`SecureOps`的参数和返回值被认为是一个处于秘密状态的**共享值，除非明确声明输入值是常量（见下面相关的`SecureOps`接口声明）。你不可以直接使用这些 "加密"值。

2. 关于`SecureOps`输入和输出`Tensor`的数据类型（`dtype'），**Rosetta的Python前端在当前版本中将统一转换为`tf.string'。

3. 对于二进制运算符，如`SecureAdd`，当前的`Rosetta v1.0.0`支持维度不超过5的`Tensor`，而对于单进制运算符，如`SecureRelu`，其张量形状不受限制（注意：`Rosetta v0.2.0`不支持维度超过2的`Tensor`）。
  


### 计算类型SecureOps

#### `SecureAdd(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  逐个元素的执行加法操作，得到 `x+y `。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意*: 这个SecureOp支持Tensorflow中的广播机制。
  
  

#### `SecureSub(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  逐个元素的执行减法操作，得到 `x-y `。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意*:这个SecureOp支持Tensorflow中的广播机制。

  

#### `SecureMul(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  逐个元素的执行乘法操作，得到 `x*y `。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意*:这个SecureOp支持Tensorflow中的广播机制。

  


#### `SecureFloorDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`  

  逐个对应元素的执行整除操作得到`x/y`，且结果向负数方向倾斜。比如$6 / 4 = 1$ 和 $(-6) / 4 = -2$。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意*:这个SecureOp支持Tensorflow中的广播机制。


#### `SecureDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  等价于 `SecureFloorDiv`. 请参考 `SecureFloorDiv`的接口说明。我们更推荐你使用`SecureFloorDiv` 而不是此接口以保持和TensorFlow的一致。


#### `SecureDivide(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  逐个元素的执行带小数结果的除法得到`x/y`. 

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意*:
  > 这个SecureOp支持Tensorflow中的广播机制。

  > 由于在Secure下实现此算子本身的复杂度较高，此算子的计算时间开销和通讯数据量开销都相对更大。在具体的上层模型构建中，应该尽可能的避免直接使用此算子。


#### `SecureTruediv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  等价于 `SecureDivide`. 请参考 `SecureDivide`的接口说明.


#### `SecureRealDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  等价于 `SecureDivide`. 请参考 `SecureDivide`的接口说明.


#### `SecureReciprocaldiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  总的来说，我们需要计算分母的倒数来实现除法，然后计算分母的倒数乘以分子来得到商。


  **参数：**

   - **`x`**:  TensorFlow中的一个`Tensor`，其值处于共享状态。
   - **`y`**:  TensorFlow中的一个张量，其值处于共享状态， 其必须具有与`x`相同的类型。
   - **`name（可选）`**:  操作的名称，其默认值为None。
   - **`lh_is_const(optional)`**:  指示`x'是否为常数的标志。如果它被设置为 "真"，"x "将被加上，就像所有各方共享的输入片的总和一样， 默认值是 "假"。
   - **`rh_is_const(optional)`**:  指示`y'是否为常数的标志。如果它被设置为 "真"，"y "将被添加到所有各方共享的输入棋子的总和中， 其默认值是 "假"。

  **返回：**

  一个 "张量"，具有与`x`相同的类型。

  
  *注意:*
  > 分母不能太大（最好小于10000），否则会溢出或影响精度。

  > 通常，输出的精度可以接近1e-4。

  > 这个SecureOp与 "SecureRealDiv "和 "SecureTrueDiv "相同，但事实上，reciprocaldiv算法比 "SecureTrueDiv "快5倍。


#### `SecureEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行等价比较的结果`(x == y)` 。结果对应的真实值为0.0时表示不相等，1.0则表示相等。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意:* 

  > 这个SecureOp支持Tensorflow中的广播机制。

  > 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**


#### `SecureGreater(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行“大于”比较的结果`(x > y)` 。结果对应的真实值为$0.0$ 时表示假，$1.0$则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意:* 

  > 这个SecureOp支持Tensorflow中的广播机制。

  > 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**

     

#### `SecureGreaterEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行“大于等于”比较的结果`(x >= y)` 。结果对应的真实值为$0.0$ 时表示假，$1.0$则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意:* 

  > 这个SecureOp支持Tensorflow中的广播机制。

  > 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**

     

#### `SecureLess(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行“小于”比较的结果`(x < y)` 。结果对应的真实值为$0.0$ 时表示假，$1.0$则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意:* 

  > 这个SecureOp支持Tensorflow中的广播机制。

  > 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**


#### `SecureLessEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行“小于等于”比较的结果`(x <= y)` 。结果对应的真实值为$0.0$ 时表示假，$1.0$则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意:* 

  > 这个SecureOp支持Tensorflow中的广播机制。

  > 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**


#### `SecureLogicalAnd(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行“逻辑与”比较的结果`(x & y)` 。结果对应的真实值为 `0.0`时表示假，`1.0`则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 `None`。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值, 默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意:* 

  > 这个SecureOp支持Tensorflow中的广播机制。

  > 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**


#### `SecureLogicalOr(x, y, name=None, lh_is_const=False, rh_is_const=False)`
  返回逐元素进行“逻辑或”比较的结果`(x | y)` 。结果对应的真实值为 `0.0`时表示假，`1.0`则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 `None`。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值, 默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意:* 

  > 这个SecureOp支持Tensorflow中的广播机制。

  > 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**


#### `SecureLogicalXor(x, y, name=None, lh_is_const=False, rh_is_const=False)`
  返回逐元素进行“逻辑异或”比较的结果`(x ^ y)` 。结果对应的真实值为 `0.0`时表示假，`1.0`则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 `None`。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值, 默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意:* 

  > 这个SecureOp支持Tensorflow中的广播机制。

  > 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**


#### `SecureLogicalNot(x, name=None)`

  返回逐元素进行“逻辑非”比较的结果`!x` 。结果对应的真实值为`0.0`时表示假，`1.0`则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 `None`。

  **返回值：**

  一个`Tensor`，类型与`x`相同。

  *注意:* 
  > 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**
  

#### `SecureMatMul(a, b, transpose_a=False, transpose_b=False, name=None)`

  执行矩阵乘法运算得到a*b。

  **参数:**

  - **`a`**: TensorFlow中的 `Tensor`，其值处于共享状态。

  - **`b`**: TensorFlow中的 `Tensor`，其值处于共享状态。且必须要和`a`的维度相兼容。

  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  - **`transpose_a(可选)`**: 如果值为`True`, `a` 在执行矩阵乘法之前先执行转置操作。默认值为 `Flase`.

  - **`transpose_b(可选)`**: 如果值为`True`, `b` 在执行矩阵乘法之前先执行转置操作。默认值为 `Flase`.

  **返回值:**

  ​	一个`Tensor`，类型与`x`相同。

  *NOTE:*  如同其他二元运算 `SecureOps`一样, **当前版本仅支持最多2维的`Tensor`参数**。



#### `SecurePow(x, y, name=None, lh_is_const=False, rh_is_const=True)`

  执行指数运算得到 $x^ y$。当前版本仅支持常整数指数。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的 `Tensor`。**在当前版本中，其中的每一个值都需要是常量整数，且各方的此值输入都相同。** 此外, `y` 和 `x`的`shape`要一致.
  - **`lh_is_const(可选)`**: **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :**`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `True`。 **当前版本，仅支持设为`True`。 在后续版本中我们将支持此值可变** 
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**

  ​	一个`Tensor`，类型与`x`相同。




#### `SecureExp(x, name=None)`

  计算`x`元素的e^x。

  **参数：**

- **`x`**: TensorFlow中的一个`Tensor`，其值处于共享状态。
- **`name(可选)`**: 操作的名称，其默认值为无。

  **返回：**

  一个 "张量"，具有与`x`相同的类型。

*注意*: 
>  该操作的当前精度为0.1 ~ 0.01，我们将在下一版本中对其进行优化。


#### `SecureSqrt(x, name=None)`

  计算`x`元素的平方根x^0.5。

  **参数：**

- **`x`**: TensorFlow中的一个`Tensor`，其值处于共享状态。
- **`name（可选）`**: 操作的名称，其默认值为无。

  **返回：**

  一个 "张量"，具有与`x`相同的类型。


#### `SecureRsqrt(x, name=None)`

  使用Newton Raphson方法计算`x`元素的1/(x^(0.5))。

  **参数：**

- **`x`**: TensorFlow中的一个`Tensor`，其值处于共享状态。
- **`name（可选）`**: 操作的名称，其默认值为无。

  **返回：**

  一个 "张量"，具有与`x`相同的类型。


#### `SecureLog(x, name=None)`

  逐元素计算自然对数$ln(x)$。本接口是为常见的 $x$ 处于 $[0.0001, 10]$时进行定制的。$x$不在此定义域内时，请使用`SecureHLog`。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**
  
  一个`Tensor`，类型与`x`相同。

  *注意*: 
  > 本算子的内部实现为了提高计算效率进行了优化，仅对$x \in [0.0001, 10]$有好的近似效果，在机器学习常见下，大部分场景下$x$都是在这个范围内的。当前版本我们具体采用的拟合多项式是：
  > $$ln(x)=\begin{cases} 85873.96716*x^3 -8360.491679*x^2 + 284.0022382*x -6.805568387& x \in (0.0001, 0.05) \\ 3.404663323 * x^3 -8.668159044*x^2 + 8.253302766*x -3.0312942 & x \in [0.05, 1.2) \\ -0.022636005*x^2+0.463403306*x-0.147409486 & x \in [1.2, 10.0) \end{cases}$$

  > **如果你需要使用更加通用和精度更好的自然对数计算，请参考 `SecureHLog`.**

  

#### `SecureLog1p(x, name=None)`

  逐元素计算$ln(x+1)$ 。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**
  
  一个`Tensor`，类型与`x`相同。

  *注意*:  
  > 此算子内部是基于`SecureLog`实现的，所以实现细节请参考`SecureLog`。

  

#### `SecureHLog(x, name=None)`

  逐元素计算$ln(x+1)$ 。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**
  
  一个`Tensor`，类型与`x`相同。

  

  *注意*:  本算子主要是用于在定义域内任意参数值的计算。其内部的实现相比`SecureLog`要复杂，所以所需要的计算时间和通讯量也更高一些，在具体使用中，请尽量优先使用`SecureLog`。

  

#### `SecureSigmoid(x, name=None)`

  逐元素计算`sigmoid`函数，即$\frac{1}{1+e^{-x}}$。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**

  一个`Tensor`，类型与`x`相同。

  *注意:*  
  > 此算子内部采用线性多项式拟合以提升性能。当前版本中具体使用的分段多项式如下: 
  > $$sigmoid(x)=\begin{cases} 0 & x \in (-\infty,-4] \\ 0.0484792 * x + 0.1998976 & x \in [-4, -2) \\ 0.1928931 * x + 0.4761351 & x \in [-2, 0) \\ 0.1928931 * x + 0.5238649 & x \in [0, 2) \\ 0.0484792 * x + 0.8001024 & x \in [2,4) \\ 1 & x \in [4, \infty) \end{cases}$$ .


#### `SecureRelu(x, name=None)`

  计算深度学习中常见的 `ReLU`函数,本质等价于`max(x, 0)` 。  

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**
  
  一个`Tensor`，类型与`x`相同。


#### `SecureReluPrime(x, name=None)`

  计算深度学习中常见的 `ReLU`函数的导数,本质等价与非负数返回1, 负数返回0。  

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**

  一个`Tensor`，类型与`x`相同。

  

#### `SecureAbs(x, name=None)`

  逐元素计算绝对值函数$|x|$。计算深度学习中常见的 `ReLU`函数,本质等价于$max(x, 0)$ 。  

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**

  一个`Tensor`，类型与`x`相同。



#### `SecureAbsPrime(x, name=None)`

  逐元素计算绝对值函数$|x|$的导数，即等价于正数返回1，否则返回-1。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**

  一个`Tensor`，类型与`x`相同。


#### `SecureMax(input_tensor, axis=None, name=None)`

  计算在指定的轴上的最大元素。 将在用户指定的`axis`上对`input_tensor`进行降维，如果`axis`参数为`None`,则进行完全的降维，返回单个元素。

  **参数:**

  - **`input_tensor`**: 待进行降维的`Tensor`. 

  - **`axis(可选)`**: 指定待降维的维度，如果为`None`(默认值)，则在所有维度上进行降维。 **当前版本中支持设为0, 1 或 None.**

  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。


  **返回值:**

  降维后的取大值`Tensor`。

  *注意：*
  > 从v1.0.0或更新的版本开始，Rosetta最多支持5维度的输入`Tensor`，而对于v0.2.0或更早的版本，只支持2维度的输入`Tensor`。


#### `SecureMin(input_tensor, axis=None, name=None)`

  计算在指定的轴上的最小元素。 将在用户指定的`axis`上对`input_tensor`进行降维，如果`axis`参数为`None`,则进行完全的降维，返回单个元素。

  **参数:**

  - **`input_tensor`**: 待进行降维的`Tensor`. 

  - **`axis(可选)`**: 指定待降维的维度，如果为`None`(默认值)，则在所有维度上进行降维。 **当前版本中支持设为0, 1 或 None.**

  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。


  **返回值:**

  降维后的取小值`Tensor`。
  
  *注意：*
  > 从v1.0.0或更新的版本开始，Rosetta最多支持5维度的输入`Tensor`，而对于v0.2.0或更早的版本，只支持2维度的输入`Tensor`。


#### `SecureMean(input_tensor, axis=None, name=None)`

  计算在指定的轴上的平均值。将在用户指定的`axis`上对`input_tensor`进行降维。如果`axis`参数为`None`,则进行完全的降维，返回单个元素。

  **参数:**

  - **`input_tensor`**: 待进行降维的`Tensor`. 

  - **`axis(可选)`**: 指定待降维的维度，如果为`None`(默认值)，则在所有维度上进行降维。 **当前版本中支持设为0, 1 或 None.**

  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

    
  **返回值:**

  降维后的取均值`Tensor`。

  *注意：*
  > 从v1.0.0或更新的版本开始，Rosetta最多支持5维度的输入`Tensor`，而对于v0.2.0或更早的版本，只支持2维度的输入`Tensor`。


#### `SecureReveal(a, reveal_party=None)`

  这个辅助的`SecureOp`可以解密`Tensor` `a`中的明文值。**由于这个接口的输出可能是明文，在生产环境中使用这个接口时要谨慎，并在各方之间达成共识**。 

  **参数:**

  - **`a`**: TensorFlow中的一个`Tensor`，其值处于共享状态。
  - **`reveal_party(optional)`**: 配置将获得明文输出的结果节点，建议参数值为结果节点列表。**默认情况下，它被设置为无，意味着所有的结果节点都将获得明文输出值**。
  

  **返回：**
  一个 "张量"，其中的值是配置的明文。

### I/O SecureOps

#### `SecureSaveV2(prefix, tensor_names, shape_and_slices, tensors, name=None)`

  用于将`Tensor`变量按照TensorFlow中V2格式规范保存到文件中的底层算子。对应于原生TensorFlow中的`save_v2`函数,  这个函数一般不直接被调用，而是借助于更上层的 `tf.train.Saver()` 接口进行调用。此处`SecureSaveV2`和`save_v2`具有一致的接口原型。  **不同于 原生`Saver`的是, 这个 `SecureOp`可以根据用户的配置将`tensor_names`所代表的变量`Tensor`恢复为明文后再保存到指定的用户方文件中。**在Rosetta框架下, 默认情况下，用户也并不需要直接使用这一接口，基于我们的`静态优化编P`技术 (请参看 [术语表](GLOSSARY_CN.md)中的定义）可以自动的完成`Saver`上层接口的替换处理。

  该接口的行为可以通过对我们的配置文件`CONFIG.json`中`SAVER_MODE` 关键字进行配置而不需要修改代码进行配置。这个值可以被设为$0~7$， 内在的会被解析为 $[P2\ P1 \ P0]$ 的3-bit的FLAG数组。即这个值的行为如下：

  - `0`: 所有方仍保存密文，不保存明文。 **这也是默认行为**；
  - `1`: 恢复明文后仅保存到 `P0`方；
  - `2`: 恢复明文后仅保存到 `P1`方；
  - `3`: 恢复明文后仅保存到 `P0`方和` P1`方；
  - `4`: 恢复明文后仅保存到 `P2`方；
  - `5`: 恢复明文后仅保存到 `P0`方和  `P2`方；
  - `6`: 恢复明文后仅保存到 `P1`方和  `P2`方；
  - `7`: 恢复明文后保存到 `P0`方、  `P1`方和 `P2`方. 

  **参数:**

  - **`prefix`**:  `string`类型的 `Tensor` 。只应该有一个元素，用于指定保存的V2 格式检查点(checkpoint)文件的前缀字符串。
  - **`tensor_name`**: `string`类型的 `Tensor` 。有${N}$个元素，用于指定具体需要保存的`Tensor`的自定义名称。 
  - **`shape_and_slices`**:`string`类型的 `Tensor` 。有${N}$个元素，用于指定具体需要保存的`Tensor`的具体切片配置。 
  - **`tensors`**: `Tensor` 对象列表，指定待保存的 $N$ 个`Tensor`。
  - **`name(可选)`**: 指定的该操作的名称，默认为`None`。

  **返回值:**

  ​	逻辑图上创建该IO逻辑的算子.

  *注意*: 每一个参与的计算法都必须要有同样的配置值，系统才可以正确的进行对应的操作。 **因为输出的文件中可以配置为保存明文，这一配置值很重要。所以使用此接口时务必小心，使用此接口前，在参与计算的多方之间对此要达成一致.**

  #### `PrivateInput(x, data_owner, name=None)`
  
  定义一个 "私有" 输入, 这个 "私有" 的输入，由指定 `data_owner` 端拥有并输入到计算图中。

  **参数:**
  - **`x`**: 数据拥有方持有的数据，支持的数据类型： `int32`，`int64`，`float`， `double`， `string`。
  - **`data_owner`**: 指定私有数据拥有方，data_owner = 0 代表 `P0` 方拥有私有数据，data_owner = 1 代表 `P1` 方拥有私有数据，data_owner = 2 代表 `P2` 方拥有私有数据。
  - **`name(可选)`**: 指定的该操作的名称，默认为`None`。

  **返回值:**
    一个`Tensor`，类型为string、形状与`x`相同。

  *注意*: 
  返回值的负载数据为加密或共享值。
