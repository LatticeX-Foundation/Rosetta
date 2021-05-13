## Introduction

String customerized ops should  implements here, and other SNN or new protocol(future) MPC implementation ops should be in the same level directories.
The `strops` will reuse the `mpcops` code and copy file from `mpcops`, the file names of `strops` are prefixed with "str_".

-- tf
 |-- futureops (new protocol of MPC)
 |-- mpcops (snn)
 |-- strops


## How to use multiple crypto protocols

每个tf算子分开实现，不再使用模板复用算子形式，op Compute调用工作流如下如下：

1. tf Compute中处理tf相关的shape广播、校验、向量化输入（【这部分代码可以复用】
2. 获取msgkey，其他参数存储
3. 调用加密协议算子， 如下：

    map<string, std::any> attrs;//参数存储
    vector<string> a, b, c;//输入向量
    ProtocolInstance()->MpcProtocol()->GetOperators("msgkey")->add(a, b, c, attrs);
    <!-- 
    GetOperators {
      return new SNNOperators("msgkey");
    }

    add {
      SNN_Add(a, b, c);//Future_Add()
    } 
    -->