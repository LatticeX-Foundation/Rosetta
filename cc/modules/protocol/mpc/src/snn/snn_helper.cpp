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
#include "snn_helper.h"

#include <helper.h>
#include "AESObject.h"
#include "generate_key.h"
#include "logger.h"
#include "model_tool.h"
#include "tools.h"
#include "AESObject.h"
#include "ParallelAESObject.h"
#include "internal/opsets.h"
#include "communication.h"

#include <string>
std::string key_0 = "";
std::string key_a = "";
std::string key_b = "";
std::string key_c = "";
std::string key_ab = "";
std::string key_ac = "";
std::string key_bc = "";
std::string key_cd = "";

AESObject* aes_randseed = nullptr;
AESObject* aes_common = nullptr;
AESObject* aes_indep = nullptr;
AESObject* aes_a_1 = nullptr;
AESObject* aes_a_2 = nullptr;
AESObject* aes_b_1 = nullptr;
AESObject* aes_b_2 = nullptr;
AESObject* aes_c_1 = nullptr;
ParallelAESObject* aes_parallel = nullptr;

int NUM_OF_PARTIES;

// global variable. For this player number
int partyNum;

// global varible parsed from config file to specify which parties
// to store the plain result with MpcSaveV2
int SAVER_MODE;
// global variable. For faster DGK computation
small_mpc_t additionModPrime[PRIME_NUMBER][PRIME_NUMBER];
small_mpc_t multiplicationModPrime[PRIME_NUMBER][PRIME_NUMBER];

static void initializeMPC() {
  // populate offline module prime addition and multiplication tables
  for (int i = 0; i < PRIME_NUMBER; ++i)
    for (int j = 0; j < PRIME_NUMBER; ++j) {
      additionModPrime[i][j] = (i + j) % PRIME_NUMBER;
      multiplicationModPrime[i][j] = (i * j) % PRIME_NUMBER;
    }
}

static int init_params(const Params_Fields& params) {
  log_info << __FUNCTION__ << endl;
  NUM_OF_PARTIES = params.PATRIY_NUM;
  partyNum = (int)params.PID;
  SAVER_MODE = (int)params.SAVER_MODE;
  if ((partyNum < 0) || (partyNum > 3)) {
    LOGE("party num [%d] is error. only supports 3PC", partyNum);
    exit(0);
  }

  return 0;
}

static int init_keys(const Params_Fields& params) {
  log_info << __FUNCTION__ << endl;
  //synchronize(1);

  if (params.TYPE == "3PC") {
    // gen private key
    // 3PC: P0-->keyA, keyCD; P1-->keyB; P2-->keyC
    if (params.PID == 0) {
      key_a = gen_key_str();
    } else if (params.PID == 1) {
      key_b = gen_key_str();
    } else if (params.PID == 2) {
      key_c = gen_key_str();
      key_cd = gen_key_str();
    }
    usleep(1000);

    // public aes key
    string kab, kac, kbc, k0;
    k0 = gen_key_str();
    kab = gen_key_str();
    kac = gen_key_str();
    kbc = gen_key_str();

    auto sync_aes_key = GetMpcOpDefault(SyncAesKey);
    sync_aes_key->Run(PARTY_A, PARTY_B, kab, kab);
    sync_aes_key->Run(PARTY_A, PARTY_C, kac, kac);
    sync_aes_key->Run(PARTY_B, PARTY_C, kbc, kbc);
    sync_aes_key->Run(PARTY_C, PARTY_A, k0, k0);
    sync_aes_key->Run(PARTY_C, PARTY_B, k0, k0);
    key_0 = k0;
    key_ab = kab;
    key_bc = kbc;
    key_ac = kac;
  }
  //synchronize(1);
  usleep(1000);

#if MPC_DEBUG_USE_FIXED_AESKEY
  key_0 = "F0000000000000000000000000000000";
  key_a = "F000000000000000000000000000000A";
  key_b = "F000000000000000000000000000000B";
  key_c = "F000000000000000000000000000000C";
  key_ab = "F00000000000000000000000000000AB";
  key_ac = "F00000000000000000000000000000AC";
  key_bc = "F00000000000000000000000000000BC";
  key_cd = "F00000000000000000000000000000CD";
#endif
#if MPC_DEBUG
  cout << "key_0 :" << key_0 << endl;
  cout << "key_a :" << key_a << endl;
  cout << "key_b :" << key_b << endl;
  cout << "key_c :" << key_c << endl;
  cout << "key_ab:" << key_ab << endl;
  cout << "key_ac:" << key_ac << endl;
  cout << "key_bc:" << key_bc << endl;
  cout << "key_cd:" << key_cd << endl;
#endif

  return 0;
}

static int init_comm(const Params_Fields& params) {
  log_info << __FUNCTION__ << endl;
  if (MPC) {
    CommOptions opt;
    opt.party_id = params.PID;
    opt.parties = params.PATRIY_NUM;
    opt.base_port = params.BASE_PORT;
    for (int i = 0; i < 3; i++) {
      opt.hosts.push_back(params.P[i].HOST);
    }
    opt.server_cert_ = params.SERVER_CERT;
    opt.server_prikey_ = params.SERVER_PRIKEY;
    opt.server_prikey_password_ = params.SERVER_PRIKEY_PASSWORD;

    if (!initialize_communication(opt))
      return -1;
    //synchronize(1);
  }

  return 0;
}
static int init_aes2(int pid) {
  /*********************** AES SETUP and SYNC *************************/
  // P0 --> "KEYS":["keyA","keyAB","keyAC","null"]
  // P1 --> "KEYS":["keyB","keyAB","null","keyBC"]
  // P2 --> "KEYS":["keyC","keyCD","keyAC","keyBC"]
  // clang-format off
  vector<vector<string>> KEY = {
    {key_a, key_ab, key_ac, ""}, 
    {key_b, key_ab, "", key_bc},
    {key_c, key_cd, key_ac, key_bc}
  };
  vector<vector<string>> KSTR = {
    {"A", "AB", "AC", "NUL"}, 
    {"B", "AB", "NUL", "BC"},
    {"C", "CD", "AC", "BC"}
  };
  // clang-format on

#define INIT_AES_OBJECT(obj, i)                                                         \
  log_info << "P" << pid << " key[" << i << "]: " << KEY[pid][i] << " of " #obj " key(" \
           << KSTR[pid][i] << ")" << endl;                                              \
  obj = new AESObject();                                                                \
  obj->Init(KEY[pid][i])

  INIT_AES_OBJECT(aes_indep, 0);
  INIT_AES_OBJECT(aes_common, 1);
  INIT_AES_OBJECT(aes_a_1, 2);
  INIT_AES_OBJECT(aes_b_1, 2);
  INIT_AES_OBJECT(aes_c_1, 2);
  INIT_AES_OBJECT(aes_a_2, 3);
  INIT_AES_OBJECT(aes_b_2, 3);
#undef INIT_AES_OBJECT
  return 0;
}

static int init_aes(const Params_Fields& params) {
  log_info << __FUNCTION__ << endl;
  /*********************** AES SETUP and SYNC *************************/
  // P0 --> "KEYS":["keyA","keyAB","keyAC","null"]
  // P1 --> "KEYS":["keyB","keyAB","null","keyBC"]
  // P2 --> "KEYS":["keyC","keyCD","keyAC","keyBC"]
  auto pid = (params.PID == 4) ? 0 : params.PID;
#if MPC_USE_INIT_KEYS2
  init_aes2(pid);
#else
  aes_indep = new AESObject(params.P[pid].KEYS[0].c_str()); // argv[4]
  aes_common = new AESObject(params.P[pid].KEYS[1].c_str()); // argv[5]
  aes_a_1 = new AESObject(params.P[pid].KEYS[2].c_str()); // argv[6]
  aes_b_1 = new AESObject(params.P[pid].KEYS[2].c_str()); // argv[6]
  aes_c_1 = new AESObject(params.P[pid].KEYS[2].c_str()); // argv[6]
  aes_a_2 = new AESObject(params.P[pid].KEYS[3].c_str()); // argv[7]
  aes_b_2 = new AESObject(params.P[pid].KEYS[3].c_str()); // argv[7]
#endif
  aes_randseed = new AESObject();
  aes_randseed->Init(key_0);

  if (PARALLEL) {
    aes_parallel = new ParallelAESObject(params.P[pid].KEYS[1].c_str()); // argv[5]
    aes_parallel->precompute();
  }

  if (!STANDALONE)
    initializeMPC();

  LOGI("init aes objects ok.");
  //synchronize(1);
  return 0;
}

static int uninit_aes() {
  log_info << __FUNCTION__ << endl;
  DELETE_WITH_CHECK(aes_randseed);
  DELETE_WITH_CHECK(aes_common);
  DELETE_WITH_CHECK(aes_indep);
  DELETE_WITH_CHECK(aes_a_1);
  DELETE_WITH_CHECK(aes_a_2);
  DELETE_WITH_CHECK(aes_b_1);
  DELETE_WITH_CHECK(aes_b_2);
  DELETE_WITH_CHECK(aes_c_1);
  DELETE_WITH_CHECK(aes_parallel);

  return 0;
}
static int uninit_comm() {
  log_info << __FUNCTION__ << endl;
  uninitialize_communication();
  return 0;
}

/*
** 
** 
*/
int initialize_mpc(const Params_Fields& params) {
  // uncomment for logging
  //Logger::Get().log_to_stdout(true);
  init_params(params);
  init_comm(params);
  init_keys(params);
  init_aes(params);
  return 0;
}
int uninitialize_mpc() {
  uninit_comm();
  uninit_aes();
  return 0;
}
int partyid() {
  return partyNum;
}

void convert_mytype_to_double(const vector<mpc_t>& vec1, vector<double>& vec2) {
  vec2.resize(vec1.size());
  for (int i = 0; i < vec1.size(); i++)
    vec2[i] = MpcTypeToFloat(vec1[i]);
}
void convert_double_to_mytype(const vector<double>& vec1, vector<mpc_t>& vec2) {
  vec2.resize(vec1.size());
  for (int i = 0; i < vec1.size(); i++)
    vec2[i] = FloatToMpcType(vec1[i]);
}

void convert_mytype_to_double_bc(const vector<mpc_t>& vec1, vector<double>& vec2) {
  vec2.resize(vec1.size());
  for (int i = 0; i < vec1.size(); i++)
    vec2[i] = MpcTypeToFloatBC(vec1[i]);
}
void convert_double_to_mytype_bc(const vector<double>& vec1, vector<mpc_t>& vec2) {
  vec2.resize(vec1.size());
  for (int i = 0; i < vec1.size(); i++)
    vec2[i] = FloatToMpcTypeBC(vec1[i]);
}
