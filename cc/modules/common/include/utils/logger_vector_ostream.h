// ==============================================================================
// Copyright 2021 The LatticeX Foundation
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
#pragma once

#include "spdlog/fmt/ostr.h"
#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"
#include "cc/modules/protocol/mpc/helix/include/helix_def.h"
#include <ostream>

#include <openssl/sha.h>
#include <vector>
#include <stdio.h>
#include <string.h>

#define AUDIT_ARRAY_LIMIT 64
#define AUDIT_STRING_LIMIT 512 

	template<typename OStream>
OStream &operator<<(OStream &os, const rosetta::helix::Share& r)
{
#if defined(ROSETTA_MPC_128) && defined(__SIZEOF_INT128__)
	char str[66];
	unsigned char* p = (unsigned char*)&r;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	snprintf(str, 66, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
			 p[0],
			 p[1],
			 p[2],
			 p[3],
			 p[4],
			 p[5],
			 p[6],
			 p[7],
			 p[8],
			 p[9],
			 p[10],
			 p[11],
			 p[12],
			 p[13],
			 p[14],
			 p[15],
			 p[16],
			 p[17],
			 p[18],
			 p[19],
			 p[20],
			 p[21],
			 p[22],
			 p[23],
			 p[24],
			 p[25],
			 p[26],
			 p[27],
			 p[28],
			 p[29],
			 p[30],
			 p[31]);
#else
	snprintf(str, 66, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X,%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
			 p[15],
			 p[14],
			 p[13],
			 p[12],
			 p[11],
			 p[10],
			 p[9],
			 p[8],
			 p[7],
			 p[6],
			 p[5],
			 p[4],
			 p[3],
			 p[2],
			 p[1],
			 p[0],
			 p[31],
			 p[30],
			 p[29],
			 p[28],
			 p[27],
			 p[26],
			 p[25],
			 p[24],
			 p[23],
			 p[22],
			 p[21],
			 p[20],
			 p[19],
			 p[18],
			 p[17],
			 p[16]);
#endif
#else
	char str[34];
	mpc_t* p = (mpc_t*)&r;
	sprintf(str, "%016lX,%016lX", *p, *(p+1));
	os<<str<<" ";
#endif
	return os;
}
extern template std::ostream& operator<< <std::ostream>(std::ostream&, const rosetta::helix::Share&);

	template<typename OStream>
OStream &operator<<(OStream &os, const rosetta::helix::BitShare& r)
{
	char str[34];
	bit_t* p = (bit_t*)&r;
	sprintf(str, "%02X,%02X", *p, *(p+1));
	os<<str<<" ";

	return os;
}
extern template std::ostream& operator<< <std::ostream>(std::ostream&, const rosetta::helix::BitShare&);

struct CStr
{
	const char* str_;
	size_t size_;
	explicit CStr(const char* str, size_t size): str_(str), size_(size){}

	template<typename OStream> 
		friend OStream& operator<<(OStream& os, const CStr& r); 
};

template<typename OStream> 
OStream& operator<<(OStream& os, const CStr& r) 
{
	size_t size = r.size_;
	if (size > AUDIT_STRING_LIMIT) {
		char bin[33];
		::SHA256((const unsigned char*)r.str_, r.size_, (unsigned char*)bin);
		bin[32] = 0;
		char res[17];
		snprintf(res, 17, "%02X%02X%02X%02X%02X%02X%02X%02X", 
						 (unsigned char)bin[0],
						 (unsigned char)bin[1],
						 (unsigned char)bin[2],
						 (unsigned char)bin[3],
						 (unsigned char)bin[4],
						 (unsigned char)bin[5],
						 (unsigned char)bin[6],
						 (unsigned char)bin[7]);

		os<<" ["<<res<<"] ["<<AUDIT_STRING_LIMIT<<"/"<<size<<"]=>";
		size = AUDIT_STRING_LIMIT;
	} else {
		os<<" ["<<size<<"/"<<size<<"]=>";
	}

	const unsigned char* p = (const unsigned char*)r.str_;
	char tmp[3];
	for (size_t i=0; i<size; ++i) {
		snprintf(tmp, 3, "%02X", *(p+i));
		os<<tmp;
	}

	return os;
}
extern template std::ostream& operator<< <std::ostream>(std::ostream&, const CStr&);
