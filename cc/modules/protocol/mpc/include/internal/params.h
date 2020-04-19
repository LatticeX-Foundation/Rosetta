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
#pragma once

#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
using namespace rapidjson;

#include "logger.h"

class Params {
 public:
  bool is_file_exist(const string& filepath) {
    if (filepath.empty())
      return false;
    return (access(filepath.c_str(), F_OK) == 0);
  }
  void if_key_not_exist_exit(bool must_exist, const char* key) {
    if (must_exist) {
      cerr << key << " not exist!\n";
      exit(0);
    }
  }
  std::string GetString(
    rapidjson::Value& v, const char* key, const char* default_value = "", bool must_exist = false) {
    if (v.HasMember(key)) {
      return v[key].GetString();
    }
    if_key_not_exist_exit(must_exist, key);
    return std::string(default_value);
  }
  int GetInt(rapidjson::Value& v, const char* key, int default_value = 0, bool must_exist = false) {
    if (v.HasMember(key)) {
      return v[key].GetInt();
    }
    if_key_not_exist_exit(must_exist, key);
    return default_value;
  }
  float GetFloat(
    rapidjson::Value& v, const char* key, float default_value = 0.0f, bool must_exist = false) {
    if (v.HasMember(key)) {
      return v[key].GetFloat();
    }
    if_key_not_exist_exit(must_exist, key);
    return default_value;
  }
  bool GetBool(
    rapidjson::Value& v, const char* key, bool default_value = 0, bool must_exist = false) {
    if (v.HasMember(key)) {
      return v[key].GetBool();
    }
    if_key_not_exist_exit(must_exist, key);
    return default_value;
  }
};

/*
The parameters described in README.md
*/
#define KEYSNUM 4
#define PARTIES 3

class Params_Fields : public Params {
 public:
  size_t PATRIY_NUM = 1; // number of parties
  size_t PID = 4; // party id, for standalone is 4; 0~3, MPC.
  string DID = "";
  string TYPE = "3PC";
  string MODE = "ALL";
  string CONFIG_FILE = "";

  // default value
  size_t MAX_EPOCHS = 20;
  size_t MINI_BATCH = 32;

  size_t USE_KFOLD = 0;
  float MOMENTUM = 0.0;
  bool NESTEROV = false;

  string SAVE_MODEL = "PLAIN-FLOAT";

  string KEY_DIR = "./key/";
  string DATA_DIR = "./data/";
  string DATA_X = "";
  string DATA_Y = "";

  int BASE_PORT = 32000;

  string TRAIN_SAMPLES_FILE = "";
  string TRAIN_LABELS_FILE = "";
  string TEST_SAMPLES_FILE = "";
  string TEST_LABELS_FILE = "";

  int LABEL_PARTY = 0; // only in 0,1,2

  struct Node {
    string NAME;
    string HOST;
    string KEYS[KEYSNUM];
  };
  Node P[PARTIES];

  // By default, the local ciphertext values are saved in model.
  // Currently, only support it as 3-bit bitmap:[P2 P1 P0]
  //  0: none, all local ciphertext
  //  1: P0,
  //  2: P1,
  //  4: P2,
  //  3: P0 and P1
  //  5: P0 and P2
  //  6: P1 and P2
  //  7: P0, P1 and P2
  int SAVER_MODE = 0;

  // server and client certifications
  string SERVER_CERT = "certs/server-nopass.cert";
  string SERVER_PRIKEY = "certs/server-prikey";
  string SERVER_PRIKEY_PASSWORD = "123456";

  void print_params() {
    log_info << toString() << endl;
  }

  string toString() {
    stringstream sss;
    // clang-format off
    sss << "\n    PARAMETERS: --------------------------------"
        << "\n    PATRIY NUM: " << PATRIY_NUM
        << "\n      PARTY ID: " << PID
        << "\n          TYPE: " << TYPE
        << "\n          MODE: " << MODE
        << "\n   DATA SET ID: " << DID
        << "\n   CONFIG FILE: " << CONFIG_FILE
        << "\n"
        << "\n    MAX EPOCHS: " << MAX_EPOCHS
        << "\n    MINI BATCH: " << MINI_BATCH
        << "\n     USE KFOLD: " << USE_KFOLD
        << "\n      MOMENTUM: " << MOMENTUM
        << "\n      NESTEROV: " << NESTEROV
        << "\n    SAVE MODEL: " << SAVE_MODEL
        << "\n"
        << "\n       KEY DIR: " << KEY_DIR
        << "\n      DATA DIR: " << DATA_DIR
        << "\n        DATA X: " << DATA_X
        << "\n        DATA Y: " << DATA_Y
        << "\n     BASE PORT: " << BASE_PORT
        << "\n"
        << "\n TRAIN SAMPLES: " << TRAIN_SAMPLES_FILE
        << "\n  TRAIN LABELS: " << TRAIN_LABELS_FILE
        << "\n  TEST SAMPLES: " << TEST_SAMPLES_FILE
        << "\n   TEST LABELS: " << TEST_LABELS_FILE
        << "\n";
    // clang-format on
    for (int i = 0; i < PARTIES; i++) {
      sss << "\n       P" << i << " NAME: " << P[i].NAME;
      sss << "\n       P" << i << " HOST: " << P[i].HOST;
      sss << "\n       P" << i << " KEYS: ";
      for (size_t k = 0; k < KEYSNUM; k++) {
        sss << P[i].KEYS[k] << " ";
      }
    }
    sss << "\n              : --------------------------------"
        << "\n";
    return sss.str();
  }
};

/*
**
**
**
**
*/
class SecureParams : public Params_Fields {
 public:
 public:
  SecureParams(int argc, char* argv[]) {
    if (!parse(argc, argv))
      exit(0);
  }
  SecureParams(int party, const string& config_json_file) {
    // todo: support config_json_file: file or string
    string pid(std::to_string(party));
    if (!parse2(pid, config_json_file))
      exit(0);
  }
  bool parse(int argc, char* argv[]) {
    if (argc < 2) {
      string program(argv[0]);
      stringstream sss;
      // clang-format off
      sss << "\nUsage:"
          << "\n\t" << program << " <PID> <CONFIG>"
          << "\n"
          << "\n  <PID> party id. 0~3 for MPC, means node P0~P3; 4 for STANDALONE"
          << "\n  <CONFIG> config file. for some (common or global) parameters"
          << "\n";
      // clang-format on
      return false;
    }

    string pid(argv[1]);
    string config_file(argv[2]);
    return parse2(pid, config_file);
  }
  bool parse2(const string& pid, const string& config_file) {
    PID = atoi(pid.c_str());

    CONFIG_FILE = config_file;
    if (!load(config_file)) {
      cerr << "load config file failed!" << endl;
      return false;
    }

    // set values
    if (TYPE == "3PC") {
      PATRIY_NUM = 3;
      if (PID > 2 || PID < 0) {
        cerr << "TYPE " << TYPE << " with error PID " << PID << endl;
        return false;
      }
    } else if (TYPE == "STANDALONE" || TYPE == "SA" || TYPE == "1PC") {
      PATRIY_NUM = 1;
    } else {
      cerr << "not supported! error TYPE " << TYPE << endl;
      return false;
    }

    print_params();
    return true;
  }
  bool load(const string& config_file) {
    ifstream ifile(config_file);
    if (!ifile.is_open()) {
      cerr << "open " << config_file << " error!\n";
      return false;
    }
    string sjson("");
    while (!ifile.eof()) {
      string s;
      getline(ifile, s);
      sjson += s;
    }
    ifile.close();

    Document doc;
    if (doc.Parse(sjson.data()).HasParseError()) {
      cerr << "parser " << config_file << " error!\n";
      return false;
    }
    log_info << "load " << config_file << " ok, content:" << endl;
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    doc.Accept(writer);
    string data = buffer.GetString();
    //log_info << data << endl;

    log_info << "parse:" << endl;
    if (!parse(doc)) {
      log_error << "parse error" << endl;
      return false;
    }

    log_info << "parse ok" << endl;
    return true;
  }

  bool parse(Document& doc) {
    TYPE = GetString(doc, "TYPE", "3PC", false);

    MAX_EPOCHS = GetInt(doc, "MAX_EPOCHS", 20, false);
    MINI_BATCH = GetInt(doc, "MINI_BATCH", 32, false);

    USE_KFOLD = GetInt(doc, "USE_KFOLD", 0, false);
    MOMENTUM = GetFloat(doc, "MOMENTUM", 0.0, false);
    NESTEROV = GetBool(doc, "NESTEROV", false, false);
    if (MOMENTUM < 0.001 || MOMENTUM > 0.999)
      MOMENTUM = 0.0;

    SAVE_MODEL = GetString(doc, "SAVE_MODEL", "PLAIN-FLOAT", false);
    if (!((SAVE_MODEL == "PLAIN-FLOAT") || (SAVE_MODEL == "PLAIN-MYTYPE") ||
          (SAVE_MODEL == "SECURE-FLOAT") || (SAVE_MODEL == "SECURE-MYTYPE"))) {
      SAVE_MODEL = "PLAIN-FLOAT";
    }

    KEY_DIR = GetString(doc, "KEY_DIR", "./key/", false);
    DATA_DIR = GetString(doc, "DATA_DIR", "./data/", false);
    DATA_X = GetString(doc, "DATA_X", "", false);
    DATA_Y = GetString(doc, "DATA_Y", "", false);

    BASE_PORT = GetInt(doc, "BASE_PORT", 32000, false);
    if (BASE_PORT < 1025 || BASE_PORT > 65500) {
      cerr << "error base port: " << BASE_PORT << endl;
      return false;
    }

    // schema: ID-[TRAIN|TEST]-[SAMPLES|LABELS]-PID.TXT
    string id("-" + to_string(PID));
    if (PID == 4)
      id = "";
    TRAIN_SAMPLES_FILE = DATA_DIR + DID + "-TRAIN-SAMPLES" + id + ".TXT";
    TRAIN_LABELS_FILE = DATA_DIR + DID + "-TRAIN-LABELS" + id + ".TXT";
    TEST_SAMPLES_FILE = DATA_DIR + DID + "-TEST-SAMPLES" + id + ".TXT";
    TEST_LABELS_FILE = DATA_DIR + DID + "-TEST-LABELS" + id + ".TXT";

    LABEL_PARTY = GetInt(doc, "LABEL_PARTY", 0, false);
    if (LABEL_PARTY < 0 || LABEL_PARTY > 2) {
      cerr << "error label party: " << LABEL_PARTY << endl;
      return false;
    }

    // node host & keys
    for (int i = 0; i < PARTIES; i++) {
      string PX("P" + to_string(i));
      if (doc.HasMember(PX.c_str()) && doc[PX.c_str()].IsObject()) {
        Value& p = doc[PX.c_str()];
        P[i].HOST = GetString(p, "HOST", "", true);
        P[i].NAME = GetString(p, "NAME", PX.c_str(), false);

        // const Value& keys = p["KEYS"];
        // if (keys.Size() != KEYSNUM) {
        //   cerr << "error size about P[i].KEYS " << keys.Size() << endl;
        //   return false;
        // }
        // for (size_t k = 0; k < keys.Size(); k++) {
        //   // P[i].KEYS[k] = KEY_DIR + "/" + to_string(i) + "/" + keys[k].GetString();
        //   P[i].KEYS[k] = KEY_DIR + keys[k].GetString();
        // }
      }
    }

    // By default, the local ciphertext values are saved in model.
    // Currently, only support it as 3-bit bitmap:[P2 P1 P0]
    //  0: none, all local ciphertext
    //  1: P0,
    //  2: P1,
    //  4: P2,
    //  3: P0 and P1
    //  5: P0 and P2
    //  6: P1 and P2
    //  7: P0, P1 and P2
    SAVER_MODE = GetInt(doc, "SAVER_MODE", 0, false);
    if (SAVER_MODE < 0 || SAVER_MODE > 7) {
      cerr << "error SAVER_MODE : " << SAVER_MODE << endl;
      cerr << "ATTENTION: SAVER_MODE should be 0~7 to "
              "specify where to Save the plaintext model. "
              "its value is parsed as a bit-flag map: [P2 P1 P0],"
              "For example, 3 means to saved in plaintext for P10 and P1"
           << endl;
      return false;
    }

    // server and client certifications
    SERVER_CERT = GetString(doc, "SERVER_CERT", "certs/server-nopass.cert", true);
    SERVER_PRIKEY = GetString(doc, "SERVER_PRIKEY", "certs/server-prikey", true);
    SERVER_PRIKEY_PASSWORD = GetString(doc, "SERVER_PRIKEY_PASSWORD", "", false);
    if (!is_file_exist(SERVER_CERT)) {
      cerr << "SERVER_CERT:" << SERVER_CERT << " not exist!" << endl;
      return false;
    }
    if (!is_file_exist(SERVER_PRIKEY)) {
      cerr << "SERVER_PRIKEY:" << SERVER_PRIKEY << " not exist!" << endl;
      return false;
    }
    return true;
  }
};
