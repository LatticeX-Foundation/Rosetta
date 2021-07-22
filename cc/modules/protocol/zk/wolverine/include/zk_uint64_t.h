#pragma once

#include <stdint.h>
#include <string>
#include <iostream>
// #include "cc/third_party/emp-toolkit/wizard-zk-fp/vole-arith/utility.h"

using std::string;
using std::ostream;
using namespace emp;

namespace rosetta {
namespace zk {

// Note[GeorgeShi]: this is actually the abstraction of Field element \in F_p
class ZkUint64 {
public:
  uint64_t value;
  
	ZkUint64() : value(0) {}

	ZkUint64(const ZkUint64& obj) {
		this->value = obj.value;
	}

	ZkUint64(int64_t input) {
		value = mod(input);
	}

	friend ostream& operator<<(ostream& os, const ZkUint64& obj) {
		char buf0[8] = {0};
		unsigned char* p0 = (unsigned char*)&(obj.value);//value
		string s0, s1;
		for (int i = 0; i < 8; i++) {
			sprintf(buf0, "%02x", p0[i] & 0xFF);
			s0.append(buf0);
		}
		
		os << s0;
		return os;
	}

	ZkUint64 operator+(const ZkUint64& rhs) const {
		ZkUint64 res(*this);
    res.value = mod(rhs.value + res.value);
		return res;
	}

	ZkUint64 operator*(const ZkUint64& rhs) const {
		ZkUint64 res(*this);
		res.value = mult_mod(rhs.value, res.value);
		return res;
	}

	ZkUint64 operator*(const uint64_t& rhs) const {
		ZkUint64 res(*this);
		res.value = mult_mod(rhs, res.value);
		return res;
	}

	// add
	ZkUint64& operator+=(const ZkUint64& rhs) {
    this->value = mod(rhs.value + value);
    return *this;
  }

	// sub
	ZkUint64& operator-=(const ZkUint64& rhs) {
		// ZkUint64 tmp = zk_fp_sub(*this, rhs);
		// this->value = tmp.value;
		this->value = mod(value + (PR - rhs.value));
    return *this;
  }

	// substract
	ZkUint64 operator-(const ZkUint64& rhs) const {
		return zk_fp_sub(*this, rhs);
  }

	// divide
	ZkUint64 operator/(const ZkUint64& rhs) const {
		// [kelvin] TODO: I will implement this interface
		return ZkUint64(rhs);
	}

	// negate
	ZkUint64 operator-() const {
    return zk_fp_neg(*this);
  }

	// substract
	bool operator>=(const ZkUint64& rhs) const {
		if (this->value >= rhs.value)
			return true;
		else
			return false;
  }

	static inline ZkUint64 zk_fp_sub(const ZkUint64 &lhs, const ZkUint64 &rhs)
	{
		ZkUint64 res = lhs;
		res.value = mod(res.value + (PR - rhs.value));
		return res;
	}

	static inline ZkUint64 zk_fp_neg(const ZkUint64 &lhs)
	{
		ZkUint64 res = lhs;
		res.value = mod(PR - lhs.value);
		return res;
	}

	static inline void zk_fp_neg_this(ZkUint64 &lhs)
	{
		lhs.value = mod(PR - lhs.value);
	}

};//ZkUint64

}//zk
}//rosetta
