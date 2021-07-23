# EMP-zk  
![arm](https://github.com/emp-toolkit/emp-zk/workflows/arm/badge.svg)
![x86](https://github.com/emp-toolkit/emp-zk/workflows/x86/badge.svg)

<img src="https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/art/logo-full.jpg" width=300px/>

Protocols
=====
The code in this repo implements a fast, scalable, communication-efficient zero-knowledge proof protocol for Boolean/arithmetic circuits and polynomials. The protocols are described in [Wolverine](https://eprint.iacr.org/2020/925), [Quicksilver](https://eprint.iacr.org/2021/076) and Mystique schemes. Additionally, it contains the subfield vector oblivious linear evaluation in [Wolverine](https://eprint.iacr.org/2020/925).

Installation
=====

1. `wget https://raw.githubusercontent.com/emp-toolkit/emp-readme/master/scripts/install.py`
2. `python install.py -install -tool -ot`
    1. By default it will build for Release. `-DCMAKE_BUILD_TYPE=[Release|Debug]` option is also available.
    2. No sudo? Change [`CMAKE_INSTALL_PREFIX`](https://cmake.org/cmake/help/v2.8.8/cmake.html#variable%3aCMAKE_INSTALL_PREFIX).
3. `git clone [this repo] && cmake . && make`

Test
=====

Testing on localhost
-----

   `./run ./bin/[binary]`

   
Testing on two
-----

1. Change the IP address in the test code 

2. run `./bin/[binary] 1 [port]` on one machine and 
  
   run `./bin/[binary] 2 [port]` on the other machine.

 
Performance
=====
The test is done by two AWS EC2 m5.2xlarge servers with throttled network.

Throughput of circuit-based ZK protocol
-----
All values are for "million gates per second".
##### Boolean circuits
|Threads|10 Mbps|20 Mbps|30 Mbps|50 Mbps|Localhost|
|-------|-------|-------|-------|-------|---------|
|1|4.4|6.2|7.0|7.5|7.6|
|2|5.3|8.1|9.9|11.8|11.8|
|3|5.7|9.1|11.4|13.9|14.3|
|4|5.8|9.9|12.2|14.9|15.8|
##### Arithmetic circuits
|Threads|100 Mbps|500 Mbps|1 Gbps|2 Gbps|Localhost|
|-------|-------|-------|-------|-------|---------|
|1|1.2|3.4|4.2|4.8|4.8|
|2|1.3|4.4|6.1|7.0|7.1|
|3|1.4|4.9|7.2|8.4|8.4|
|4|1.4|5.0|7.5|8.9|8.9|


Question
=====
Please send email to Chenkai Weng (ckweng@u.northwestern.edu).

## Acknowledgement
This work was supported in part DARPA under Contract No. HR001120C0087. Any opinions, findings and conclusions or recommendations expressed in this material are those of the author(s) and do not necessarily reflect the views of DARPA. The authors would also like to thank the support from PlatON Network and ChainLink Lab.
