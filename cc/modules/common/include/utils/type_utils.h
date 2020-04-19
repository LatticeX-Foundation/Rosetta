// ==============================================================================
// Copyright 2020 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
#ifndef TYPE_UTILS_H_
#define TYPE_UTILS_H_

extern "C"{
#include "relic.h"
}

#include "fix16.h"

namespace typeutils {

uint16_t to_conv_endian16(uint16_t i) {
	uint16_t i1, i2;
	i1 = i & 0xFF;
	i2 = (i >> 8) & 0xFF;

	return ((uint16_t) i1 << 8) | i2;
}

uint32_t to_conv_endian32(uint32_t i) {
	uint32_t i1, i2, i3, i4;
	i1 = i & 0xFF;
	i2 = (i >> 8) & 0xFF;
	i3 = (i >> 16) & 0xFF;
	i4 = (i >> 24) & 0xFF;

	return ((uint32_t) i1 << 24) | ((uint32_t) i2 << 16) | ((uint32_t) i3 << 8)
			| i4;
}

int get_fix16(float value, bn_t big1, bn_t big2)
{
	//div fix16 value
	LOGI("the value: %f", value);
	fix16_t f16 = fix16_from_float(value);
	unsigned char* pz = (unsigned char*)&f16;
	LOGI("%02x %02x %02x %02x", pz[3],pz[2],pz[1],pz[0]);

	//send 16-bit parts
	unsigned short v16_J = f16 & 0x0000ffff;//J part
	unsigned short v16_S = f16 >> 16 & 0x0000ffff;//S paj	
	LOGI("J part: %d, S part: %d", v16_J, v16_S);

	//to big-endian, just revert endian
	uint16_t spart = to_conv_endian16(v16_S);
	uint16_t jpart = to_conv_endian16(v16_J);

    //big number
    bn_read_bin(big1, (uint8_t*)&spart, sizeof(spart));
    bn_read_bin(big2, (uint8_t*)&jpart, sizeof(jpart));

	char data1[256] = {0};
    memset(data1, 0, 256);
    bn_write_str(data1, 256, big1, 10);

    char data2[256] = {0};
    memset(data2, 0, 256);
    bn_write_str(data2, 256, big2, 10);
    LOGD("the recovery J: %s, S: %s", data2, data1);

	//std::vector<bn_t> vec;
    //vec.push_back(big1);
    //vec.push_back(big2);//<s,j>

    return 0;
}


int get_float_from_bn(bn_t big1, bn_t big2, float& out)
{
	uint16_t spart, jpart; 
	uint8_t bin1[128] = {0};
	memset(bin1, 0, 128);
	int len1 = bn_size_bin(big1);
	bn_write_bin(bin1, len1, big1);
	printf("S part binary:\n");
	for (int i = 0; i < len1; ++i)
	{
		printf("%02x ", bin1[i]);
	}
	printf("\n");

	uint8_t bin2[128] = {0};
	memset(bin2, 0, 128);
	int len2 = bn_size_bin(big2);
	bn_write_bin(bin2, len2, big2);
	printf("J part binary:\n");
	for (int i = 0; i < len2; ++i)
	{
		printf("%02x ", bin2[i]);
	}
	printf("\n");
	if (len1 <= 2 && len2 <= 2)
	{
		//to little endian
		uint16_t s = bin1[0];
		if (len1 == 2)
			s = s << 8 | bin1[1];

		uint16_t j = bin2[0];
		if (len2 == 2)
			j = j << 8 | bin2[1];

		fix16_t f16 = s;
		f16 = f16 << 16 | j;
		out = fix16_to_float(f16);
		LOGD("len1 %d, len2: %d, the output s: %d, j: %d, f16: %d, float out: %f", len1, len2, s, j, f16, out);
		return 0;
	}
	else
	{
		LOGE("bad big number, big1 len: %d, big2 len: %d", len1, len2);
		return -1;
	}

	// LOGD("len1: %d, len2: %d", len1, len2);

	// char text[32] = {0};
	// bn_write_str(text, 32, big1, 10);
	// spart = (uint16_t)atoi(text);
	// LOGD("text big1 recory: %s, spart: %d", text, spart);
	

	// memset(text, 0, 32);
	// bn_write_str(text, 32, big2, 10);
	// jpart = (uint16_t)atoi(text);
	// LOGD("text big2 recory: %s, jpart: %d", text, jpart);

	// //send 16-bit parts
	// fix16_t fix16 = spart << 16 | jpart;
	// out = fix16_to_float(fix16);
	// LOGD("## J part: %d, S part: %d, out: %f", spart, jpart, out);

	return 0;
}

} // typeutil



#endif // TYPE_UTILS_H_




