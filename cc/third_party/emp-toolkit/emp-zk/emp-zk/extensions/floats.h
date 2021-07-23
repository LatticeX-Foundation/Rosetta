#ifndef __EMP_ZK_FLOAT_
#define __EMP_ZK_FLOAT_

#include "emp-tool/emp-tool.h"
#include <iostream>
using namespace emp;
using namespace std;

inline Integer FloatToInt62(Float input, int s) {
	Integer fraction(25, 0, PUBLIC);
	memcpy(fraction.bits.data(), input.value.data(), 23*sizeof(block));
	fraction[23] = Bit(true, PUBLIC);
	fraction.resize(61, false);

	Integer exp(8, 0, PUBLIC);
	memcpy(exp.bits.data(), input.value.data()+23, 8*sizeof(block));
	exp = exp - Integer(8, 127+23-s, PUBLIC);
	Integer negexp = -exp;
	fraction = If(!exp[7], fraction << exp, fraction >> negexp);
	fraction = If(negexp>=Integer(8, 61, PUBLIC), Integer(61, 0, PUBLIC), fraction);
	fraction = If(input[31], -fraction, fraction);
	fraction.resize(62, false);
	return fraction;
}

inline Float Int62ToFloat(Integer input, int s) {
	input = If(input > Integer(62, (1ULL<<60) - 1, PUBLIC), input - Integer(62, (1ULL<<61) - 1, PUBLIC),input);
	input.bits.pop_back();
	assert(input.size() == 61);
	const Integer twentyThree(8, 23, PUBLIC);

	Float output(0.0, PUBLIC);
	Bit signBit = input.bits[60];
	Integer unsignedInput = input.abs();
	//	for(int i = 0; i < 61; ++i)cout << unsignedInput[i].reveal(PUBLIC);cout <<endl;

	Integer firstOneIdx = Integer(8, 60, PUBLIC) - unsignedInput.leading_zeros().resize(8);
	Bit leftShift = firstOneIdx >= twentyThree;
	Integer shiftOffset = If(leftShift, firstOneIdx - twentyThree, twentyThree - firstOneIdx);
	Integer shifted = If(leftShift, unsignedInput >> shiftOffset, unsignedInput << shiftOffset);
	// exponent is biased by 127
	Integer exponent = firstOneIdx + Integer(8, 127 - s, PUBLIC);

	output.value[31] = signBit;
	memcpy(output.value.data()+23, exponent.bits.data(), 8*sizeof(block));
	memcpy(output.value.data(), shifted.bits.data(), 23*sizeof(block));
	return output;
}

#endif //__EMP_ZK_FLOAT_
