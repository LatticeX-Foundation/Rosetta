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

#include"cc/modules/common/include/utils/logger_vector_ostream.h"

template<typename T>
struct Vector {
	const std::vector<T>& vec_;
	explicit Vector(const std::vector<T>& vec) : vec_(vec) {
	}

	template<typename OStream> friend OStream &operator<<(OStream &os, const Vector& r)
	{
		int size = r.vec_.size();
		if (size > AUDIT_ARRAY_LIMIT) {
			char bin[33];
			::SHA256((const unsigned char*)r.vec_.data(), sizeof(T) * r.vec_.size(), (unsigned char*)bin);
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

			os<<" ["<<res<<"] ["<<AUDIT_ARRAY_LIMIT<<"/"<<size<<"]=>";
			size = AUDIT_ARRAY_LIMIT ;
		} else {
			os<<" ["<<size<<"/"<<size<<"]=>";
		}

		for (int i=0; i<size; ++i) {
			os<<r.vec_[i] <<" ";
		}

		return os;
	}
};

#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"
template<>
struct Vector<mpc_t> {
	const std::vector<mpc_t>& vec_;
	explicit Vector(const std::vector<mpc_t>& vec) : vec_(vec) {
	}

	template<typename OStream> friend OStream &operator<<(OStream &os, const Vector& r)
	{
		int size = r.vec_.size();
		if (size > AUDIT_ARRAY_LIMIT) {
			char bin[33];
			::SHA256((const unsigned char*)r.vec_.data(), sizeof(mpc_t) * r.vec_.size(), (unsigned char*)bin);
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

			os<<" ["<<res<<"] ["<<AUDIT_ARRAY_LIMIT<<"/"<<size<<"]=>";
			size = AUDIT_ARRAY_LIMIT ;
		} else {
			os<<" ["<<size<<"/"<<size<<"]=>";
		}

#if defined(ROSETTA_MPC_128) && defined(__SIZEOF_INT128__)
		char str[33];
		for (int i=0; i<size; ++i) {
			unsigned char* p = (unsigned char*)&r.vec_[i];
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
			snprintf(str, 33, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
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
					 p[15]);
#else
			snprintf(str, 33, "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
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
					 p[0]);
#endif
			os<<str<<" ";
		}
#else
		char str[17];
		for (int i=0; i<size; ++i) {
			sprintf(str, "%016lX", r.vec_[i]);
			os<<str<<" ";
		}
#endif
		return os;
	}
};

template<>
struct Vector<small_mpc_t> {
	const std::vector<small_mpc_t>& vec_;
	explicit Vector(const std::vector<small_mpc_t>& vec) : vec_(vec) {
	}

	template<typename OStream> friend OStream &operator<<(OStream &os, const Vector& r)
	{
		int size = r.vec_.size();
		if (size > AUDIT_ARRAY_LIMIT) {
			char bin[33];
			::SHA256((const unsigned char*)r.vec_.data(), sizeof(small_mpc_t) * r.vec_.size(), (unsigned char*)bin);
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

			os<<" ["<<res<<"] ["<<AUDIT_ARRAY_LIMIT<<"/"<<size<<"]=>";
			size = AUDIT_ARRAY_LIMIT;
		} else {
			os<<" ["<<size<<"/"<<size<<"]=>";
		}

		//char str[1025];
		char tmp[4];
		for (int i=0; i<size; ++i) {
			snprintf(tmp, 4, "%02X ", r.vec_[i]);
			os<<tmp;
		}

		return os;
	}
};

template<>
struct Vector<std::string> {
	const std::vector<std::string>& vec_;
	explicit Vector(const std::vector<std::string>& vec) : vec_(vec) {
	}

	template<typename OStream> friend OStream &operator<<(OStream &os, const Vector& r)
	{
		int size = r.vec_.size();
		if (size > AUDIT_ARRAY_LIMIT) {
			size_t len = 0;
			for (int i=0; i<size; ++i) {
				len += r.vec_[i].capacity();
			}
			char bin[33];
			::SHA256((const unsigned char*)r.vec_[0].c_str(), len, (unsigned char*)bin);
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

			os<<" ["<<res<<"] ["<<AUDIT_ARRAY_LIMIT<<"/"<<size<<"]=>";
			size = AUDIT_ARRAY_LIMIT;
		} else {
			os<<" ["<<size<<"/"<<size<<"]=>";
		}

		//char str[1025];
		char tmp[4];
		for (int i=0; i<size; ++i) {
			for (int j=0; j<r.vec_[i].size(); ++j) {
				snprintf(tmp, 3, "%02X", r.vec_[i][j]);
				os<<tmp;
			}
			os<<" ";
		}

		return os;
	}
};

#include "cc/modules/protocol/mpc/helix/include/helix_def.h"
template<>
struct Vector<rosetta::helix::Share> {
	const std::vector<rosetta::helix::Share>& vec_;
	explicit Vector(const std::vector<rosetta::helix::Share>& vec) : vec_(vec) {
	}

	template<typename OStream> friend OStream &operator<<(OStream &os, const Vector& r)
	{
		int size = r.vec_.size();
		if (size > AUDIT_ARRAY_LIMIT) {
			char bin[33];
			::SHA256((const unsigned char*)r.vec_.data(), sizeof(rosetta::helix::Share) * r.vec_.size(), (unsigned char*)bin);
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

			os<<" ["<<res<<"] ["<<AUDIT_ARRAY_LIMIT<<"/"<<size<<"]=>";
			size = AUDIT_ARRAY_LIMIT ;
		} else {
			os<<" ["<<size<<"/"<<size<<"]=>";
		}

#if defined(ROSETTA_MPC_128) && defined(__SIZEOF_INT128__)
		char str[66];
		for (int i=0; i<size; ++i) {
			unsigned char* p = (unsigned char*)&r.vec_[i];
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
					 os<<str<<" ";
		}
#else
		char str[34];
		for (int i=0; i<size; ++i) {
			mpc_t* p = (mpc_t*)&r.vec_[i];
			sprintf(str, "%016lX,%016lX", *p, *(p+1));
			os<<str<<" ";
		}
#endif
		return os;
	}
};

template<>
struct Vector<rosetta::helix::BitShare> {
	const std::vector<rosetta::helix::BitShare>& vec_;
	explicit Vector(const std::vector<rosetta::helix::BitShare>& vec) : vec_(vec) {
	}

	template<typename OStream> friend OStream &operator<<(OStream &os, const Vector& r)
	{
		int size = r.vec_.size();
		if (size > AUDIT_ARRAY_LIMIT) {
			char bin[33];
			::SHA256((const unsigned char*)r.vec_.data(), sizeof(rosetta::helix::BitShare) * r.vec_.size(), (unsigned char*)bin);
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

			os<<" ["<<res<<"] ["<<AUDIT_ARRAY_LIMIT<<"/"<<size<<"]=>";
			size = AUDIT_ARRAY_LIMIT ;
		} else {
			os<<" ["<<size<<"/"<<size<<"]=>";
		}

		char str[34];
		for (int i=0; i<size; ++i) {
			bit_t* p = (bit_t*)&r.vec_[i];
			sprintf(str, "%02X,%02X", *p, *(p+1));
			os<<str<<" ";
		}

		return os;
	}
};


extern template struct Vector<char>;
extern template struct Vector<mpc_t>;
extern template struct Vector<small_mpc_t>;
extern template struct Vector<rosetta::helix::Share>;
extern template struct Vector<rosetta::helix::BitShare>;

