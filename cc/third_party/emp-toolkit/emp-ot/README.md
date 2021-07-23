# emp-ot 
![arm](https://github.com/emp-toolkit/emp-ot/workflows/arm/badge.svg)
![x86](https://github.com/emp-toolkit/emp-ot/workflows/x86/badge.svg)

<img src="https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/art/logo-full.jpg" width=300px/>

Protocols
=====
This repo contains state-of-the-art OT implementations. Include two base OTs, IKNP OT extension and Ferret OT extension. All hash functions used for OTs are implemented with [MiTCCR](https://github.com/emp-toolkit/emp-tool/blob/master/emp-tool/utils/mitccrh.h#L8) for optimal concrete efficiency.

Installation
=====

1. `wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py`
2. `python install.py -install -tool -ot`
    1. By default it will build for Release. `-DCMAKE_BUILD_TYPE=[Release|Debug]` option is also available.
    2. No sudo? Change [`CMAKE_INSTALL_PREFIX`](https://cmake.org/cmake/help/v2.8.8/cmake.html#variable%3aCMAKE_INSTALL_PREFIX).

Test
=====

Testing on localhost
-----

   `./run ./bin/[binary] logn`

with `[binary]=ot` for common OT functionalities, `[binary]=ferret` for ferret specific functionalities, `logn` as the log(number of OT). The script `run` will locally open two programs.
   
Testing on two
-----

1. Change the IP address in the test code (e.g. [here](https://github.com/emp-toolkit/emp-ot/blob/master/test/ot.cpp))

2. run `./bin/[binary] 1 [port] logn` on one machine and 
  
   run `./bin/[binary] 2 [port] logn` on the other machine.
  
Performance
=====
All tested between two AWS c5.4xlarge intances.

## IKNP-style protocols

```
50 Mbps
128 NPOTs:	Tests passed.	12577 us
Passive IKNP OT	Tests passed.	129262 OTps
Passive IKNP COT	Tests passed.	388316 OTps
Passive IKNP ROT	Tests passed.	386190 OTps
128 COOTs:	Tests passed.	11073 us
Active IKNP OT	Tests passed.	129152 OTps
Active IKNP COT	Tests passed.	387380 OTps
Active IKNP ROT	Tests passed.	385235 OTps

10 Gbps
128 NPOTs:	Tests passed.	11739 us
Passive IKNP OT	Tests passed.	1.55476e+07 OTps
Passive IKNP COT	Tests passed.	2.96661e+07 OTps
Passive IKNP ROT	Tests passed.	1.65765e+07 OTps
128 COOTs:	Tests passed.	20064 us
Active IKNP OT	Tests passed.	1.39589e+07 OTps
Active IKNP COT	Tests passed.	2.42705e+07 OTps
Active IKNP ROT	Tests passed.	1.47379e+07 OTps

```

## Ferret protocols
(unit: million random correlated OT per second)
### Semi-honest
bandwidth |10 Mbps|30 Mbps|50 Mbps
------------------|-------|-------|-------
1  thread         |12.1   |16.0   |16.0
2 threads         |16.3   |27.0   |30.8
3 threads         |18.3   |34.2   |40.7
4 threads         |19.7   |39.5   |48.8
5 threads         |20.5   |43.2   |55.0
6 threads         |21.4   |47.1   |61.2

### Malicious
bandwidth |10 Mbps|30 Mbps|50 Mbps
------------------|-------|-------|-------|
1 thread          |11.6   |13.9   |13.9
2 threads         |16.0   |26.6   |27.1
3 threads         |18.3   |33.8   |40.0
4 threads         |19.6   |38.3   |47.4
5 threads         |20.4   |42.4   |53.7
6 threads         |21.3   |46.5   |59.8

Usage
=====
Our test files already provides useful sample code. Here we provide an overview.

Standard OT
-----

```cpp
#include<emp-tool/emp-tool.h> // for NetIO, etc
#include<emp-ot/emp-ot.h>   // for OTs

block b0[length], b1[length];
bool c[length];
NetIO io(party==ALICE ? nullptr:"127.0.0.1", port); // Create a network with Bob connecting to 127.0.0.1
OTNP<NetIO> np(&io); // create a Naor Pinkas OT using the network above
if (party == ALICE)
// ALICE is sender, with b0[i] and b1[i] as messages to send
    np.send(b0, b1, length); 
else
// Bob is receiver, with c[i] as the choice bit 
// and obtains b0[i] if c[i]==0 and b1[i] if c[i]==1
    np.recv(b0, c, length);  
```
Note that `NPOT` can be replaced to `OTCO`, `IKNP`, or `FerretCOT` without changing any other part of the code. They all share the same [API](https://github.com/emp-toolkit/emp-ot/blob/master/emp-ot/ot.h)

Correlated OT and Random OT
-----

Correlated OT and Random OT are supported for `IKNP` and `FerretCOT`. See following as an example. They all share extra [APIs](https://github.com/emp-toolkit/emp-ot/blob/master/emp-ot/cot.h)
```cpp
block delta;

IKNP<NetIO> ote(&io, false); // create a semi honest OT extension

//Correlated OT
if (party == ALICE)
    ote.send_cot(b0, delta, length);
else
    ote.recv_cot(b0, c, length);
    
//Random OT
if (party == ALICE)
    ote.send_rot(b0, b1, length);
else
    ote.recv_rot(b0, c, length);
```

Ferret OT
-----

Ferret OT produces correlated OT with random choice bits (rcot). Extra APIs are [here](https://github.com/emp-toolkit/emp-ot/blob/master/emp-ot/ferret/ferret_cot.h). Our implementation provides two interface `ferretot.rcot()` and `ferretot.rcot_inplace()`. While the first one support filling an external array of any length, an extra memcpy is needed. The second option work on the provided array directly and thus avoid the memcpy. However, it produces a fixed number of OTs (`ferretcot->n`) for every invocation. The [sample code](https://github.com/emp-toolkit/emp-ot/blob/master/test/ferret.cpp#L7) is mostly self-explainable on how to use it.

Note that the choice bit is embedded as the least bit of the `block` on the receiver's side. To make sure the correlation works for all bits, the least bit of Delta is 1. This can be viewed as an extension of the point-and-permute technique. See [this code](https://github.com/emp-toolkit/emp-ot/blob/master/emp-ot/ferret/ferret_cot.hpp#L211) on how ferret is used to fullfill standard `cot` interface.

Citation
=====
```latex
@misc{emp-toolkit,
   author = {Xiao Wang and Alex J. Malozemoff and Jonathan Katz},
   title = {{EMP-toolkit: Efficient MultiParty computation toolkit}},
   howpublished = {\url{https://github.com/emp-toolkit}},
   year={2016}
}
```

Question
=====
Please send email to wangxiao@cs.northwestern.edu. Ferret is also developed and maintained by Chenkai Weng (ckweng@u.northwestern.edu).

## Acknowledgement
This work was supported in part by the National Science Foundation under Awards #1111599 and #1563722. The Ferret implementation is partially based upon work supported by DARPA under Contract No. HR001120C0087. Any opinions, findings and conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of DARPA. The authors would also like to thank the support from PlatON Network and ChainLink Lab.
