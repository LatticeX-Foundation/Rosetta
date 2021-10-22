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

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>

#include "cc/modules/protocol/zk/mystique/include/mystique_impl.h"

#include "cc/modules/protocol/zk/mystique/include/mystique_ops_impl.h"
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/protocol/utility/include/util.h"
// #include "cc/modules/protocol/utility/include/config.h"

#include "cc/modules/protocol/zk/mystique/include/wvr_util.h"
#include "cc/modules/protocol/zk/mystique/include/mystique_internal.h"
#include "cc/modules/protocol/utility/include/prg.h"
#include "cc/modules/common/include/utils/perf_stats.h"
#include "cc/modules/protocol/zk/mystique/include/mystique_plain_internal.h"

using namespace rosetta::zk;
using emp::block;
using rosetta::zk::ZkIntFp;

#define DEBUG_PRINT_IMP_OPS 1
#define USE_MATMUL_PROOF 0
#define USE_ZK_INT_FP_BATCH 1
#define USE_RELU_SOFTMAX 0

using ZkVec = std::vector<rosetta::zk::ZkIntFp>;
using StrVec = std::vector<std::string>;
extern int64_t matmul_convert_alloc_time; //us
extern int64_t matmul_proof_time; //us

namespace rosetta {

int MystiqueOpsImpl::PrivateInput(const string& node_id, const vector<double>& in_x, vector<string>& out_x){
    int p = io->GetPartyId(node_id);
    return  PrivateInput(p,in_x,out_x);
}

int MystiqueOpsImpl::PrivateInput(
  int party_id,
  const vector<double>& in_x,
  vector<string>& out_x) {
  tlog_debug << "calling MystiqueOpsImpl::PrivateInput" << ENDL;

  int vec_size = in_x.size();
  out_x.resize(vec_size);

  int zk_input_party_id = party_id + 1;
  if (zk_input_party_id != ALICE) {
    tlog_error << " In ZK, only private input from Prover is allowed!";
    throw std::runtime_error(string("private input is only allowed for prover"));
  }

  vector<uint64_t> field_elements(vec_size);
  zk_encode(in_x, field_elements);
  
#if LOCAL_SIMULATE
  convert_mac_to_string(field_elements.data(), out_x, vec_size);
  return 0;
#endif

  vector<ZkIntFp> inner_x(vec_size);
  batch_feed((IntFp*)inner_x.data(), field_elements.data(), field_elements.size());
  sync_zk_bool<BoolIO<ZKNetIO>>();

  convert_mac_to_string(inner_x, out_x);
  return 0;
}

int MystiqueOpsImpl::PublicInput(const string& node_id, const vector<double>& in_x, vector<string>& out_x){
    int p = io->GetPartyId(node_id);
    return  PublicInput(p,in_x,out_x);
}

int MystiqueOpsImpl::PublicInput(int party_id, const vector<double>& in_x, vector<string>& out_x) {
  int64_t size = in_x.size();

#if LOCAL_SIMULATE
  vector<uint64_t> field_elements(size);
  zk_encode(in_x, field_elements);
  convert_mac_to_string(field_elements.data(), out_x, size);
  return 0;
#endif

  vector<ZkIntFp> out_v;
  PublicInput(party_id, in_x, out_v);
  convert_mac_to_string(out_v, out_x);
  return 0;
}

int MystiqueOpsImpl::PublicInput(const string& node_id, const vector<double>& in_x, vector<ZkIntFp>& out_x){
    int p = io->GetPartyId(node_id);
    return  PublicInput(p,in_x,out_x);
}

int MystiqueOpsImpl::PublicInput(
  int party_id,
  const vector<double>& in_x,
  vector<ZkIntFp>& out_x) {
  tlog_debug << "calling MystiqueOpsImpl::PublicInput" << ENDL;
  int size = in_x.size();
  out_x.clear();
  out_x.resize(size);
  vector<uint64_t> field_elements(size);
  zk_encode(in_x, field_elements);

  for (int i = 0; i < size; ++i) {
    out_x[i] = ZkIntFp(field_elements[i], PUBLIC);
  }
  return 0;
}

int MystiqueOpsImpl::Broadcast(const string& from_node, const char* msg, char* result, size_t size){
      int from_party = io->GetPartyId(from_node);
    return  Broadcast(from_party,msg,result,size);
}

int MystiqueOpsImpl::Broadcast(int from_party, const char* msg, char* result, size_t size) {
  // from_party \in {0,1}
  // result must be allocate outside!
  if (from_party + 1 == zk_party_id) {
    ((ZKFpExecPrv<BoolIO<ZKNetIO>>*)(ZKFpExec::zk_exec))->io->send_data((const void*)msg, size);
  } else {
    ((ZKFpExecPrv<BoolIO<ZKNetIO>>*)(ZKFpExec::zk_exec))->io->recv_data((void*)result, size);
  }
  return 0;
}

int MystiqueOpsImpl::Broadcast(const string& from_node, const string& msg, string& result){
    int from_party = io->GetPartyId(from_node);
    return  Broadcast(from_party,msg,result);
}

int MystiqueOpsImpl::Broadcast(int from_party, const string& msg, string& result) {
  return Broadcast(from_party, msg.data(), (char*)result.data(), msg.size());
}

int MystiqueOpsImpl::Add(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "-----> Mystique Add";
  int size = a.size();
  int a_scale = 1;
  int b_scale = 1;
  int output_scale_multiplier = 1; 
#if LOCAL_SIMULATE
  vector<uint64_t> fp_a(size);
  vector<uint64_t> fp_b(size);
  vector<uint64_t> fp_c(size);
  a_scale = convert_string_to_mac(a.data(), fp_a.data(), size);
  b_scale = convert_string_to_mac(b.data(), fp_b.data(), size);
  for(int i =0; i < size; ++i) {
    if(a_scale == b_scale && a_scale == 1) {
    fp_c[i] = mod(fp_a[i] + fp_b[i]);
    } else if(a_scale == 1 && b_scale ==2) {
      fp_c[i] = mod(fp_a[i] * _FLOAT_SCALE_ + fp_b[i]);
      output_scale_multiplier = 2;
    } else if(a_scale == 2 && b_scale ==1) {
      fp_c[i] = mod(fp_a[i] + fp_b[i] * _FLOAT_SCALE_);
      output_scale_multiplier = 2;
    }
  }
  convert_mac_to_string(fp_c.data(), output, size, humanable, output_scale_multiplier);
  return 0;
  
  #if DEBUG_MODE
  for(int i = 0; i < size; ++i) {
    tlog_debug << i <<"-th add : " << fp_a[i] << " + " << fp_b[i] << "-->" << fp_c[i];
  }
  #endif

#endif

  vector<ZkIntFp> a_zks(size), b_zks(size), c_zks(size);
  a_scale = convert_string_to_mac(a, a_zks);
  b_scale = convert_string_to_mac(b, b_zks);
  // todo: refine code
  if (a_scale != b_scale && _OUTPUT_WITH_HIGH_SCALE) {
      tlog_info << "Adjust scale for Op Add:" << a_scale << " VS " << b_scale;
  }
  bool need_scale_align = (a_scale == 2 || b_scale ==2);
  for (int i = 0; i < size; ++i) {
    // adjust scale 
    if (a_scale != b_scale) {
      if(a_scale == 2 && b_scale == 1) {
        b_zks[i] = b_zks[i] * _FLOAT_SCALE_;
      } else if (a_scale == 1 && b_scale == 2) {
        a_zks[i] = a_zks[i] * _FLOAT_SCALE_;
      } else {
        throw std::runtime_error("Wrong decoded scale_multiplier!" + to_string(a_scale) + " VS " + to_string(b_scale));
      }
    }

    c_zks[i].value = (a_zks[i] + b_zks[i]).value;
  }
  
  if (need_scale_align) {
    output_scale_multiplier = 2;
  }

  #if DEBUG_MODE
  for(int i = 0; i < size; ++i) {
    tlog_debug << i <<"-th add : " << a_zks[i].value << " + " << b_zks[i].value << "-->" << c_zks[i].value;
  }

  #endif

  convert_mac_to_string(c_zks, output, humanable, output_scale_multiplier);
  tlog_debug << "Mystique Add ok. <----";
  return 0;
}

int MystiqueOpsImpl::Matmul(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  SimpleTimer timer_proof;
  tlog_debug << "----> Mystique Matmul";
  int m = 0, k = 0, n = 0;
  int a_scale = 1;
  int out_scale_multiplier = 1;
  if (
    attr_info && attr_info->count("m") > 0 && attr_info->count("n") > 0 &&
    attr_info->count("k") > 0) {
    m = std::stoi(attr_info->at("m"));
    k = std::stoi(attr_info->at("k"));
    n = std::stoi(attr_info->at("n"));
  } else {
    throw std::runtime_error("please fill m, k, n for Mystique Matmul");
  }

  bool transpose_a = false, transpose_b = false, rh_is_const = false;
  if (attr_info->count("transpose_a") > 0 && attr_info->at("transpose_a") == "1")
    transpose_a = true;
  if (attr_info->count("transpose_b") > 0 && attr_info->at("transpose_b") == "1")
    transpose_b = true;
  if (attr_info->count("rh_is_const") > 0 && attr_info->at("rh_is_const") == "1")
    rh_is_const = true;

  tlog_info << "Mystique Matmul m, k, n: " << m << ", " << k << ", " << n
            << ", rh_is_const: " << rh_is_const;

#if LOCAL_SIMULATE
  // decode to zk matmul for zk matrix
  ZkU64Matrix a_fps(m, k);
  a_scale = convert_string_to_mac(a.data(), a_fps.data(), a.size());
  // assert the input must be well-scaled
  if(a_scale != 1) {
    tlog_error << "scale value for Matmul left input:" << a_scale;
    throw std::runtime_error("Wrong decoded scale_multiplier!" + to_string(a_scale));
  }
  if (transpose_a) {
    a_fps.transposeInPlace();
  }

  ZkU64Matrix c_fps(m, n);
  if (rh_is_const) {
    DoubleMatrix db(k, n);
    convert_string_to_double(b, db.data(), true); // readable double string
    if (transpose_b) {
      db.transposeInPlace();
    }
    matmul_convert_alloc_time += timer_proof.ns_elapse();

    timer_proof.start();
    // todo: FIX this !!
    mystique_plain_matmul_proof_r_const(a_fps, db, c_fps, !_OUTPUT_WITH_HIGH_SCALE);
    
    matmul_proof_time += timer_proof.ns_elapse();
  } else {
    ZkU64Matrix b_fps(k, n);
    int b_scale = convert_string_to_mac(b.data(), (uint64_t*)b_fps.data(), b.size());
      // assert the input must be well-scaled
    if(b_scale != 1) {
      tlog_error << "scale value for Matmul Right input:" << b_scale;
      throw std::runtime_error("Wrong decoded scale_multiplier!:" + to_string(b_scale));
    }
    if (transpose_b) {
      b_fps.transposeInPlace();
    }
    matmul_convert_alloc_time += timer_proof.ns_elapse();

    timer_proof.start();
    // todo: FIX this !!
    mystique_plain_matmul_proof(a_fps, b_fps, c_fps, !_OUTPUT_WITH_HIGH_SCALE);

    matmul_proof_time += timer_proof.ns_elapse();
  }

  // encode output
  out_scale_multiplier = 1;
  if (_OUTPUT_WITH_HIGH_SCALE) {
    tlog_info << "SCALE DOUBLE in Matmul!";
    out_scale_multiplier = 2;
  }
  output.resize(c_fps.size());
  convert_mac_to_string((uint64_t*)c_fps.data(), output, c_fps.size(), humanable, out_scale_multiplier);
  //! cout << "c_fps: \n" << c_fps << ENDL;
  tlog_info << "mystique_matmul_fp_with_proof elapse (us):  " << timer_proof.ns_elapse() / 1000
           << " us, proof accumulate: " << matmul_proof_time / 1000 << " us." << ENDL;
  tlog_info << "Mystique Matmul ok. <---- " << zk_party_id << ",  m, k, n: " << m << ", " << k
           << ", " << n;
  return 0;
#endif

  // decode to zk matmul for zk matrix
  ZkMatrix za(m, k);
  a_scale = convert_string_to_mac(a.data(), (__uint128_t*)za.data(), a.size());
  // assert the input must be well-scaled
  if(a_scale != 1) {
    tlog_error << "scale value for Matmul left input:" << a_scale;
    throw std::runtime_error("Wrong decoded scale_multiplier!" + to_string(a_scale));
  }
  if (transpose_a) {
    za.transposeInPlace();
  }

  ZkMatrix zc(m, n);
  if (rh_is_const) {
    DoubleMatrix db(k, n);
    convert_string_to_double(b, db.data(), true); // readable double string
    if (transpose_b) {
      db.transposeInPlace();
    }
    matmul_convert_alloc_time += timer_proof.ns_elapse();

    timer_proof.start();
    // todo: FIX this !!
    mystique_matmul_proof_r_const(za, db, zc, !_OUTPUT_WITH_HIGH_SCALE);
    
    matmul_proof_time += timer_proof.ns_elapse();
  } else {
    ZkMatrix zb(k, n);
    int b_scale = convert_string_to_mac(b.data(), (__uint128_t*)zb.data(), b.size());
      // assert the input must be well-scaled
    if(b_scale != 1) {
      tlog_error << "scale value for Matmul Right input:" << b_scale;
      throw std::runtime_error("Wrong decoded scale_multiplier!:" + to_string(b_scale));
    }
    if (transpose_b) {
      zb.transposeInPlace();
    }
    matmul_convert_alloc_time += timer_proof.ns_elapse();

    timer_proof.start();
    // todo: FIX this !!
    mystique_matmul_proof(za, zb, zc, !_OUTPUT_WITH_HIGH_SCALE);

    matmul_proof_time += timer_proof.ns_elapse();
  }

  // encode output
  out_scale_multiplier = 1;
  if (_OUTPUT_WITH_HIGH_SCALE) {
    tlog_info << "SCALE DOUBLE in Matmul!";
    out_scale_multiplier = 2;
  }
  convert_mac_to_string((__uint128_t*)zc.data(), output, zc.size(), humanable, out_scale_multiplier);
  //! cout << "zc: \n" << zc << ENDL;
  tlog_info << "mystique_matmul_fp_with_proof elapse (us):  " << timer_proof.ns_elapse() / 1000
           << " us, proof accumulate: " << matmul_proof_time / 1000 << " us." << ENDL;
  tlog_info << "Mystique Matmul ok. <---- " << zk_party_id << ",  m, k, n: " << m << ", " << k
           << ", " << n;
  return 0;
}

int MystiqueOpsImpl::Square(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "-----> Mystique Square";
  // Not the most efficient one, but works fine.
  // vector<string> b = a;
  int ret = this->Mul(a, a, output, attr_info);
  tlog_debug << "Mystique Square ok. <----";
  return ret;
}

int MystiqueOpsImpl::Negative(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  int size = a.size();
  int a_scale = 1;
  int output_scale_multiplier = 1; 
#if LOCAL_SIMULATE
  vector<ZkUint64> a_fps(size);
  a_scale = convert_string_to_mac(a.data(), (uint64_t*)a_fps.data(), a.size());

  for (int i = 0; i < size; ++i) {
    a_fps[i] = mystique_plain_neg(a_fps[i]);
  }

  convert_mac_to_string(a_fps, output, humanable, a_scale);
  tlog_debug << "Mystique Negate ok. <----";
  return 0;
#endif

  vector<ZkIntFp> a_zks(size);
  a_scale = convert_string_to_mac(a, a_zks);

  for (int i = 0; i < size; ++i) {
    ZkIntFp::zk_fp_neg_this(a_zks[i], this->party);
  }

  convert_mac_to_string(a_zks, output, humanable, a_scale);
  tlog_debug << "Mystique Negate ok. <----";
  return 0;
}

int MystiqueOpsImpl::Sub(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "-----> Mystique Sub";
  int size = a.size();
  int a_scale = 1;
  int b_scale = 1;
  int output_scale_multiplier = 1; 
#if LOCAL_SIMULATE
  vector<uint64_t> fp_a(size);
  vector<uint64_t> fp_b(size);
  vector<uint64_t> fp_c(size);
  a_scale = convert_string_to_mac(a.data(), fp_a.data(), size);
  b_scale = convert_string_to_mac(b.data(), fp_b.data(), b.size());
  for(int i =0; i < size; ++i) {
    if(a_scale == b_scale && a_scale == 1) {
    fp_c[i] = mod(PR + fp_a[i] - fp_b[i]);
    } else if(a_scale == 1 && b_scale ==2) {
      fp_c[i] = mod(PR + fp_a[i] * _FLOAT_SCALE_ - fp_b[i]);
      output_scale_multiplier = 2;
    } else if(a_scale == 2 && b_scale ==1) {
      fp_c[i] = mod(PR + fp_a[i] - fp_b[i] * _FLOAT_SCALE_ );
      output_scale_multiplier = 2;
    }
  }
  
  #if DEBUG_MODE
  for(int i = 0; i < size; ++i) {
    tlog_debug << i <<"-th sub : " << fp_a[i] << " + " << fp_b[i] << "-->" << fp_c[i];
  }
  #endif

  convert_mac_to_string(fp_c.data(), output, size, humanable, output_scale_multiplier);
  return 0;
#endif

  vector<ZkIntFp> a_zks(size), b_zks(size), c_zks(size);

  a_scale = convert_string_to_mac(a, a_zks);
  b_scale = convert_string_to_mac(b, b_zks);
  // todo: refine code
  if (a_scale != b_scale && _OUTPUT_WITH_HIGH_SCALE) {
    tlog_info << "Adjust scale for Op Sub:" << a_scale << " VS " << b_scale;
  }
  bool need_scale_align = (a_scale == 2 || b_scale ==2);
  for (int i = 0; i < size; ++i) {
    // adjust scale 
    if (a_scale != b_scale && _OUTPUT_WITH_HIGH_SCALE) {
      if(a_scale == 2 && b_scale == 1) {
        b_zks[i] = b_zks[i] * _FLOAT_SCALE_;
      } else if (a_scale == 1 && b_scale == 2) {
        a_zks[i] = a_zks[i] * _FLOAT_SCALE_;
      } else {
        throw std::runtime_error("Wrong decoded scale_multiplier!" + to_string(a_scale) + " VS " + to_string(b_scale));
      }
    }
    c_zks[i].value = (a_zks[i] - b_zks[i]).value;
  }

  if (need_scale_align) {
    output_scale_multiplier = 2;
  }
  convert_mac_to_string(c_zks, output, humanable, output_scale_multiplier);
  tlog_debug << "Mystique Sub ok. <----";
  return 0;
}

int MystiqueOpsImpl::Mul(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "-----> Mystique Mul";
  bool rh_is_const = false;
  if (attr_info && attr_info->count("rh_is_const") > 0 && attr_info->at("rh_is_const") == "1") {
    rh_is_const = true;
  }
  bool need_broadcast = false;
  if (attr_info && attr_info->count("need_broadcast") > 0 && attr_info->at("need_broadcast") == "1") {
    need_broadcast = true;
  }

  int size = a.size();
  output.clear();
  output.resize(size);
  int output_scale_multiplier = 1; 
  int b_size = b.size();
  int ret = 0;
  int a_scale = 1;
  int b_scale = 1; 
#if LOCAL_SIMULATE
  vector<uint64_t> fp_a(size);
  vector<uint64_t> fp_c(size);
  vector<uint64_t> fp_b(b_size);
  a_scale = convert_string_to_mac(a.data(), fp_a.data(), size);
  b_scale = convert_string_to_mac(b.data(), fp_b.data(), b_size);
  vector<uint64_t> fp_expanded_b(size);
  if(need_broadcast) {
    tlog_info << "expand input b in MUL";
    assert(size % b_size == 0);
    for(int i = 0; i< size; ++i) {
      fp_expanded_b[i] = fp_b[i % b_size];
    }
  } else {
    tlog_info << "no need to expend in MUL";
    assert(size == b_size);
    fp_expanded_b.swap(fp_b);
  }

  if (rh_is_const) {
  } else {
    if(a_scale == 1 && b_scale == 1 && _OUTPUT_WITH_HIGH_SCALE) {
      tlog_info << "SCALE DOUBLE in Mul!";
      for (auto i = 0; i < size; i++) {
        fp_c[i] = mult_mod(fp_a[i], fp_expanded_b[i]);
        tlog_debug << i << "-th " << fp_a[i] << " * " << fp_expanded_b[i] << "-->" << fp_c[i];
      }
      output_scale_multiplier = 2;
    } else {
      if((a_scale > 1 || b_scale > 1) && _OUTPUT_WITH_HIGH_SCALE) {
        tlog_warn << "SCALE BACK in Mul :" << a_scale << " VS " << b_scale;
      }
      ret = _Inner_Mul(fp_a, fp_expanded_b, fp_c, a_scale, b_scale);
    }
  }
  #if DEBUG_MODE

  for(int i =0; i< size; ++i) {
    tlog_info << "zk Mul: " << fp_a[i] << " * " << fp_expanded_b[i] << "-->" << fp_c[i] << ENDL;
  }
  #endif

  convert_mac_to_string(fp_c.data(), output, size, humanable, output_scale_multiplier);
  return 0;
#endif


  vector<ZkIntFp> a_zks(size);
  vector<ZkIntFp> b_zks(b_size);
  vector<ZkIntFp> c_zks(size);
  a_scale = convert_string_to_mac(a, a_zks);
  if (rh_is_const) {
    tlog_info << "Input in Mul set rh_is_const attr!";
    vector<double> db(b_size);
    convert_string_to_double(b, db.data(), true); // readable double string
    // print_vec(db, 20, "Debug const B:");
    vector<uint64_t> fb(b_size);
    zk_encode_fp(db.data(), b_size, fb.data());
    // We allow outputs to be un-truncated with double-scale!
    // This can be tricky, we may remove this in the future.
    if(a_scale == 1 && _OUTPUT_WITH_HIGH_SCALE) {
      tlog_info << "SCALE DOUBLE in Mul with rh_is_const!";
      for (auto i = 0; i < size; i++) {
        if(!need_broadcast) {
          c_zks[i] = a_zks[i] * fb[i];
        } else {
          tlog_debug << "debug broadcast stub A in rh_is_const!";
          int d = i % b_size;
          c_zks[i] = a_zks[i] * fb[d];
        }
      }
      output_scale_multiplier = 2;
    } else {
        if(a_scale > 1 && _OUTPUT_WITH_HIGH_SCALE) {
          tlog_warn << "SCALE BACK in Mul with rh_is_const!" << a_scale;
      }
      ret = _Inner_Mul(a_zks, fb, c_zks, a_scale, need_broadcast);
    }
  } else {
    b_scale = convert_string_to_mac(b, b_zks);
    if(a_scale == 1 && b_scale == 1) {
      if (_OUTPUT_WITH_HIGH_SCALE) {
        tlog_info << "SCALE DOUBLE in Mul!";
        for (auto i = 0; i < size; i++) {
          if(!need_broadcast) {
            c_zks[i] = a_zks[i] * b_zks[i];
          } else {
            tlog_debug << "debug broadcast stub B in both scaled case!";
            int d = i % b_size;
            c_zks[i] = a_zks[i] * b_zks[d];   
          }
        }
        output_scale_multiplier = 2;
      } else {
        _Inner_Mul(a_zks, b_zks, c_zks);
        output_scale_multiplier = 1;
      }
    } else {
      if((a_scale > 1 || b_scale > 1) && _OUTPUT_WITH_HIGH_SCALE) {
        tlog_warn << "SCALE BACK in Mul :" << a_scale << " VS " << b_scale;
      }
      ret = _Inner_Mul(a_zks, b_zks, c_zks, a_scale + b_scale -1, need_broadcast);
    }
  }
  convert_mac_to_string(c_zks, output, humanable, output_scale_multiplier);
  
  #if DEBUG_MODE
  for(int i =0; i< size; ++i) {
    tlog_info << "zk Mul: " << a_zks[i].value << " * " << b_zks[i%b_size].value << "-->" << c_zks[i].value << ENDL;
  }
  #endif

  tlog_debug << "Mystique Mul ok. <----";
  return ret;
}

int MystiqueOpsImpl::Mean(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  size_t rows, cols;
  if (attr_info && attr_info->count("rows") > 0 && attr_info->count("cols") > 0) {
    rows = (size_t)std::stoul(attr_info->at("rows"));
    cols = (size_t)std::stoul(attr_info->at("cols"));
    if (rows * cols != a.size())
      throw std::runtime_error("rows*cols != input size");
  } else {
    tlog_error << "please fill rows,cols for mean(x, rows, cols) ";
    return -1;
  }

  tlog_info << "Mystique Mean ----> rows: " << rows << ", cols: " << cols;
  int a_scale = 1;
#if LOCAL_SIMULATE
  vector<ZkUint64> a_fps(a.size());
  a_scale = convert_string_to_mac(a.data(), a_fps.data(), a.size());

  vector<ZkUint64> result_fps(rows, 0);
  for (size_t i = 0; i < rows; i++) {
    result_fps[i] = std::accumulate(a_fps.begin() + i*cols, a_fps.begin() + (i+1)*cols, ZkUint64(0));
  }

  // local CONST MUL
  vector<float> inv_n_float(1, float(1.0) / cols);
  vector<ZkUint64> inv_fp(1);
  zk_encode_fp(inv_n_float.data(), 1, (uint64_t*)inv_fp.data());
  vector<uint64_t> c_fps(rows);
  for (auto i = 0; i < rows; ++i) 
    c_fps[i] = (result_fps[i] * inv_fp[0]).value;
  
  vector<float> float_c(rows);
  zk_decode_(c_fps, float_c, (a_scale+1));
  zk_encode(float_c, c_fps);
  // tlog_info << "inv_fps: " << inv_fp[0].value << ", result: " << result_fps[i].value << ", fl: " << fl << " cast_float: " << (float((int64_t)(c_fps[i]))) << ", c_fps: " << c_fps[i];
 
  convert_mac_to_string(c_fps, output, humanable, a_scale);
  tlog_info << "Mystique Mean ok. <---- a_scale: " << a_scale;
  return 0;
#endif//

  vector<ZkIntFp> a_zks(a.size());
  a_scale = convert_string_to_mac(a, a_zks);

  vector<ZkIntFp> results(rows, ZkIntFp(uint64_t(0), PUBLIC));
  for (size_t i = 0; i < rows; i++) {
    results[i] = std::accumulate(a_zks.begin() + i*cols, a_zks.begin() + (i+1)*cols, ZkIntFp(uint64_t(0), PUBLIC));
  }

  // local CONST MUL
  vector<double> INV_N_double(1, double(1.0) / cols);
  vector<uint64_t> INV_N(1);
  zk_encode_fp(INV_N_double.data(), 1, INV_N.data());
  
  vector<ZkIntFp> c_zks(rows);
  for (auto i = 0; i < rows; ++i) {
    c_zks[i] = results[i] * INV_N[0];
  }

  // truncate!
  int size =rows;
  vector<Integer> bc(size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(bc.data(), (ZkIntFp*)c_zks.data(), size);
  float2int_counter += size;
  int2float_counter += size;
  for (size_t i = 0; i < bc.size(); i++) {
    SimpleTimer timer_b2f;
    Float fl = std::move(Int62ToFloat(bc[i], a_scale*ZK_F));
    int2float_elapsed += timer_b2f.ns_elapse();
    SimpleTimer timer_f2b;
    bc[i] = FloatToInt62(fl, 0);
    float2int_elapsed += timer_f2b.ns_elapse();
    // tlog_info << "inv_fps: " << INV_N[0] << ", result: " << (uint64_t)(results[i].value >> 64);//<< ", fl: " << fl << ", c_fps: " << c_fps[i];
  }

  // sync_zk_bool<BoolIO<ZKNetIO>>();
  bool2arith<BoolIO<ZKNetIO>>(c_zks.data(), bc.data(), bc.size());
  sync_zk_bool<BoolIO<ZKNetIO>>();
  
  convert_mac_to_string(c_zks, output);
  // temp for debuging
  // vector<string> scale_c_str(rows);
  // convert_mac_to_string(c_zks, scale_c_str);
  // vector<double> input_a(size);
  // vector<double> scaled_res(size);
  // vector<double> debug_res(size);

  // this->Reveal(a, input_a);
  // this->Reveal(scale_c_str, scaled_res);
  // this->Reveal(output, debug_res);
  // print_vec(input_a, 20, "Debug revealed Input plain A:");
  // print_vec(scaled_res, 20, "Debug revealed scaled Mean plain B:");
  // print_vec(debug_res, 20, "Debug revealed Mean plain:");

  tlog_info << "Mystique Mean ok. <---- ";
  return 0;
}

int MystiqueOpsImpl::Max(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  size_t rows, cols;
  if (attr_info && attr_info->count("rows") > 0 && attr_info->count("cols") > 0) {
    rows = (size_t)std::stoul(attr_info->at("rows"));
    cols = (size_t)std::stoul(attr_info->at("cols"));
  } else {
    tlog_error << "please fill rows,cols for mystique_max(x, rows, cols) ";
    return -1;
  }
  int a_scale = 0;
  tlog_debug << "Mystique Max ----> rows: " << rows << ", cols: " << cols;

#if LOCAL_SIMULATE
  vector<ZkUint64> a_fps(a.size());
  a_scale = convert_string_to_mac(a.data(), a_fps.data(), a.size(), humanable);

  vector<ZkUint64> result_fps;
  mystique_plain_max_matrix(a_fps, rows, cols, result_fps);

  convert_mac_to_string(result_fps, output, humanable, a_scale);
  tlog_debug << "Mystique Max ok. <----  ";
  return 0;
#endif

  vector<ZkIntFp> a_zks(a.size());
  a_scale = convert_string_to_mac(a, a_zks);

  vector<ZkIntFp> results;
  mystique_max_matrix(a_zks, rows, cols, results);

  convert_mac_to_string(results, output, humanable, a_scale);
  tlog_debug << "Mystique Max ok. <----  ";
  return 0;
}

int MystiqueOpsImpl::Relu(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  int size = a.size();
  
  int scale_count = 1;
  
  tlog_info << "-----> Mystique Relu, scale count: " << scale_count << " element size: " << a.size();

#if LOCAL_SIMULATE
  // JJ
  #if 1
  vector<uint64_t> fp_a(size);
  vector<uint64_t> fp_c(size);
  int a_scale = convert_string_to_mac(a.data(), fp_a.data(), size);
    vector<float> float_a(size);
  zk_decode_(fp_a, float_a, a_scale);
  for(int i = 0; i < size; ++i) {
    if( float_a[i] <= 0.0) {
      float_a[i] = float(0.0);
    }
  }
  zk_encode_float32(float_a, fp_c);
  convert_mac_to_string(fp_c.data(), output, size, humanable, 1);
  return 0;
  // GF
  #else 
  vector<ZkUint64> a_fps(size), c_fps(size);
  scale_count = convert_string_to_mac(a.data(), a_fps.data(), a.size());
  mystique_plain_relu(a_fps, c_fps, scale_count);

  convert_mac_to_string(c_fps, output);
  tlog_debug << "Mystique Relu ok. <----";
  return 0;
  #endif
#endif

  vector<ZkIntFp> a_zks(size), c_zks(size);
  scale_count = convert_string_to_mac(a, a_zks);
  mystique_relu(a_zks, c_zks, scale_count);

  convert_mac_to_string(c_zks, output);
  tlog_debug << "Mystique Relu ok. <----";
  return 0;
}

int MystiqueOpsImpl::Div(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "-----> Mystique Div";

  bool rh_is_const = false;
  size_t size = a.size();
  if (attr_info && attr_info->count("rh_is_const") > 0 && attr_info->at("rh_is_const") == "1") {
    rh_is_const = true;
  }

  if (rh_is_const) {
    tlog_debug << "Input in Div set rh_is_const attr!";
    size = b.size();
    vector<double> db(size);
    vector<string> inverse_b_str(size);
    convert_string_to_double(b, db.data(), true);
    for (size_t i = 0; i < size; i++) {
#if !use_literal_value_binary_version
      inverse_b_str[i] = std::move(string(sizeof(double) + 1, '$'));
      double inverse = 1.0 / db[i];
      memcpy((char*)inverse_b_str[i].data(), &inverse, sizeof(inverse));
#else
      inverse_b_str[i] = std::move(std::to_string(1.0 / db[i]));
#endif
    }

    attr_type mul_attr;
    mul_attr["rh_is_const"] = "1";
    return Mul(a, inverse_b_str, output, &mul_attr);
  }

#if 0
  // Step 1: calc Invert b result
  vector<ZkIntFp> b_zks(size);
  vector<ZkIntFp> b_invert_zks(size);
  convert_string_to_mac(b, b_zks);
  int ret = _Inner_Invert(b_zks, b_invert_zks);

  // Step 2: calc a * b_invert => c
  vector<ZkIntFp> a_zks(size);
  vector<ZkIntFp> c_zks(size);
  convert_string_to_mac(a, a_zks);
  _Inner_Mul(a_zks, b_invert_zks, c_zks);

  // Step 3: Convert mac to string
  convert_mac_to_string(c_zks, output);

#else
  vector<ZkIntFp> a_zks(size);
  vector<ZkIntFp> b_zks(size);
  vector<ZkIntFp> c_zks(size);
  convert_string_to_mac(a, a_zks);
  convert_string_to_mac(b, b_zks);
  bool ret = _Inner_Div(a_zks, b_zks, c_zks);
  convert_mac_to_string(c_zks, output);
#endif

  tlog_debug << "Mystique Div ok. <----";
  return ret;
}

int MystiqueOpsImpl::Truediv(
  const vector<string>& a,
  const vector<string>& b,
  vector<string>& output,
  const attr_type* attr_info) {
  int ret = Div(a, b, output, attr_info);
  return ret;
}

int MystiqueOpsImpl::Sqrt(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "calling MystiqueOpsImpl::Sqrt string" << ENDL;
  int size = a.size();
  output.clear();
  output.resize(size);
  vector<ZkIntFp> a_zks(size);
  vector<ZkIntFp> res_zks(size);

  convert_string_to_mac(a, a_zks);
  _Inner_Sqrt(a_zks, res_zks);
  convert_mac_to_string(a_zks, output);

  tlog_debug << "Mystique Sqrt ok. <----";
  return 0;

}

// get 1/\sqrt{a}
int MystiqueOpsImpl::Rsqrt(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "calling MystiqueOpsImpl::Rsqrt string" << ENDL;

  int size = a.size();
  output.clear();
  output.resize(size);
  int a_scale  = 1;
#if LOCAL_SIMULATE
  tlog_info << "calling MystiqueOpsImpl::Rsqrt Local simulation!" << ENDL;
  vector<uint64_t> fp_a(size);
  vector<uint64_t> fp_c(size);
  a_scale = convert_string_to_mac(a.data(), fp_a.data(), size);
  
  vector<float> float_a(size);
  zk_decode_(fp_a, float_a, a_scale);
  vector<float> float_c(size);
  for(int i =0; i < size; ++i) {
    float_c[i] = float(1.0)/sqrt(float_a[i]);
  }
  zk_encode_float32(float_c, fp_c);
  #if DEBUG_MODE
  print_vec(fp_a, 20, "debug sim Rsqrt input:");
  print_vec(fp_c, 20, "debug sim Rsqrt result:");
  #endif

  convert_mac_to_string(fp_c.data(), output, size, humanable);
  return 0;
#endif

  vector<ZkIntFp> a_zks(size);
  vector<ZkIntFp> c_zks(size);
  convert_string_to_mac(a, a_zks);
  int gate_num = 0;
  sync_zk_bool<BoolIO<ZKNetIO>>();
  tlog_debug << "DEBUG STUB After sync_zk_bool" << ENDL;
  vector<Integer> bin_a(size);
  arith2bool<BoolIO<ZKNetIO>>(bin_a.data(), (IntFp*)a_zks.data(), size);
  tlog_debug << "DEBUG STUB 3: arith2bool" << ENDL;
  vector<Float> zk_float(size, (0.0, PUBLIC));
  Float CONST_ONE(1.0, PUBLIC); 
  float2int_counter += size;
  int2float_counter += size;
  for(int i = 0; i < size; ++i) {
    SimpleTimer timer_b2f;
    zk_float[i] = Int62ToFloat(bin_a[i], ZK_F);
    int2float_elapsed += timer_b2f.ns_elapse();
    zk_float[i] = zk_float[i].sqrt();
    zk_float[i] = CONST_ONE / zk_float[i];
    SimpleTimer timer_f2b;
    bin_a[i] = FloatToInt62(zk_float[i], ZK_F);
    float2int_elapsed += timer_f2b.ns_elapse();
  }
  tlog_debug << "Add Gate num for RSqrt: " << CircuitExecution::circ_exec->num_and() - gate_num << ENDL;
  bool2arith<BoolIO<ZKNetIO>>(c_zks.data(), bin_a.data(), size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  tlog_debug << "DEBUG STUB 3: bool2arith" << ENDL;
  convert_mac_to_string(c_zks, output);
  #if DEBUG_MODE
  for(int i =0; i< size; ++i) {
    tlog_info << "zk Qsqrt: " << a_zks[i].value << "-->" << c_zks[i].value << ENDL;
  }
  #endif
  
  return 0;
  // int size = a.size();
  // output.clear();
  // output.resize(size);
  // vector<ZkIntFp> a_zks(size);
  // vector<__uint128_t> c_zks(size);
  // convert_string_to_mac(a, a_zks);

  // vector<double> plain_a(size);
  // vector<double> plain_c(size);
  // vector<uint64_t> field_elements(size);
  // // Step 1: get float 1/sqrt(a) result
  // if(zk_party_id == ALICE) {
  //   zk_decode(a_zks, plain_a);
  //   for(auto i = 0; i < size; ++i) {
  //     // If parameter is illegal, set result to 0 for now.
  //     if (abs(plain_a[i]) < MINIMUN) {
  //       plain_c[i] = 0.0;
  //     } else {
  //       plain_c[i] = 1.0 / sqrt(plain_a[i]);
  //     }
  //     tlog_debug << i << "-th:" << plain_a[i] << "=>" << plain_c[i] << ENDL;
  //   }
  //   zk_encode(plain_c, field_elements);
  // }

  // // step 3: prover input the new truncated value.
  // vector<ZkIntFp> c_prime(size);
  // vector<ZkIntFp> res_proof(size);
  // int ret = 0;
  // for(int i = 0; i < size; ++i) {
  // 	c_prime[i] = ZkIntFp(field_elements[i], ALICE);
  //   convert_mac_to_string(c_prime[i].value, output[i]);
  //   tlog_debug << "Float Rsqrt: " << plain_c[i] << "-->" << field_elements[i] << " --> " << output[i] << ENDL;
  //   // 4.1: prepare data for the result proof.
  //   //    We perform the (1 / sqrt(x)) ^2 * x in ZK style so that the error does not propogate.
  //   #if ZK_INNER_CHECK
  //   res_proof[i] = c_prime[i] * c_prime[i];
  //   res_proof[i] = res_proof[i] *  a_zks[i];
  //   tlog_debug << "get "<< i << "-th ZK proof Done" << ENDL;
  //   #endif
  // }

  // #if !ZK_INNER_CHECK
  //   return ret;
  // #else

  // // sync_zk_bool<BoolIO<ZKNetIO>>();

  // // 4.2: prove the result is correct.
  // vector<double> ONE(size, (1.0 * _TWO_FLOAT_SCALE_));
  // vector<ZkIntFp> ZK_SCALED_ONE(size);
  // PublicInput(PUBLIC, ONE, ZK_SCALED_ONE);
  // _inner_proof_and_check(res_proof, ZK_SCALED_ONE, _TWO_FLOAT_SCALE_ * _FLOAT_SCALE_);
  // tlog_debug << "Mystique Rsqrt ok. <----";
  // return ret;
  // #endif
}

int MystiqueOpsImpl::Invert(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "-----> Mystique Invert" << ENDL;

  int size = a.size();
  output.clear();
  output.resize(size);
  vector<ZkIntFp> a_zks(size);
  vector<ZkIntFp> res_zks(size);

  convert_string_to_mac(a, a_zks);
  int ret = _Inner_Invert(a_zks, res_zks);
  convert_mac_to_string(res_zks, output);

  return ret; 
}

int MystiqueOpsImpl::Exp(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "-----> Mystique Exp" << ENDL;

#if USE_EXP_APPROX
  int ret = Exp_Approx(a, output, attr_info);
#else 
  int ret = Exp_Exact(a, output, attr_info);
#endif
  tlog_debug << "Mystique Exp ok" << ENDL;
  return ret;
}

int MystiqueOpsImpl::Reveal(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "calling MystiqueOpsImpl::Reveal string" << ENDL;
  int vec_size = a.size();
  output.clear();
  output.resize(vec_size);
  vector<double> plain_val(vec_size);


  int ret = this->Reveal(a, plain_val, attr_info);

  if (ret < 0) {
    return ret;
  }
  // encoded as string.
  for (int i = 0; i < vec_size; ++i) {
    output[i] = std::to_string(plain_val[i]);
  }
  return 0;
}

int MystiqueOpsImpl::Reveal(
  const vector<string>& a,
  vector<double>& output,
  const attr_type* attr_info) {
  tlog_debug << "calling MystiqueOpsImpl::Reveal[double]" << ENDL;

  int vec_size = a.size();
  output.resize(vec_size);
  int a_scale = 1;
#if LOCAL_SIMULATE
  vector<uint64_t> fp_a(vec_size);
  // vector<double> float_a(vec_size);
  vector<float> float_a(vec_size);
  a_scale = convert_string_to_mac(a.data(), fp_a.data(), vec_size);
  tlog_debug << "reveal input scale:" << a_scale << ":" << fp_a[0];
  zk_decode_(fp_a, float_a, a_scale);
  for(int i = 0; i < vec_size; ++i) {
    output[i] = double(float_a[i]);
  }


  return 0;
#endif

  std::vector<ZkIntFp> inner_a(vec_size);
  a_scale = convert_string_to_mac(a, inner_a);

#if 0
  // just for interface adapter, not optimal for now.
  uint64_t *all_x = new uint64_t[vec_size];
  IntFp *mat_c = new IntFp[vec_size];
  for (int i = 0; i < vec_size; ++i) {
    mat_c[i] = inner_a[i];
    // extract x and send it.
    all_x[i] = uint64_t(inner_a[i].value >> 64);
  }
  if(zk_party_id == ALICE) {
    zk_ios[0]->send_data(all_x, vec_size*sizeof(uint64_t));
    // in the current version, it seems that when calling the send_data, we should 
    //    run flush to make sure they sync.
    zk_ios[0]->flush();
  } else {
    zk_ios[0]->recv_data(all_x, vec_size*sizeof(uint64_t));
  }

  vector<uint64_t> output_x(vec_size);
  for (int i = 0; i < vec_size; ++i) {
    output_x[i] = all_x[i];
  }

  bool status = batch_reveal_check(mat_c, all_x, vec_size);
  tlog_info << "succeed in verifying zk!! check: " << status;
  delete[] all_x;
  delete[] mat_c;
#else
  vector<uint64_t> output_x(vec_size);
  batch_reveal((IntFp*)(inner_a.data()), output_x.data(), vec_size);
  tlog_info << "succeed in verifying zk!!";
#endif
  zk_decode_(output_x, output, a_scale);
  return 0;
  //   int vec_size = a.size();
  //   // in ZK, the output is also an input parameter,constisting of the expected value?
  //   // output.resize(vec_size);

  //   std::vector<__uint128_t> inner_a(vec_size);
  //   convert_string_to_mac(a,inner_a, true, this);

  //   vector<uint64_t> field_elements(vec_size);
  //   zk_encode(output, field_elements);
  //   uint64_t* expected = new uint64_t[vec_size];

  //   IntFp *mat_c = new IntFp[vec_size];
  //   for (int i = 0; i < vec_size; ++i) {
  //     mat_c[i].value = inner_a[i];
  //     cout << "expected[" << i << "]:" << field_elements[i] << ENDL;
  //     cout << "curr inner X:" << inner_a[i] << ENDL;
  //     expected[i] = field_elements[i];
  //   }

  //   if (zk_party_id == ALICE) {
  //     tlog_info << "Wow, I'm prover, and am exposing the value to show you now!";
  //     output.clear();
  //     output.resize(a.size());
  //     zk_decode(inner_a, output);

  //   }

  //   bool ret = batch_reveal_check(mat_c, expected, vec_size);
  //   if(ret) {
  //     tlog_info << "succeed in verifying zk!!" << ENDL;
  //     return 0;
  //   } else {
  //     return 1;
  //   }
}

int MystiqueOpsImpl::Softmax(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_debug << "-----> Mystique Softmax";
  assert(attr_info);
  int size = a.size();
  output.clear();
  output.resize(size);
  auto start = clock_start();
  // get rows & cols
  int64_t rows, cols;
  if (attr_info->count("rows") > 0 && attr_info->count("cols") > 0) {
    rows = std::stol(attr_info->at("rows"));
    cols = std::stol(attr_info->at("cols"));
    tlog_debug << "softmax rows:" << rows << ", cols:" << cols << ENDL;
  } else {
    throw runtime_error("ERROR! Softmax op must fill rows & cols attributes.");
    return -1;
  }

#if LOCAL_SIMULATE
  {
    vector<uint64_t> fp_a(size);
    int a_scale = convert_string_to_mac(a.data(), fp_a.data(), size);
    tlog_info << "Softmax input scale: " << a_scale;
    vector<float> float_a(size);
    zk_decode_(fp_a, float_a, a_scale);

    for (int i = 0; i < rows; i++) {
      vector<float> rows_ft_temp(cols, 0.0);
      auto iter_max = std::max_element((float*)&(float_a[i * cols]), (float*)&(float_a[i*cols])+cols);
      float input_max = *iter_max;
      float sum(0.0);

      for (int j = 0; j < cols; j++) {
        rows_ft_temp[j] = std::exp(float_a[i * cols + j] - input_max);
        sum = sum + rows_ft_temp[j];
      }

      // // log(sum)
      // float sum_final(0.0);
      // for (int j = 0; j < cols; j++) {
      //   rows_ft_temp[j] = std::exp(float_a[i * cols + j] - input_max - std::log(sum));
      //   sum_final = sum_final + rows_ft_temp[j];
      // }

      for (int j = 0; j < cols; j++) {
        rows_ft_temp[j] = rows_ft_temp[j] / sum;//sum_final;
      }
      vector<uint64_t> res_int(cols);
      // zk_encode(rows_ft_temp, res_int);
      zk_encode_float32(rows_ft_temp, res_int);

      vector<string> rows_temp(cols);
      convert_mac_to_string(res_int.data(), rows_temp, cols);
      std::copy(rows_temp.begin(), rows_temp.end(), output.begin() + (i * cols));
    }
  }
  return 0;
#endif

  //Arithmetic to Bool
  vector<ZkIntFp> a_zks(size);
  vector<Integer> a_int(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  int scale_num = convert_string_to_mac(a, a_zks);
  if (scale_num > 2 && _OUTPUT_WITH_HIGH_SCALE) {
    throw std::runtime_error("Wrong input scale for Softmax input:" + to_string(scale_num));
  }

  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(a_int.data(), (IntFp*)a_zks.data(), size);

  // softmax operation(row major)
  vector<Float> rows_ft_temp(cols, (0.0, PUBLIC));
  vector<Integer> res_int(cols);
  vector<ZkIntFp> res_zks(cols);
  vector<string> rows_temp(cols);

  for (int i = 0; i < rows; i++) {
    // calc exp(a[ix]) & sum(exp(a[ix]...))
    Float sum(0., PUBLIC);
    int2float_counter += cols;
    for (int j = 0; j < cols; j++) {
      SimpleTimer timer_b2f;
      rows_ft_temp[j] = Int62ToFloat(a_int[i * cols + j], ZK_F * scale_num);
      int2float_elapsed += timer_b2f.ns_elapse();
    }

    // get max
    Float input_max = rows_ft_temp[0];
    for (size_t j = 1; j < cols; j++)
    {
      input_max = If(input_max.less_than(rows_ft_temp[j]), rows_ft_temp[j], input_max);
    }

    for (int j = 0; j < cols; j++) {
      rows_ft_temp[j] = (rows_ft_temp[j]-input_max).exp();
      sum = sum + rows_ft_temp[j];
    }

    float2int_counter += cols;
    // calc output[x0] = exp(a[x0]) / sum
    for (int j = 0; j < cols; j++) {
      rows_ft_temp[j] = rows_ft_temp[j] / sum;
      SimpleTimer timer_f2b;
      res_int[j] = FloatToInt62(rows_ft_temp[j], ZK_F);
      float2int_elapsed += timer_f2b.ns_elapse();
    }

    // Bool to Arithmetic
    bool2arith<BoolIO<ZKNetIO>>((IntFp*)res_zks.data(), res_int.data(), cols);
    sync_zk_bool<BoolIO<ZKNetIO>>();
    convert_mac_to_string(res_zks, rows_temp);

    std::copy(rows_temp.begin(), rows_temp.end(), output.begin() + (i * cols));
  }

  auto switch_time = time_from(start);
  tlog_debug << "softmax ok, costing (us) " << switch_time << " for party" << zk_party_id << ENDL;
  tlog_debug << "-----> Mystique Softmax ok";
  return 0;
}

int MystiqueOpsImpl::Sigmoid(
  const vector<string>& a,
  vector<string>& output,
  const attr_type* attr_info) {
  tlog_info << "-----> Mystique Sigmoid";
  /*************************************
   * 1/(1+e^(-x))
   *************************************/

  size_t size = a.size();
  vector<ZkIntFp> a_zks(size);
  vector<Integer> a_int(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  int scale_num = convert_string_to_mac(a, a_zks);
  tlog_debug << "(sigmoid input)scale number:" << scale_num;

  // Arithmetic to Bool
  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(a_int.data(), (IntFp*)a_zks.data(), size);

  // Calc 1/(1+e^(-x))
  vector<Float> a_ft(size, (0.0, PUBLIC));
  Float ONE(1.0, PUBLIC);
  for (int i = 0; i < size; i++) {
    a_ft[i] = Int62ToFloat(a_int[i], ZK_F * scale_num);
    a_ft[i] = -a_ft[i];
    a_ft[i] = a_ft[i].exp();
    a_ft[i] = a_ft[i] + ONE;
    a_ft[i] = ONE / a_ft[i];
    a_int[i] = FloatToInt62(a_ft[i], ZK_F);
  }

  // Bool to Arithmetic
  bool2arith<BoolIO<ZKNetIO>>((IntFp*)a_zks.data(), a_int.data(), size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  convert_mac_to_string(a_zks, output);

  tlog_info << "-----> Mystique Sigmoid ok";
  return 0;
}

int MystiqueOpsImpl::Exp_Approx(const vector<string>& a, vector<string>& output, const attr_type* attr_info) {
  /*****************************************************************
  Approximates the exponential function using a limit approximation:
  exp(x) = (1 + x / n) ^ n
  Here we compute exp by choosing n = 2 ** D for some large d equal to
  iterations. We then compute (1 + x / n) once and square `D` times.
  Here we set `D` equal (8 or 16).
  note: Need to optimize.
******************************************************************/
  const unsigned int D = 8;
  int size = a.size();
  output.clear();
  output.resize(size);
  vector<string> a_prime_str(size);

  double inverse = 1.0 / pow(2, D);
#if !use_literal_value_binary_version
  string inverse_two_pow_d(sizeof(double) + 1, '$');
  memcpy((char*)inverse_two_pow_d.data(), &inverse, sizeof(inverse));
#else
  string inverse_two_pow_d = std::move(std::to_string(inverse));
#endif
  vector<string> nInv_str(size, inverse_two_pow_d); // optimized by Private * Public
  vector<ZkIntFp> c_zks(size);
  vector<ZkIntFp> a_prime_zks(size);
  ZkIntFp one_zks(1.0, PUBLIC);

  // step 1: a' = a * (1/n)
  attr_type attrs;
  attrs.insert(std::make_pair(string("rh_is_const"), string("1")));
  tlog_info << "-----> Mystique Exp to Mul_const ..." << ENDL;
  int nRet = Mul(a, nInv_str, a_prime_str, &attrs);
  assert(!nRet);

  // step 2: c = 1 + a'
  convert_string_to_mac(a_prime_str, a_prime_zks);
  for (int i = 0; i < size; i++) {
    c_zks[i] = a_prime_zks[i] + one_zks;
  }

  // step 3: c2 = c * c; c4 = c2 * c2; c8 = c4 * c4;... then c256 is then result value.
  vector<ZkIntFp> ir_res(size);
  for (int i = 0; i < D; i++) {
    nRet = _Inner_Mul(c_zks, c_zks, ir_res);
    assert(!nRet);
    c_zks = ir_res;
  }
  convert_mac_to_string(ir_res, output);

  return nRet;
}

int MystiqueOpsImpl::Exp_Exact(const vector<string>& a, vector<string>& output, const attr_type* attr_info) {
  int size = a.size();
  vector<ZkIntFp> a_zks(size);
  convert_string_to_mac(a, a_zks);

  // Step 1:FP to Integer
  vector<Integer> a_Int(size);
  vector<ZkIntFp> res_Int(size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(a_Int.data(), (IntFp*)a_zks.data(), size);

  // Step 2:calc exp
  float2int_counter += size;
  int2float_counter += size;
  Float ir_res(0., PUBLIC);
  for (auto i = 0; i < size; i++) {
    SimpleTimer timer_b2f;
    ir_res = Int62ToFloat(a_Int[i], ZK_F);
    int2float_elapsed += timer_b2f.ns_elapse();
    ir_res = ir_res.exp();
    SimpleTimer timer_f2b;
    a_Int[i] = FloatToInt62(ir_res, ZK_F);
    float2int_elapsed += timer_f2b.ns_elapse();
  }

  // Step 3: Bool to Arithmetic
  bool2arith<BoolIO<ZKNetIO>>((IntFp*)res_Int.data(), a_Int.data(), size); 
  sync_zk_bool<BoolIO<ZKNetIO>>();
  convert_mac_to_string(res_Int, output);

  return 0;
}

int MystiqueOpsImpl::_Inner_Invert(const vector<ZkIntFp>& a, vector<ZkIntFp>& output) {
  // Step 1: FP to Integer
  auto size = a.size();
  vector<Integer> a_Int(size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(a_Int.data(), (IntFp*)a.data(), size);

  // Step 2: calc invert
  Float ir_res(0., PUBLIC);
  Float one(1.0, PUBLIC);

  float2int_counter += size;
  int2float_counter += size;
  for (auto i = 0; i < size; i++) {
    // integer to float, and calc invert
    SimpleTimer timer_b2f;
    ir_res = Int62ToFloat(a_Int[i], ZK_F);
    int2float_elapsed += timer_b2f.ns_elapse();
    ir_res = (one / ir_res);
    SimpleTimer timer_f2b;
    a_Int[i] = FloatToInt62(ir_res, ZK_F);
    float2int_elapsed += timer_f2b.ns_elapse();
  }

  // Step 3: Bool to Arithmetic
  bool2arith<BoolIO<ZKNetIO>>((IntFp*)output.data(), a_Int.data(), size); 
  sync_zk_bool<BoolIO<ZKNetIO>>();
  return 0;
}

int MystiqueOpsImpl::_Inner_Mul(const vector<uint64_t>& a, const vector<uint64_t>& b, vector<uint64_t>& output, int a_scale, int b_scale) {
  
  auto size = a.size();
  vector<float> float_a(size);
  vector<float> float_b(size);
  vector<uint64_t> fp_c(size);
  vector<float> float_c(size);
  // very tricky!
  // case 1: normal truncation
  if(a_scale == 1 && b_scale == 1) {
    tlog_info << "sim truncation case 1 : 1 VS 1";
    for(int i = 0; i < size; ++i) {
      fp_c[i] = mult_mod(a[i], b[i]);
    }
    zk_decode_(fp_c, float_c);
    zk_encode_float32(float_c, output);
  } else if( (a_scale + b_scale) == 3) {
    tlog_info << "sim truncation case 2: " << to_string(a_scale) << " VS " << to_string(b_scale);
    for(int i = 0; i < size; ++i) {
      fp_c[i] = mult_mod(a[i], b[i]);
      tlog_debug << "Fp_res:" << fp_c[i];
    }
    zk_decode_(fp_c, float_c, a_scale + b_scale);
    zk_encode_float32(float_c, output);
  } else {
    tlog_info << "sim truncation case 3: 2 VS 2";
    zk_decode_(a, float_a, a_scale);
    zk_decode_(b, float_b, b_scale);
    
    for(int i =0; i < size; ++i) {
      float_c[i] = float_a[i] * float_b[i];
    }
    zk_encode_float32(float_c, output);
  }
  return 0;

}


template <typename T>
int MystiqueOpsImpl::_Inner_Mul(const vector<ZkIntFp>& a, const vector<T>& b, vector<ZkIntFp>& output, int extra_scale_multiplier, bool rhs_broadcast) {
  // assert(a.size() == b.size());
  // Step 1: a * b => c
  auto size = a.size();
  auto b_size = b.size();
  vector<ZkIntFp> c_zks(size);

  // to avoid overflow in Fp
  if (extra_scale_multiplier == 3) {
    tlog_warn << "debug both inputs double-scaled";

    // 1. We have to truncate both LHS and RHS to get Fp with one scale.
    sync_zk_bool<BoolIO<ZKNetIO>>();
    vector<Integer> a_bool(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    vector<Integer> b_bool(b_size, Integer(ZK_INT_LEN, 0, PUBLIC));
    vector<Integer> a_prime_res(size, Integer(ZK_INT_LEN, 0, PUBLIC));
    vector<Integer> b_prime_res(b_size, Integer(ZK_INT_LEN, 0, PUBLIC));
    vector<ZkIntFp> scaled_a(size);
    vector<ZkIntFp> scaled_b(b_size);    
    vector<ZkIntFp> double_scaled_c(size);
    arith2bool<BoolIO<ZKNetIO>>(a_bool.data(), (IntFp*)a.data(), size);
    arith2bool<BoolIO<ZKNetIO>>(b_bool.data(), (IntFp*)b.data(), b_size);
    Float a_float(0., PUBLIC);
    Float b_float(0., PUBLIC);
    float2int_counter += size + b_size;
    int2float_counter += size + b_size;
    tlog_info << "MUl for double scaled inputs, part 1: scale down both operands with size:" << size << " VS " << b_size;
    for (auto i = 0; i < size; i++) {
      SimpleTimer timer_b2f;
      a_float = Int62ToFloat(a_bool[i], ZK_F * 2);
      int2float_elapsed += timer_b2f.ns_elapse();
      SimpleTimer timer_f2b;
      a_prime_res[i] = FloatToInt62(a_float, ZK_F);
      float2int_elapsed += timer_f2b.ns_elapse();
    }
    for (auto i = 0; i < b_size; i++) {
      SimpleTimer timer_b2f;
      b_float = Int62ToFloat(b_bool[i], ZK_F * 2);
      int2float_elapsed += timer_b2f.ns_elapse();
      SimpleTimer timer_f2b;
      b_prime_res[i] = FloatToInt62(b_float, ZK_F);
      float2int_elapsed += timer_f2b.ns_elapse();
    }

    bool2arith<BoolIO<ZKNetIO>>((IntFp*)scaled_a.data(), a_prime_res.data(), size);
    bool2arith<BoolIO<ZKNetIO>>((IntFp*)scaled_b.data(), b_prime_res.data(), b_size);
    sync_zk_bool<BoolIO<ZKNetIO>>(); 
    // 2. Mul in Fp:
    for (auto i = 0; i < size; i++) {
      if(!rhs_broadcast) {
          double_scaled_c[i] = scaled_a[i] * scaled_b[i];
        } else {
          tlog_debug << "debug broadcast stub C in double scaled case!";
          int d = i % b_size;
          double_scaled_c[i] = scaled_a[i] * scaled_b[d];
        }
    }
    // 3. Truncate again:
    tlog_info << "MUl for double scaled inputs, part 2: scale down result with size:" << size;
    sync_zk_bool<BoolIO<ZKNetIO>>();
    arith2bool<BoolIO<ZKNetIO>>(a_bool.data(), (IntFp*)double_scaled_c.data(), size);
    float2int_counter += size;
    int2float_counter += size;
    for (auto i = 0; i < size; i++) {
      SimpleTimer timer_b2f;
      a_float = Int62ToFloat(a_bool[i], ZK_F * 2);
      int2float_elapsed += timer_b2f.ns_elapse();
      SimpleTimer timer_f2b;
      a_prime_res[i] = FloatToInt62(a_float, ZK_F);
      float2int_elapsed += timer_f2b.ns_elapse();
    }
    tlog_info << "begin b2a:" << size;
    bool2arith<BoolIO<ZKNetIO>>((IntFp*)output.data(), a_prime_res.data(), size);
    sync_zk_bool<BoolIO<ZKNetIO>>();
    return 0;
  }


  for (auto i = 0; i < size; i++) {
    if(!rhs_broadcast) {
      c_zks[i] = a[i] * b[i];
    } else {
      tlog_debug << "debug broadcast stub D !";
      int d = i % b_size;
      c_zks[i] = a[i] * b[d];
    }
  }

  // Step 2: result(c_zks) to bool
  sync_zk_bool<BoolIO<ZKNetIO>>();
  vector<Integer> c_res(size, Integer(ZK_INT_LEN, 0, PUBLIC));
  arith2bool<BoolIO<ZKNetIO>>(c_res.data(), (IntFp*)c_zks.data(), size);
  
  // Step 3: truncate
  Float ir_res(0., PUBLIC);
  
  float2int_counter += size;
  int2float_counter += size;
  tlog_warn << "extra Truncation with Float in _Mul_:" << extra_scale_multiplier << ENDL;
  for (auto i = 0; i < size; i++) {
    SimpleTimer timer_b2f;
    ir_res = Int62ToFloat(c_res[i], extra_scale_multiplier*ZK_F);
    int2float_elapsed += timer_b2f.ns_elapse();
    SimpleTimer timer_f2b;
    c_res[i] = FloatToInt62(ir_res, 0);
    float2int_elapsed += timer_f2b.ns_elapse();
  }

  // Step 4: Bool to Arithmetic
  bool2arith<BoolIO<ZKNetIO>>((IntFp*)output.data(), c_res.data(), size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  return 0;
}

int MystiqueOpsImpl::_Inner_Div(const vector<ZkIntFp>& a, const vector<ZkIntFp>& b, vector<ZkIntFp>& output) {
  assert(a.size() == b.size());

  // Step 1: arithmetic to bool for a, b
  auto size = a.size();
  vector<Integer> a_int(size);
  vector<Integer> b_int(size);
  vector<Integer> c_int(size);;
  sync_zk_bool<BoolIO<ZKNetIO>>();
  arith2bool<BoolIO<ZKNetIO>>(a_int.data(), (IntFp*)a.data(), size);
  arith2bool<BoolIO<ZKNetIO>>(b_int.data(), (IntFp*)b.data(), size);

  // Step 2: a/b => c(integer)
  Float a_f(0., PUBLIC);
  Float b_f(0., PUBLIC);
  Float ir_res(0., PUBLIC);

  float2int_counter += size;
  int2float_counter += 2 * size;
  for (auto i = 0; i < size; i++) {
    SimpleTimer timer_b2f;
    a_f = Int62ToFloat(a_int[i], ZK_F);
    b_f = Int62ToFloat(b_int[i], ZK_F);
    int2float_elapsed += timer_b2f.ns_elapse();
    ir_res = a_f / b_f;
    SimpleTimer timer_f2b;
    c_int[i] = FloatToInt62(ir_res, ZK_F);
    float2int_elapsed += timer_f2b.ns_elapse();
  }

  // Step 3: bool to arithmetic for c
  bool2arith<BoolIO<ZKNetIO>>((IntFp*)output.data(), c_int.data(), size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  return 0;
}

int MystiqueOpsImpl::_Inner_Sqrt(const vector<ZkIntFp>& a, vector<ZkIntFp>& output) {
  int size = a.size();
  int gate_num = 0;

  sync_zk_bool<BoolIO<ZKNetIO>>();
  vector<Integer> bin_a(size);
  arith2bool<BoolIO<ZKNetIO>>(bin_a.data(), (IntFp*)a.data(), size);

  vector<Float> zk_float(size, (0.0, PUBLIC));
  float2int_counter += size;
  int2float_counter += size;
  for(int i = 0; i < size; ++i) {
    SimpleTimer timer_b2f;
    zk_float[i] = Int62ToFloat(bin_a[i], ZK_F);
    int2float_elapsed += timer_b2f.ns_elapse();
    zk_float[i] = zk_float[i].sqrt();
    SimpleTimer timer_f2b;
    bin_a[i] = FloatToInt62(zk_float[i], ZK_F);
    float2int_elapsed += timer_f2b.ns_elapse();
  }
  tlog_debug << "Add Gate num for Sqrt: " << CircuitExecution::circ_exec->num_and() - gate_num << ENDL;

  bool2arith<BoolIO<ZKNetIO>>(output.data(), bin_a.data(), size);
  sync_zk_bool<BoolIO<ZKNetIO>>();
  return 0;
}

} // namespace rosetta
