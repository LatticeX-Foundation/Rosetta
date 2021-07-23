#pragma once

#include "emp-tool/emp-tool.h"
#include "emp-zk/emp-zk-bool/emp-zk-bool.h"
#include "emp-zk/emp-zk-arith/int_fp.h"

template<typename IO>
inline IntFp bool2arith(Integer &x) {
	IntFp y;
	y.value = EdaBits<IO>::conv->bool2arith(x);
	return y;
}

template<typename IO>
inline void bool2arith(IntFp *y, Integer *x, int sz) {
	EdaBits<IO>::conv->bool2arith((__uint128_t*)y, x, sz);
}

template<typename IO>
inline Integer arith2bool(IntFp &x) {
	return EdaBits<IO>::conv->arith2bool(x.value);
}

template<typename IO>
inline void arith2bool(Integer *y, IntFp *x, int sz) {
	EdaBits<IO>::conv->arith2bool(y, (__uint128_t*)x, sz);
}
