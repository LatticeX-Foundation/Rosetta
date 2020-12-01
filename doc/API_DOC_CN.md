# Rosetta 算子API文档

- [Rosetta 算子API文档](#rosetta-算子api文档)
  - [概述](#概述)
    - [术语和定义](#术语和定义)
    - [通用的说明](#通用的说明)
  - [SecureOps API](#secureops-api)
    - [计算类SecureOps](#计算类secureops)
      - [`SecureAdd(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#secureaddx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureSub(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securesubx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureMul(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securemulx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureFloorDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securefloordivx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securedivx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureDivide(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securedividex-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureTruediv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securetruedivx-y-namenone-lh_is_constfalse-rh_is_constfalse)
      - [`SecureRealDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#securerealdivx-y-namenone-lh_is_constfalse-rh_is_constfalse)
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
      - [`SecureLog(x, name=None)`](#securelogx-namenone)
      - [`SecureLog1p(x, name=None)`](#securelog1px-namenone)
      - [`SecureHLog(x, name=None)`](#securehlogx-namenone)
      - [`SecureSigmoid(x, name=None)`](#securesigmoidx-namenone)
      - [`SecureRelu(x, name=None)`](#securerelux-namenone)
      - [`SecureReluPrime(x, name=None)`](#securereluprimex-namenone)
      - [`SecureAbs(x, name=None)`](#secureabsx-namenone)
      - [`SecureAbsPrime(x, name=None)`](#secureabsprimex-namenone)
      - [`SecureMax(input_tensor, axis=None, name=None)`](#securemaxinput_tensor-axisnone-namenone)
      - [`SecureMean(input_tensor, axis=None, name=None)`](#securemeaninput_tensor-axisnone-namenone)
      - [`SecureReveal(a, reveal_party=-1)`](#securereveala-reveal_party-1)
    - [I/O SecureOps](#io-secureops)
      - [`SecureSaveV2(prefix, tensor_names, shape_and_slices, tensors, name=None)`](#securesavev2prefix-tensor_names-shape_and_slices-tensors-namenone)
      - [`PrivateInput(x, data_owner, name=None)`](#privateinputx-data_owner-namenone)


## 概述

> 注意，此文档内容可能会有所滞后，最新的接口文档请参考[用户接口文档](API_DOC.md)。


在Rosetta框架中，用户在`import latticex.rosetta` 后，不需要修改现有TensorFlow程序中的代码便可以直接的在各自拥有的隐私数据集上进行基于MPC的多方协同的人工智能模型训练或推断（可以参考我们的[tutorial文档](TUTORIALS_CN.md)）。支持这些上层便利性的主要组件是我们在Rosetta内部基于TensorFlow的自定义算子库扩展机制实现了支持Secure功能的新算子(Operation)。为与原生TensorFlow中的API算子(下文直接简称其为`Ops`)相区分，我们称这些自定义的算子为`SecureOps` 。

这里我们介绍`Rosetta v0.2.0`版本中所支持的各个`SecureOps`的接口使用方法。大部分的`SecureOps` 的原型都与TensorFlow中的`Ops`一致，只有在极少数的情况下，我们对对应的`Ops`进行了更多MPC相关功能的扩展（比如`SaveV2`算子等）。

如果你需要自己基于Rosetta的底层API构建自己特定的隐私保护模型，或者对我们的底层实现感兴趣，你都可以参考本文档。此外，在源代码中的一些单元测试实例代码也有助于理解各`SecureOps`的使用。

### 术语和定义

在本文档中我们尽量使用易懂的方式介绍各个接口。为了表述的简介，会涉及‘秘密分享’等少量的术语，请参考我们的[术语表](GLOSSARY_CN.md)。

### 通用的说明

1. 与原生TensorFlow中的 `Ops `的输入和输出 `Tensor `不同，`SecureOps `的参数和返回值被认为是一个**共享的值**，处于秘密共享状态，除非显式声明一个输入值是一个常数（参见下面的具体相关某一`MpsOps `的接口说明）。你一般不会直接使用这些 "乱码 "值。

2. 在 `SecureOps`的输入和输出`tensor`的数据类型 (`dtype`)上，**Rosetta的Python前台将统一转换为当前版本中的`tf.string'。**

3. 对于诸如 `SecureAdd`等二元运算符，当前的 `Rosetta v0.2.0 `不支持维度大于2的 `Tensor`参数，而对于诸如 `SecureRelu `等单元运算符，其 `Tensor Shape`不受限制。

   


## SecureOps API

### 计算类SecureOps

#### `SecureAdd(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  逐个元素的执行加法操作，得到 `x+y `。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个 `Tensor`。类型与`x`相同。

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

  一个 `Tensor`。类型与`x`相同。

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

  一个 `Tensor`。类型与`x`相同。

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

  一个 `Tensor`。类型与`x`相同。

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

  一个 `Tensor`。类型与`x`相同。

  *注意*:
  - 这个SecureOp支持Tensorflow中的广播机制。
  - 由于在Secure下实现此算子本身的复杂度较高，此算子的计算时间开销和通讯数据量开销都相对更大。在具体的上层模型构建中，应该尽可能的避免直接使用此算子。

  

#### `SecureTruediv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  等价于 `SecureDivide`. 请参考 `SecureDivide`的接口说明.

  

#### `SecureRealDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  等价于 `SecureDivide`. 请参考 `SecureDivide`的接口说明.

  

#### `SecureEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回 逐元素进行等价比较的结果`(x == y)` 。结果对应的真实值为$0.0$ 时表示不相等，$1.0$则表示相等。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个 `Tensor`。类型与`x`相同。

  *注意:* 

   - 这个SecureOp支持Tensorflow中的广播机制。

   - 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**

     


#### `SecureGreater(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行“大于”比较的结果`(x > y)` 。结果对应的真实值为$0.0$ 时表示假，$1.0$则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个 `Tensor`。类型与`x`相同。

  *注意:* 

   * 这个SecureOp支持Tensorflow中的广播机制。

   * 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**

     

#### `SecureGreaterEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行“大于等于”比较的结果`(x >= y)` 。结果对应的真实值为$0.0$ 时表示假，$1.0$则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个 `Tensor`。类型与`x`相同。

  *注意:* 

   * 这个SecureOp支持Tensorflow中的广播机制。

   * 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**

     

#### `SecureLess(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行“小于”比较的结果`(x < y)` 。结果对应的真实值为$0.0$ 时表示假，$1.0$则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个 `Tensor`。类型与`x`相同。

  *注意:* 

   * 这个SecureOp支持Tensorflow中的广播机制。
   * 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**



#### `SecureLessEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行“小于等于”比较的结果`(x <= y)` 。结果对应的真实值为$0.0$ 时表示假，$1.0$则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值。默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个 `Tensor`。类型与`x`相同。

  *注意:* 

   * 这个SecureOp支持Tensorflow中的广播机制。
   * 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**

#### `SecureLogicalAnd(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  返回逐元素进行“逻辑与”比较的结果`(x & y)` 。结果对应的真实值为 $0.0$ 时表示假，$1.0$ 则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 `None`。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值, 默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个 `Tensor`。类型与`x`相同。

  *注意:* 

   * 这个SecureOp支持Tensorflow中的广播机制。
   * 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**

#### `SecureLogicalOr(x, y, name=None, lh_is_const=False, rh_is_const=False)`
  返回逐元素进行“逻辑或”比较的结果`(x | y)` 。结果对应的真实值为 $0.0$ 时表示假，$1.0$ 则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 `None`。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值, 默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个 `Tensor`。类型与`x`相同。

  *注意:* 

   * 这个SecureOp支持Tensorflow中的广播机制。
   * 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**

#### `SecureLogicalXor(x, y, name=None, lh_is_const=False, rh_is_const=False)`
  返回逐元素进行“逻辑异或”比较的结果`(x ^ y)` 。结果对应的真实值为 $0.0$ 时表示假，$1.0$ 则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`y`**: TensorFlow中的`Tensor`，其值处于共享状态。且必须具有与`x`相同的`shape`。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 `None`。
  - **`lh_is_const(可选)`**：标识`x`是否为常数。如果它被设置为`True`，那么`x`将被视为所有各方共享的输入值的总和，即真实值, 默认值为 `False`。
  - **`rh_is_const(可选)`** :标识`y`是否为常数。如果设置为`True`，那么`y `将被视为各方共享的输入值的综合，即真实值，默认值为 `False`。

  **返回值：**

  一个 `Tensor`。类型与`x`相同。

  *注意:* 

   * 这个SecureOp支持Tensorflow中的广播机制。
   * 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**

#### `SecureLogicalNot(x, name=None)`
  返回逐元素进行“逻辑非”比较的结果`!x` 。结果对应的真实值为 $0.0$ 时表示假，$1.0$ 则表示真。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 `None`。

  **返回值：**

  一个 `Tensor`。类型与`x`相同。

  *注意:* 
   * 由于和其他计算类算子一样，结果是密文共享状态的值，**所以不可以直接的在后续程序中直接的对此算子的结果进行判断类型语句的处理！**
  

#### `SecureMatMul(a, b, transpose_a=False, transpose_b=False, name=None)`

  执行矩阵乘法运算得到$a*b$。

  **参数:**

  - **`a`**: TensorFlow中的 `Tensor`，其值处于共享状态。

  - **`b`**: TensorFlow中的 `Tensor`，其值处于共享状态。且必须要和`a`的维度相兼容。

  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  - **`transpose_a(可选)`**: 如果值为`True`, `a` 在执行矩阵乘法之前先执行转置操作。默认值为 `Flase`.

  - **`transpose_b(可选)`**: 如果值为`True`, `b` 在执行矩阵乘法之前先执行转置操作。默认值为 `Flase`.

  **返回值:**

  ​	一个 `Tensor`。类型与`x`相同。

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

  ​	一个 `Tensor`。类型与`x`相同。

  


#### `SecureLog(x, name=None)`

  逐元素计算自然对数$ln(x)$。任意维度的 `x` 都是支持的。本接口是为常见的 $x$ 处于 $[0.0001, 10]$时进行定制的。$x$不在此定义域内时，请使用`SecureHLog`。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**
  
  一个 `Tensor`。类型与`x`相同。

  *注意*: 本算子的内部实现为了提高计算效率进行了优化，仅对$x \in [0.0001, 10]$有好的近似效果，在机器学习常见下，大部分场景下$x$都是在这个范围内的。当前版本我们具体采用的拟合多项式是：

  $$ln(x)=\begin{cases} 85873.96716*x^3 -8360.491679*x^2 + 284.0022382*x -6.805568387& x \in (0.0001, 0.05) \\ 3.404663323 * x^3 -8.668159044*x^2 + 8.253302766*x -3.0312942 & x \in [0.05, 1.2) \\ -0.022636005*x^2+0.463403306*x-0.147409486 & x \in [1.2, 10.0) \end{cases}$$

   **如果你需要使用更加通用和精度更好的自然对数计算，请参考 `SecureHLog`.**

  

#### `SecureLog1p(x, name=None)`

  逐元素计算$ln(x+1)$ 。任意维度的 `x` 都是支持的。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**
  
  一个 `Tensor`。类型与`x`相同。

  *注意*:  此算子内部是基于`SecureLog`实现的，所以实现细节请参考`SecureLog`。

  

#### `SecureHLog(x, name=None)`

  逐元素计算$ln(x+1)$ 。任意维度的 `x` 都是支持的。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**
  
  一个 `Tensor`。类型与`x`相同。

  

  *注意*:  本算子主要是用于在定义域内任意参数值的计算。其内部的实现相比`SecureLog`要复杂，所以所需要的计算时间和通讯量也更高一些，在具体使用中，请尽量优先使用`SecureLog`。

  

#### `SecureSigmoid(x, name=None)`

  逐元素计算`sigmoid`函数，即$\frac{1}{1+e^{-x}}$。任意维度的 `x` 都是支持的。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**

  一个 `Tensor`。类型与`x`相同。

  *注意:*  此算子内部采用线性多项式拟合以提升性能。当前版本中具体使用的分段多项式如下: 

  $$sigmoid(x)=\begin{cases} 0 & x \in (-\infty,-4] \\ 0.0484792 * x + 0.1998976 & x \in [-4, -2) \\ 0.1928931 * x + 0.4761351 & x \in [-2, 0) \\ 0.1928931 * x + 0.5238649 & x \in [0, 2) \\ 0.0484792 * x + 0.8001024 & x \in [2,4) \\ 1 & x \in [4, \infty) \end{cases}$$


  .

  

#### `SecureRelu(x, name=None)`

  计算深度学习中常见的 `ReLU`函数,本质等价于$max(x, 0)$ 。  任意维度的 `x` 都是支持的。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**
  
  一个 `Tensor`。类型与`x`相同。

  

#### `SecureReluPrime(x, name=None)`

  计算深度学习中常见的 `ReLU`函数的导数,本质等价与非负数返回1, 负数返回0。  任意维度的 `x` 都是支持的。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**

  一个 `Tensor`。类型与`x`相同。

  

#### `SecureAbs(x, name=None)`

  逐元素计算绝对值函数$|x|$。计算深度学习中常见的 `ReLU`函数,本质等价于$max(x, 0)$ 。  任意维度的 `x` 都是支持的。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**

  一个 `Tensor`。类型与`x`相同。



#### `SecureAbsPrime(x, name=None)`

  逐元素计算绝对值函数$|x|$的导数，即等价于正数返回1，否则返回-1。任意维度的 `x` 都是支持的。

  **参数:**

  - **`x`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

  **返回值:**

  一个 `Tensor`。类型与`x`相同。

  

#### `SecureMax(input_tensor, axis=None, name=None)`

  计算在指定的轴上的最大元素。 **当前版本中，仅支持最多2维的`Tensor`参数**。将在用户指定的`axis`上对`input_tensor`进行降维。如果`axis`参数为`None`,则进行完全的降维，返回单个元素。

  **参数:**

  - **`input_tensor`**: 待进行降维的`Tensor`. 

  - **`axis(可选)`**: 指定待降维的维度。如果为`None`(默认值)，则在所有维度上进行降维。 **当前版本中支持设为0, 1 或 None.**

  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

    

  **返回值:**

  降维后的新`Tensor`。

  

#### `SecureMean(input_tensor, axis=None, name=None)`

  计算在指定的轴上的平均值。 **当前版本中，仅支持最多2维的`Tensor`参数**。将在用户指定的`axis`上对`input_tensor`进行降维。如果`axis`参数为`None`,则进行完全的降维，返回单个元素。

  **参数:**

  - **`input_tensor`**: 待进行降维的`Tensor`. 

  - **`axis(可选)`**: 指定待降维的维度。如果为`None`(默认值)，则在所有维度上进行降维。 **当前版本中支持设为0, 1 或 None.**

  - **`name(可选)`**: 指定的该操作的名称，默认值为 None。

    
  **返回值:**

  降维后的新`Tensor`。

  

#### `SecureReveal(a, reveal_party=-1)`

  该辅助性质的`SecureOp`主要用于恢复对应的真实值，主要用于调试和测试验证中。 **由于这个接口返回的是`a`对应的真实值，所以务必小心使用此接口，尤其是生产环境下，使用此接口前，在参与计算的多方之间对此要达成一致。**  

  **参数:**

  - **`a`**: TensorFlow中的 `Tensor`，其值处于共享状态。
  - **`reveal_party(optional)`**: 配置具体那一个参与方的本地返回值中可以返回明文真实值结果。这个值需要在0~7之间，并且会在内部被解析为一个3bit的flag bitmap$[P2\ P1 \ P0]$。 例如，设为3时会使得仅`P0` and` P1` 的本地返回值是明文。**默认情况下，此参数被置为7, 表示 `P0`, `P1`和`P2`方的本地返回都可以得到对应的明文值。**

  **返回值:**

  一个 `Tensor`。根据配置参数，可以是明文值。

### I/O SecureOps

#### `SecureSaveV2(prefix, tensor_names, shape_and_slices, tensors, name=None)`

  用于将`Tensor`变量按照TensorFlow中V2格式规范保存到文件中的底层算子。对应于原生TensorFlow中的`save_v2`函数,  这个函数一般不直接被调用，而是借助于更上层的 `tf.train.Saver()` 接口进行调用。此处`SecureSaveV2`和`save_v2`具有一致的接口原型。  **不同于 原生`Saver`的是, 这个 `SecureOp`可以根据用户的配置将`tensor_names`所代表的变量`Tensor`恢复为明文后再保存到指定的用户方文件中。**在Rosetta框架下, 默认情况下，用户也并不需要直接使用这一接口，基于我们的`静态优化编P`技术 (请参看 [术语表](GLOSSARY_CN.md)中的定义）可以自动的完成`Saver`上层接口的替换处理。

  该接口的行为可以通过对我们的配置文件`CONFIG.json`中`SAVER_MODE` 关键字进行配置而不需要修改代码进行配置。这个值可以被设为$0~7$， 内在的会被解析为 $[P2\ P1 \ P0]$ 的3-bit的FLAG数组。即这个值的行为如下：

  - $0$: 所有方仍保存密文，不保存明文。 **这也是默认行为**；
  - $1$: 恢复明文后仅保存到 `P0`方；
  - $2$: 恢复明文后仅保存到 `P1`方；
  - $3$: 恢复明文后仅保存到 `P0`方和` P1`方；
  - $4$: 恢复明文后仅保存到 `P2`方；
  - $5$: 恢复明文后仅保存到 `P0`方和  `P2`方；
  - $6$: 恢复明文后仅保存到 `P1`方和  `P2`方；
  - $7$: 恢复明文后保存到 `P0`方、  `P1`方和 `P2`方. 

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
  - **`data_owner`**: 指定私有数据拥有方，data_owner = 0 代表 P0 方拥有私有数据，data_owner = 1 代表 P1 方拥有私有数据，data_owner = 2 代表 P2 方拥有私有数据。
  - **`name(可选)`**: 指定的该操作的名称，默认为`None`。

  **返回值:**
    一个 `Tensor`。类型和形状与`x`相同。

  *注意*: 返回值是一个跟 x 形状和类型一样的共享值。
