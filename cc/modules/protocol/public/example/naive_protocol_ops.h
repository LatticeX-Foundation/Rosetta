// // ==============================================================================
// // Copyright 2020 The LatticeX Foundation
// // This file is part of the Rosetta library.
// //
// // The Rosetta library is free software: you can redistribute it and/or modify
// // it under the terms of the GNU Lesser General Public License as published by
// // the Free Software Foundation, either version 3 of the License, or
// // (at your option) any later version.
// //
// // The Rosetta library is distributed in the hope that it will be useful,
// // but WITHOUT ANY WARRANTY; without even the implied warranty of
// // MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// // GNU Lesser General Public License for more details.
// //
// // You should have received a copy of the GNU Lesser General Public License
// // along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// // ==============================================================================

// #pragma once
// #include <string>
// #include <unordered_map>
// #include <vector>
// // #include <any>

// using namespace std;

// namespace rosetta {
// // This is the interface that each specific cryptographic protocol should implement!
// class ProtocolOps {
//  protected:
//   string _op_msg_id;

//   // TODO:use c++17
//   // typedef std::unordered_map<string, std::any> attr_type;
//   typedef std::unordered_map<string, string> attr_type;


// public:
//     ProtocolOps(const string& msg_id): _op_msg_id(msg_id) {};
//     /*
//         @desc: encode the literal number to a protocol-specific format 
//                 wrappered as string.
//         @param:
//             in, the vector of literal number
//             out, the vector of the encoded strings
//         @return:
//             0 if success, errcode otherwise
//     */
//     // TODO: cannot use virtual function with template
//     // template <typename T> 
//     virtual int Tf2Rtt(const vector<double> in,
//                         vector<string>* out);

//     // decode the string in protocol-specific format to literal number
//     // TODO: cannot use virtual function with template
//     // template <typename T>
//     virtual int Rtt2Tf(const vector<string>* in,
//                         vector<double>* out);

//     virtual int RandSeed(std::string op_seed,
//                         string& out_str);

//     virtual int PrivateInput(int party_id,
//                             double in_v,
//                             string& out_str);
    
//     virtual int PrivateInput(int party_id,
//                             vector<double> in_vec,
//                             vector<string>& out_str_vec);

//     virtual int Synchronize();
//     virtual int SyncAesKey(int partyA, int partyB, string& key_send, string& key_recv);
//     virtual int PRZS(int party0, int party1, vector<string>& strings);

//     virtual int Trunc(vector<string>& X, size_t size);
//     virtual int Reveal(const vector<string>& a, vector<string>& b, attr_type& attr);

//     virtual int Negative(const vector<string>& a, vector<string>& b, attr_type& attr);

//     virtual int Add(const vector<string> & input_a,
//                     const vector<string>& input_b,
//                     vector<string>* output_c,
//                     attr_type& attr);

//     virtual int Sub(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);
    
//     virtual int Mul(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);
    
//     virtual int Div(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);
    
//     virtual int FloorDiv(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);

//     virtual int Suqare(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);
//     virtual int PowConst(const vector<string>& a,
//                     int b,
//                     vector<string>& c,
//                     attr_type& attr);
//     virtual int Pow(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);

//     virtual int Min(const vector<string>& a,
//                     vector<string>& b,
//                     attr_type& attr);
//     virtual int Max(const vector<string>& a,
//                     vector<string>& b,
//                     attr_type& attr);
//     virtual int Sum(const vector<string>& a,
//                     vector<string>& b,
//                     attr_type& attr);
//     virtual int Mean(const vector<string>& a,
//                     vector<string>& b,
//                     attr_type& attr);

//     virtual int MatMul(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);

//     virtual int Equal(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);

//     virtual int NotEqual(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);
  
//     virtual int Less(const vector<string>& a,
//                 const vector<string>& b,
//                 vector<string>& c,
//                 attr_type& attr);
    
//     virtual int LessEqual(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);
    
//     virtual int Greater(const vector<string>& a,
//                     const vector<string>& b,
//                     vector<string>& c,
//                     attr_type& attr);
    
//     virtual int GreaterEqual(const vector<string>& a,
//                         const vector<string>& b,
//                         vector<string>& c,
//                         attr_type& attr);

//     virtual int MSB(const vector<string>& a,
//                     vector<string>& b,
//                     attr_type& attr);
//     virtual int RelLU(const vector<string>& a,
//                     vector<string>& b,
//                     attr_type& attr);
//     virtual int DReLU(const vector<string>& a,
//                     vector<string>& b,
//                     attr_type& attr);

//     virtual int Log(const vector<string>& a,
//                     vector<string>& b,
//                     attr_type& attr);
//     virtual int Log1p(const vector<string>& a,
//                     vector<string>& b,
//                     attr_type& attr);
//     virtual int Sigmoid(const vector<string>& a,
//                     vector<string>& b,
//                     attr_type& attr);

// };

// } // namespace rosetta
