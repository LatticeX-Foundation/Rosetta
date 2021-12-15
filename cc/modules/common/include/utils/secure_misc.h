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


#include "cc/modules/protocol/mpc/comm/include/mpc_helper.h"


#if DO_SECURE_OP_PERFORMANCE_STATISTICS
//! Usage
//! CALL_SECURE_OP_STATS_BEG(XXX);
//! ProtocolManager::Instance()->GetProtocol(task_id)->GetOps(...)->OP(...);
//! CALL_SECURE_OP_STATS_END(XXX);
#define CALL_SECURE_OP_STATS_BEG(opname) SECURE_OP_CALL_PROTOCOL_OP_STATS_BEG_(stats_name_, opname)
#define CALL_SECURE_OP_STATS_END(opname) SECURE_OP_CALL_PROTOCOL_OP_STATS_END_(stats_name_, opname)
#else
#define CALL_SECURE_OP_STATS_BEG(opname) (void)0
#define CALL_SECURE_OP_STATS_END(opname) (void)0
#endif



#ifdef DEBUG
#define CIPHERTEXT_REVEAL_OUTPUT(share_val, rows, cols, tips, msg_idx)   \
    {                                                                    \
        int real_rows = (rows), real_cols = (cols);                      \
        assert(share_val.size() >= (real_rows * real_cols));             \
        int idx = 0;                                                     \
        msg_id_t msg_id("ciphertext_reveal#" + to_string(msg_idx));      \
        vector<string> rv_val, recv_nodes{"P0", "P1", "P2"};             \
        string recv_parties = encode_reveal_multi_node(recv_nodes);      \
        attr_type attr_info;                                             \
        attr_info.insert(std::pair<string, string>("receive_parties", recv_parties));   \
        ptc_base_->GetOps(msg_id)->Reveal(share_val, rv_val, &attr_info);               \
        tlog_info_(task_id_) << tips;                           \
        for (int ii_ = 0; ii_ < real_rows; ii_++) {             \
            string line;                                        \
            for (int jj_ = 0; jj_ < real_cols; jj_++)           \
                line += (rv_val[idx++] + ", ");                 \
            tlog_info_(task_id_) << line.c_str();               \
        }                                                       \
    }

#define PLAINTEXT_VALUE_OUTPUT(pt_val, rows, cols, tips)        \
    {                                                           \
        int real_rows = (rows), real_cols = (cols);             \
        assert(pt_val.size() >= (real_rows * real_cols));       \
        tlog_info_(task_id_) << tips;                           \
        int idx = 0;                                            \
        for (int ii_ = 0; ii_ < real_rows; ii_++) {             \
            string line;                                        \
            for (int jj_ = 0; jj_ < real_cols; jj_++)           \
                line += (pt_val[idx++] + ", ");                 \
            tlog_info_(task_id_) << line.c_str();               \
        }                                                       \
    }

#define REVEAL_OUTPUT(share_val, rows, cols, tips, internal)          \
    {                                                                 \
        int real_rows = (rows), real_cols = (cols);                   \
        assert(share_val.size() >= (real_rows * real_cols));          \
        int idx = 0;                                                  \
        vector<string> recv_nodes{"P0", "P1", "P2"};                  \
        string recv_parties = encode_reveal_multi_node(recv_nodes);   \
        vector <double> rv_val;                                       \
        internal->Reveal(share_val, rv_val, recv_parties);            \
        tlog_info_(task_id_) << tips;                                 \
        for (int ii_ = 0; ii_ < real_rows; ii_++) {                   \
            string line;                                              \
            for (int jj_ = 0; jj_ < real_cols; jj_++)                 \
                line += (to_string(rv_val[idx++]) + ", ");            \
            tlog_info_(task_id_) << line.c_str();                     \
        }                                                             \
    }

#define PROTOCOL_REVEAL_OUTPUT(share_val, rows, cols, tips, internal)      \
    {                                                                 \
        int real_rows = (rows), real_cols = (cols);                   \
        assert(share_val.size() >= (real_rows * real_cols));          \
        int idx = 0;                                                  \
        vector<string> recv_nodes{"P0", "P1", "P2"};                  \
        vector <double> rv_val;                                       \
        internal->Reveal(share_val, rv_val, recv_nodes);              \
        tlog_info_(task_id_) << tips;                                 \
        for (int ii_ = 0; ii_ < real_rows; ii_++) {                   \
            string line;                                              \
            for (int jj_ = 0; jj_ < real_cols; jj_++)                 \
                line += (to_string(rv_val[idx++]) + ", ");            \
            tlog_info_(task_id_) << line.c_str();                     \
        }                                                             \
    }

#define PLAINTEXT_OUTPUT(pt_val, rows, cols, tips)              \
    {                                                           \
        int real_rows = (rows), real_cols = (cols);             \
        assert(pt_val.size() >= (real_rows * real_cols));       \
        tlog_info_(task_id_) << tips;                           \
        int idx = 0;                                            \
        for (int ii_ = 0; ii_ < real_rows; ii_++) {             \
            string line;                                        \
            for (int jj_ = 0; jj_ < real_cols; jj_++)           \
                line += (to_string(pt_val[idx++]) + ", ");      \
            tlog_info_(task_id_) << line.c_str();               \
        }                                                       \
    }

#define SECURE_OP_REVEAL(share_val, rows, cols, msg_id, tips)  \
{  \
    shared_ptr<ProtocolBase> protocol_base = ProtocolManager::Instance() \
      ->GetProtocol(ProtocolManager::Instance()->QueryMappingID(ctx->device()->attributes().incarnation())); \
    shared_ptr<NET_IO> net_io = protocol_base->GetNetHandler();    \
\
    vector<string> recv_nodes = net_io->GetNonComputationNodes();                              \
    vector<string> computation_nodes = net_io->GetParty2Node();         \
    recv_nodes.insert(recv_nodes.end(), computation_nodes.begin(), computation_nodes.end());                      \
                                                                                                                                                                                                                         \
    string recv_parties = encode_reveal_multi_node(recv_nodes);                                               \
    attr_type attr_info;                        \
    attr_info["receive_parties"] = recv_parties;                 \
            \
    vector<string> plain_text;                \
    protocol_base->GetOps(msg_id)->Reveal(share_val, plain_text, &attr_info);              \
\
    for (int i = 0; i < rows; i++) {    \
        stringstream ss;                        \
        for (int j = 0; j < cols; j++) {                               \
            ss << plain_text[i * cols + j] << ", ";            \
        }                                                             \
        log_info << tips << ": " << ss.str();                       \
    }                                    \
}

#else
#define CIPHERTEXT_REVEAL_OUTPUT(share_val, rows, cols, tips, msg_id)   (void)0
#define PLAINTEXT_VALUE_OUTPUT(pt_val, rows, cols, tips)                (void)0
#define REVEAL_OUTPUT(share_val, rows, cols, tips, msg_idx)             (void)0 
#define PROTOCOL_REVEAL_OUTPUT(share_val, rows, cols, tips, msg_idx)    (void)0 
#define PLAINTEXT_OUTPUT(pt_val, rows, cols, tips)                      (void)0  
#define SECURE_OP_REVEAL(share_val, rows, cols, msg_id, tips)                     (void)0  
#endif