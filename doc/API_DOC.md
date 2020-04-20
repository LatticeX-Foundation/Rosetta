# Rosetta Operations API

- [Rosetta Operations API](#rosetta-operations-api)
  - [Overview](#overview)
    - [Terms and definition](#terms-and-definition)
    - [Common notes](#common-notes)
  - [MpcOps API](#mpcops-api)
    - [Computational MpcOps](#computational-mpcops)
      - [`MpcAdd(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpcaddx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcSub(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpcsubx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcMul(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpcmulx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcFloorDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpcfloordivx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpcdivx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcDivide(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpcdividex-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcTruediv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpctruedivx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcRealDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpcrealdivx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpcequalx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcGreater(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpcgreaterx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcGreaterEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpcgreaterequalx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcLess(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpclessx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcLessEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`](#mpclessequalx-y-namenone-lhisconstfalse-rhisconstfalse)
      - [`MpcMatMul(a, b, transpose_a=False, transpose_b=False, name=None)`](#mpcmatmula-b-transposeafalse-transposebfalse-namenone)
      - [`MpcPow(x, y, name=None, lh_is_const=False, rh_is_const=True)`](#mpcpowx-y-namenone-lhisconstfalse-rhisconsttrue)
      - [`MpcLog(x, name=None)`](#mpclogx-namenone)
      - [`MpcLog1p(x, name=None)`](#mpclog1px-namenone)
      - [`MpcHLog(x, name=None)`](#mpchlogx-namenone)
      - [`MpcSigmoid(x, name=None)`](#mpcsigmoidx-namenone)
      - [`MpcRelu(x, name=None)`](#mpcrelux-namenone)
      - [`MpcReluPrime(x, name=None)`](#mpcreluprimex-namenone)
      - [`MpcAbs(x, name=None)`](#mpcabsx-namenone)
      - [`MpcAbsPrime(x, name=None)`](#mpcabsprimex-namenone)
      - [`MpcMax(input_tensor, axis=None, name=None)`](#mpcmaxinputtensor-axisnone-namenone)
      - [`MpcMean(input_tensor, axis=None, name=None)`](#mpcmeaninputtensor-axisnone-namenone)
      - [`MpcReveal(a, reveal_party=-1)`](#mpcreveala-revealparty-1)
    - [I/O MpcOps](#io-mpcops)
      - [`MpcSaveV2(prefix, tensor_names, shape_and_slices, tensors, name=None)`](#mpcsavev2prefix-tensornames-shapeandslices-tensors-namenone)
## Overview

By using Rosetta framework, users can directly perform training or inference task on all of their respective dataset without leaking any privacy to others just after adding a single code `import latticex.rosetta`  at the header of your existing TensorFlow programs (see our [tutorial documentation](TUTORIALS.md) for more details). The main magic component that supports these upper-level conveniences is our implementation of a new suites of MPC-enabled `Operation` based on TensorFlow's flexiable extension mechanism for introducing new operation library. To distinguish it from the native TensorFlow API Operation (hereinafter referred to directly as `Ops`), we refer to these customized Operation as `MpcOps`.

Here we describe how to use the various `MpcOps` interfaces supported in the `Rosetta v0.1.0` version. Most of interface signature of these `MpcOps` is consistent with the corresponding `Ops`' in TensorFlow, and only in a few cases have we extended the native one with more MPC-related functionality (e.g., the `SaveV2` operation, etc.).

If you need to build your own specific privacy protection model based on Rosetta's underlying API, or are interested in our extending our `MpcOps` set, this is the right place you should start with. In addition, unit tests in the source code can also help you to understand the usage of the various `MpcOps`.

### Terms and definition

We will try to represent each `MpcOp` interface in an clear and easy-to-understand way as far as we can. Occasionally, we will use some cryptographic terms for concision, wich you can refer to [glossary document](GLOSSARY.md) for their definition if you are not sure.

### Common notes

1. Unlike the input and output `Tensor` of `Ops` in native TensorFlow, the parameters and return values of `MpcOps` are considered to be a **shared value** in a secret state, unless an explicit declaration is made that an input value is an explicit constant (see the related `MpsOps` interface declaration below). You may not use these 'garbled' values directly.

2. On data type (`dtype`) of  `MpcOps` input and output `tensor`, **Rosetta's Python frontend will uniformly convert them as `tf.float64' in the current version.**

3. For binary operators such as `MpcAdd`, the current `Rosetta v0.1.0` does not support the `Tensor` with dimension more than 2, while for unary operators such as `MpcRelu`, their tensor shape is not restricted.
  
  

## MpcOps API

### Computational MpcOps

#### `MpcAdd(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns `x + y` element-wise.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* broadcasting is supported for this MpcOp.
  
  


#### `MpcSub(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns `x - y` element-wise.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* broadcasting is supported for this MpcOp.
  

  
#### `MpcMul(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns `x * y` element-wise.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* broadcasting is supported for this MpcOp.

  


#### `MpcFloorDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`  

  Integral floor divison of `x` by `y` in shared status element-wise, rounding toward the most negative integer. For example,$6 / 4 = 1$ and $(-6) / 4 = -2$.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* broadcasting is supported for this MpcOp.
  
  


#### `MpcDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Alias for `MpcFloorDiv`. Please refer to `MpcFloorDiv`. we recommend you to use `MpcFloorDiv` as you can.

    

#### `MpcDivide(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Computes Python style division of `x` by `y` element-wise.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.
  
  *NOTE:* 
    
  - broadcasting is supported for this MpcOp.
  - due to its intrinsic algorithm complexity in MPC style to meet the security guarantee, **this MpcOp is comparatively much more time-consuming. So you may aviod to use this MpcOp as possible as you can.**
  - this MpcOp is just the same as `MpcRealDiv` and `MpcTrueDiv`.
  
  

#### `MpcTruediv(x, y, name=None, lh_is_const=False, rh_is_const=False)`
  
  Alias for `MpcDivide`. Please refer to `MpcDivide`.

  
  
#### `MpcRealDiv(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Alias for `MpcDivide`. Please refer to `MpcDivide`.

  

#### `MpcEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x == y)` element-wise, as $0.0$ for false and $1.0$ for true.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 
  
   * broadcasting is supported for this MpcOp.
  
   * The output values are still in the shared status, just like other `MpcOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**
  
     


#### `MpcGreater(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x > y)` element-wise, as $0.0$ for false and $1.0$ for true.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

   * broadcasting is supported for this MpcOp.
   * The output values are still in the shared status, just like other `MpcOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**



#### `MpcGreaterEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x >= y)` element-wise, as 0 for false and 1 for true.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

   * broadcasting is supported for this MpcOp.
   * The output values are still in the shared status, just like other `MpcOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause of your source code directly.**

  

#### `MpcLess(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x < y)` element-wise, as 0 for false and 1 for true.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

   * Broadcasting is supported for this MpcOp.
   * The output values are still in the shared status, just like other `MpcOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**



#### `MpcLessEqual(x, y, name=None, lh_is_const=False, rh_is_const=False)`

  Returns the truth value of `(x <= y)` element-wise, as 0 for false and 1 for true.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A `Tensor` in TensorFlow, whose values are in shared status. . Must have the same type as `x`.
  - **`name(optional)`**: A name for the operation, the default value of it is None.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `False`.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:* 

   * Broadcasting is supported for this MpcOp.
   * The output values are still in the shared status, just like other `MpcOps`. So in the current version, **you should not use the local output `Tensor` in any following predicate clause directly.**

  

#### `MpcMatMul(a, b, transpose_a=False, transpose_b=False, name=None)`

  Multiplies matrix `a` by matrix `b`, producing `a` * `b`.

  **Args:**

  - **`a`**: A  `Tensor` in TensorFlow, whose values are in shared status. 

  - **`b`**: A `Tensor` in TensorFlow, whose values are in shared status. Must be logically compatible with `a`.

  - **`name(optional)`**: A name for the operation (optional).

  - **`transpose_a(optional)`**: If `True`, `a` is transposed before multiplication. Its default value is `Flase`.

  - **`transpose_b(optional)`**: If `True`, `b` is transposed before multiplication.Its defaut value is `False`.

  **Returns:**

  ​	A `Tensor`. each value in it is the inner product of the corresponding vector in `a` and `b`.

  

  *NOTE:*  Just like other binary `MpcOps`, **in the current version, only at most 2-dimension input `Tensor` is supported.**



#### `MpcPow(x, y, name=None, lh_is_const=False, rh_is_const=True)`

  Returns $x^ y$ element-wise. 

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`y`**: A  `Tensor` in TensorFlow. **In the current version, every value must be a constant plaintext  integer, and all parties must have the same value.** Besides, `y` should have the same shape as `x`.
  - **`lh_is_const(optional)`**: flag indicating whether the `x` is a const number. If it is set as True, the `x` will be added just as the sum of all parties' shared input pieces. The default value is `False`.
  - **`rh_is_const(optional)`** :flag indicating whether the `y` is a const number. If it is set as True, the `y` will be added just as the sum of all parties' shared input pieces.The default value is `True`. **In the current version, only `True` is supported. In the future version, we will support it to be `False`** 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.
  
  


#### `MpcLog(x, name=None)`

  Computes natural logarithm of `x` element-wise. Any dimension of `x` is supported. This is optimized version for $x \in [0.0001, 10]$, so **DO NOT** use it for other $x$.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  

  *NOTE*: The inner implementation of this `MpcLog` is optimized for efficiency by polynomial interpolation. for  $x \in [0.0001, 10]$, which are often used in machine learning tasks. And its average absolute error  is less than  $0.01$. The detailed segmental polynomials we use is:

  $$ln(x)=\begin{cases} 85873.96716*x^3 -8360.491679*x^2 + 284.0022382*x -6.805568387& x \in (0.0001, 0.05) \\ 3.404663323 * x^3 -8.668159044*x^2 + 8.253302766*x -3.0312942 & x \in [0.05, 1.2) \\ -0.022636005*x^2+0.463403306*x-0.147409486 & x \in [1.2, 10.0) \end{cases}$$

   **If your want to use the general and high-precision mathmetical natural logarithm, you should use `MpcHLog`.**

  

#### `MpcLog1p(x, name=None)`

  Computes natural logarithm of `x+1` element-wise. Any dimension of `x` is supported.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  

  *NOTE*:  The inner implementation of this `MpcLog1p` is based on `MpcLog`, you may refer to `MpcLog` for more information.

  

#### `MpcHLog(x, name=None)`

  Computes natural logarithm of `x` element-wise. Any dimension of `x` is supported.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  

  *NOTE*:  This is for computing general and high-precision mathmetical natural logarithm for all $x$ in domain. **This `MpcOp` is much more time-consuming, so avoid to use this as you can.** 

  

#### `MpcSigmoid(x, name=None)`

  Computes the sigmoid function, which means  $\frac{1}{1+e^{-x}}$  element-wise. Any dimension of `x` is supported.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  *NOTE:*  The inner implementation of this `MpcSigmoid` is  optimized for efficiency by using polynomial interpolation. The segmental linear polynomials we use is as follows: 

  $$sigmoid(x)=\begin{cases} 0 & x \in (-\infty,-4] \\ 0.0484792 * x + 0.1998976 & x \in [-2, 0) \\ 0.1928931 * x + 0.4761351 & x \in [0,2) \\ 0.0484792 * x + 0.8001024 & x \in [2,4) \\ 1 & x \in [4, \infty) \end{cases}$$

  .

  

#### `MpcRelu(x, name=None)`

  Computes rectified linear functionality, which means $max(x, 0)$ element-wise.  Any dimension of `x` is supported.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  

#### `MpcReluPrime(x, name=None)`

  Computes derivation of rectified linear functionality, which means $1$ if $x >= 0$, otherwise $0$.  Any dimension of `x` is supported.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  

#### `MpcAbs(x, name=None)`

  Computes absolute value of each value in `x`.  Any dimension of `x` is supported.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.



#### `MpcAbsPrime(x, name=None)`

  Computes derivation of  absolute value of each value in `x`, which means $1$ if $x > 0$, otherwise $-1$. Any dimension of `x` is supported.

  **Args:**

  - **`x`**: A  `Tensor` in TensorFlow, whose values are in shared status. 
  - **`name(optional)`**: A name for the operation, the default value of it is None.

  **Returns:**

  ​	A `Tensor`. Has the same type as `x`.

  

#### `MpcMax(input_tensor, axis=None, name=None)`

  Computes the maximum of elements across dimensions of a tensor.  **in the current version, only 2-dimension input `Tensor` is supported.**

  Reduces `input_tensor` along the dimensions given in `axis`. 

  If `axis` is None, all dimensions are reduced, and a tensor with a single element is returned.

  **Args:**

  - **`input_tensor`**: The tensor to reduce. 

  - **`axis(optional)`**: The dimensions to reduce. If `None` (the default), reduces all dimensions. **In the current version it must be 0, 1 or None.**

  - **`name(optional)`**: A name for the operation (optional).

    

  **Returns:**

  The reduced tensor.

  

#### `MpcMean(input_tensor, axis=None, name=None)`

  Computes the mean of elements across dimensions of a tensor.  **in the current version, only 2-dimension input `Tensor` is supported.**

  Reduces `input_tensor` along the dimensions given in `axis`. 

  If `axis` is None, all dimensions are reduced, and a tensor with a single element is returned.

  **Args:**

  - **`input_tensor`**: The tensor to reduce. 
  - **`axis(optional)`**: The dimensions to reduce. If `None` (the default), reduces all dimensions. **In the current version it must be 0, 1 or None.**
  - **`name(optional)`**: A name for the operation (optional).

  **Returns:**

  The reduced tensor.

  

#### `MpcReveal(a, reveal_party=-1)`

  This auxiliary `MpcOp` can reveal the plaintext value of the `Tensor` `a`. **Since the output of this interface can be plaintext, be cautious and reach a consensus among all the parties when using this in production environment.**  

  **Args:**

  - **`a`**: A  `Tensor` in Tensorflow, whose values are in shared status. 
  - **`reveal_party(optional)`**: Configure which party can get the plaintext output, It can be set to: $0$, meaning that only party `P0` can get the plaintext output values; $1$, meaning that only party `P1` can get the  plaintext output values. **By default, it is set to `-1`, meaning that both `P0` and `P1` can get the plaintext putput values.**

  **Returns:**
  
  A `tensor`, of which values can be in plaintext as configured.

### I/O MpcOps

#### `MpcSaveV2(prefix, tensor_names, shape_and_slices, tensors, name=None)`

  This is the underlying Operation to save tensors in V2 checkpoint format. In native TensorFlow, its corresponding operation, `save_v2`,  which has the identical interface signature, is also wrapped by the `tf.train.Saver()` interface, rather than being called directly.  **Unlike the native `Saver`, this `MpcOp` can save the plaintext tensors in files only in specific parties by configuration.** In Rosetta, by default, you have no need to use this `MpcOp` neither, becasue our `Static Pass` functionality (see its definition in [glossary](GLOSSARY.md)) can help you perform the correct replacement intrinsically.

  The configurational option in `Rosetta v0.1.0` is the `SAVER_MODE` keyword in `CONFIG.json`.  Its  value os parsed as a 3-bit Flag for $[P2\ P1 \ P0]$, so it can be one of the following:

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


