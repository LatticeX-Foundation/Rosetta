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
#include "cc/modules/protocol/mpc/comm/include/config.h"
namespace rosetta {

namespace {
bool is_file_exist(const string& filepath) {
  if (filepath.empty())
    return false;
  return (access(filepath.c_str(), F_OK) == 0);
}
void if_key_not_exist_then_exit(bool must_exist, const char* key) {
  if (must_exist) {
    log_error << "key[" << key << "] not exist!\n";
    exit(0);
  }
}
std::string GetString(
  rapidjson::Value& v,
  const char* key,
  const char* default_value = "",
  bool must_exist = false) {
  if (v.HasMember(key)) {
    return v[key].GetString();
  }
  if_key_not_exist_then_exit(must_exist, key);
  return std::string(default_value);
}
int GetInt(rapidjson::Value& v, const char* key, int default_value = 0, bool must_exist = false) {
  if (v.HasMember(key)) {
    return v[key].GetInt();
  }
  if_key_not_exist_then_exit(must_exist, key);
  return default_value;
}
float GetFloat(
  rapidjson::Value& v,
  const char* key,
  float default_value = 0.0f,
  bool must_exist = false) {
  if (v.HasMember(key)) {
    return v[key].GetFloat();
  }
  if_key_not_exist_then_exit(must_exist, key);
  return default_value;
}
bool GetBool(
  rapidjson::Value& v,
  const char* key,
  bool default_value = 0,
  bool must_exist = false) {
  if (v.HasMember(key)) {
    return v[key].GetBool();
  }
  if_key_not_exist_then_exit(must_exist, key);
  return default_value;
}
} // namespace

std::string PSIConfig::to_string() {
  std::stringstream sss;
  // clang-format off
  sss << "\n     PSI Config: --------------------------------"
      << "\n      PEER HOST: " << HOST
      << "\n     COMM. PORT: " << PORT
      << "\n     RECV PARTY: " << RECV_PARTY
      << "\n";
  // clang-format on
  return sss.str();
}

MPCConfig::MPCConfig() {
  // default values
  for (int i = 0; i < MPC_PARTIES; i++) {
    P[i].NAME = "Party " + std::to_string(i);
    P[i].HOST = "127.0.0.1";
  }
}

std::string MPCConfig::to_string() {
  std::stringstream sss;
  // clang-format off
  sss << "\n      MPC Config: --------------------------------"
      << "\n FLOAT PRECISION: " << FLOAT_PRECISION_M
      << "\n      SAVER MODE: " << SAVER_MODE
      << "\n     SERVER CERT: " << SERVER_CERT
      << "\n   SERVER PRIKEY: " << SERVER_PRIKEY
      << "\n SERVER PASSWORD: " << "******"
      << "\n";
  // clang-format on

  for (int i = 0; i < MPC_PARTIES; i++) {
    sss << "\n        P" << i << " NAME: " << P[i].NAME;
    sss << "\n        P" << i << " HOST: " << P[i].HOST;
    sss << "\n        P" << i << " PORT: " << P[i].PORT;
  }
  sss << "\n";
  return sss.str();
}

RosettaConfig::RosettaConfig(int argc, char* argv[]) {
  if (argc < 2) {
    stringstream sss;
    // clang-format off
      sss << "\nUsage:"
          << "\n\t" << string(argv[0]) << " <PID> <CONFIG>"
          << "\n"
          << "\n  <PID> party id. 0~3 for MPC, means node P0~P3; (0 or 1 for PSI); and so on"
          << "\n  <CONFIG> json-file or json-string"
          << "\n";
    // clang-format on
    exit(0);
  }

  int party = atoi(argv[1]);
  string config_json(argv[2]);

  //! @attention use PARTY_ID = party
  bool ret = load(party, config_json);
  if (!ret)
    exit(0);
  PARTY = party;
}
RosettaConfig::RosettaConfig(int party, const string& config_json) {
  //! @attention use PARTY_ID = party
  bool ret = load(party, config_json);
  if (!ret)
    exit(0);
  PARTY = party;
}
RosettaConfig::RosettaConfig(const string& config_json) {
  //! @attention use PARTY_ID in config_json
  int party = -1;
  bool ret = load(party, config_json);
  if (!ret)
    exit(0);
}

bool RosettaConfig::load(int party, const string& config_file) {
  PARTY = party;
  // config_json: json-file or json-string

  string sjson(config_file);
  ifstream ifile(config_file);
  if (!ifile.is_open()) {
    //log_warn << "open " << config_file << " error!\n";
    log_debug << "try to load as json string" << endl;
  } else {
    sjson = "";
    while (!ifile.eof()) {
      string s;
      getline(ifile, s);
      sjson += s;
    }
    ifile.close();
  }

  Document doc;
  if (doc.Parse(sjson.data()).HasParseError()) {
    log_error << "parser " << config_file << " error!\n";
    return false;
  }

  StringBuffer buffer;
  PrettyWriter<StringBuffer> writer(buffer);
  doc.Accept(writer);
  string data = buffer.GetString();
  log_debug << "Rosetta Config Source String:\n" << data << endl;

  if (!parse(doc)) {
    log_error << "parse error" << endl;
    return false;
  }

  return true;
}

bool RosettaConfig::parse(Document& doc) {
  //! @todo the PARTY_ID field in CONFIG.json have not yet used
  PARTY = GetInt(doc, "PARTY_ID", -1, false);

  // PSIConfig
  if (doc.HasMember("PSI") && doc["PSI"].IsObject()) {
    auto& cfg = psi;
    Value& PSI = doc["PSI"];
    cfg.HOST = GetString(PSI, "HOST", "127.0.0.1", false);
    cfg.PORT = GetInt(PSI, "PORT", 12345, false);
    cfg.RECV_PARTY = GetInt(PSI, "RECV_PARTY", 2, false);
  }

  // MPCConfig
  if (doc.HasMember("MPC") && doc["MPC"].IsObject()) {
    auto& cfg = mpc;
    Value& MPC = doc["MPC"];

    cfg.FLOAT_PRECISION_M = GetInt(MPC, "FLOAT_PRECISION", 13, false);

    // nodes
    for (int i = 0; i < MPC_PARTIES; i++) {
      string Pi("P" + to_string(i));
      if (MPC.HasMember(Pi.c_str()) && MPC[Pi.c_str()].IsObject()) {
        Value& p = MPC[Pi.c_str()];
        cfg.P[i].NAME = GetString(p, "NAME", Pi.c_str(), false);
        cfg.P[i].HOST = GetString(p, "HOST", "127.0.0.1", false);
        cfg.P[i].PORT = GetInt(p, "PORT", 9999, true);
      }
    }

    // saver mode
    cfg.SAVER_MODE = GetInt(MPC, "SAVER_MODE", 0, false);
    if ((cfg.SAVER_MODE < 0) || (cfg.SAVER_MODE > 7)) {
      log_error << "error SAVER_MODE: " << cfg.SAVER_MODE << ", expected 0~7." << endl;
      return false;
    }

    // server and client certifications
    cfg.SERVER_CERT = GetString(MPC, "SERVER_CERT", "certs/server-nopass.cert", false);
    cfg.SERVER_PRIKEY = GetString(MPC, "SERVER_PRIKEY", "certs/server-prikey", false);
    cfg.SERVER_PRIKEY_PASSWORD = GetString(MPC, "SERVER_PRIKEY_PASSWORD", "", false);
    if (!is_file_exist(cfg.SERVER_CERT)) {
      log_error << "SERVER_CERT:" << cfg.SERVER_CERT << " not exist!" << endl;
      return false;
    }
    if (!is_file_exist(cfg.SERVER_PRIKEY)) {
      log_error << "SERVER_PRIKEY:" << cfg.SERVER_PRIKEY << " not exist!" << endl;
      return false;
    }
  }

  // fmt_print();
  return true;
}

void RosettaConfig::fmt_print() {
  log_debug << "=======================================" << endl;
  log_debug << "          PARTY: " << PARTY << endl;
  log_debug << psi.to_string();
  log_debug << mpc.to_string();
  log_debug << "=======================================" << endl;
}

} // namespace rosetta