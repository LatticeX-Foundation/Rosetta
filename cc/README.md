


## Overview



## Project Structure

```
cc/
├── build/
├── certs/
├── cmake/
├── conf/
├── modules/
│   ├── common/
│   │   ├── include/
│   │   ├── src/
│   │   ├── tests/
│   │   └── CMakeLists.txt
│   ├── io/
│   │   ├── examples/
│   │   ├── include/
│   │   ├── src/
│   │   ├── tests/
│   │   └── CMakeLists.txt
│   ├── protocol/
│   │   ├── he/
│   │   ├── mpc/
│   │   │   ├── include/
│   │   │   ├── src/
│   │   │   │   ├── aby/
│   │   │   │   ├── snn/
│   │   │   │   └── spdz/
│   │   │   ├── tests/
│   │   │   └── CMakeLists.txt
│   │   ├── psi/
│   │   └── zk/
│   └── CMakeLists.txt
├── tf/
│   ├── misc/
│   ├── mpcops/
│   │   ├── grads_ops/
│   │   ├── ops/
│   │   └── tests/
│   └── pass/
├── third_party/
├── CMakeLists.txt
├── compile_and_test.sh
└── README.md
```

**Dependent chain:** (outer) ---> op.so ---> io.so ---> common.a

> **Notice:** set the ROSETTA_DPASS=OFF will disable the dynamic pass module 
