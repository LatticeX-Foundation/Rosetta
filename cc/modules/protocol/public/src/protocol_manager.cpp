#include "cc/modules/protocol/public/include/protocol_manager.h"
#include <stdexcept>
#include <iostream>
#include "cc/modules/common/include/utils/rtt_logger.h"
#include "cc/modules/common/include/utils/rtt_exceptions.h"
#include "cc/modules/iowrapper/include/io_manager.h"
#include "cc/modules/protocol/public/include/protocol_ops.h"


#if ROSETTA_ENABLES_PROTOCOL_MPC_HELIX
#include "cc/modules/protocol/mpc/helix/include/helix_impl.h" 
#endif

#if ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN
#include "cc/modules/protocol/mpc/snn/include/snn_protocol.h"
using namespace rosetta::snn;
#endif

#include "cc/modules/protocol/mpc/naive/include/naive_impl.h"

namespace rosetta {

ProtocolManager* ProtocolManager::Instance() {
  static ProtocolManager s_protocol_mgr_;
  return &s_protocol_mgr_;
}

/**
 * @desc: Get all registered protocols
 */
vector<string> ProtocolManager::GetSupportedProtocols() {
  vector<string> res(registered_factories_.size());
  for (auto& prot : registered_factories_) {
    res.push_back(prot.first);
  }
  return res;
}

void ProtocolManager::RegisterProtocol(const string& protocol, shared_ptr<IProtocolFactory> factory) {
  registered_factories_[protocol] = factory;
}

void ProtocolManager::DeRegisterProtocol(const string& protocol) {
  if (registered_factories_.find(protocol) == registered_factories_.end())
    registered_factories_.erase(protocol);
}

shared_ptr<ProtocolBase> ProtocolManager::GetProtocol(const string& task_id/*=""*/) {
  std::lock_guard<std::mutex> lock(protocol_mutex_);
  auto protocol = working_protocols_.find(task_id);
  if (working_protocols_.find(task_id) == working_protocols_.end()) {
    return nullptr;
  }
  
  return protocol->second;
}

string ProtocolManager::GetProtocolName(const string& task_id/*=""*/) {
  auto proto = GetProtocol(task_id);
  if (proto.get() == nullptr)
    return "";
  else
    return proto->Name();
}

int ProtocolManager::ActivateProtocol(const string& protocol, const string& task_id/*=""*/) {
  // check if exist
  {
    std::lock_guard<std::mutex> lock(protocol_mutex_);
    if (working_protocols_.find(task_id) != working_protocols_.end()) {
      tlog_warn_(task_id) << "activate protocol failed, already exist task id: " << task_id ;
      return 0;
    }
  }

  // create task
  auto factory = registered_factories_.find(protocol);
  if (factory == registered_factories_.end()) {
    throw std::runtime_error("protocol: " + protocol + " not support");
  }
  
  auto proto_inst = factory->second->Create(task_id);
  
  // activate
  if (0 != proto_inst->Init()) {
    tlog_warn_(task_id) << "activate protocol failed, init protocol for task: " << task_id << " failed.";
    return -1;
  }
  
  // add protocol
  {
    std::lock_guard<std::mutex> lock(protocol_mutex_);
    working_protocols_[task_id] = proto_inst;
  }
  tlog_info_(task_id) << "create and activate ok. task: " << task_id << " for protocol: " << protocol;
  return 0;
}

/**
 *  @desc: to deactivate the current protocol. This will close the underlying
 *      network and other resource.
 *  @param:
 *      task_id, task id binded to the protocol object.
 */
int ProtocolManager::DeactivateProtocol(const string& task_id/*=""*/) {
  shared_ptr<rosetta::ProtocolBase> proto;
  {
    std::lock_guard<std::mutex> lock(protocol_mutex_);
    auto iter = working_protocols_.find(task_id);
    if (iter == working_protocols_.end()) {
      tlog_warn_(task_id) << "deactivate protocol failed, task id: " << task_id << " not exists!";
      return 1;
    }

    proto = iter->second;
    working_protocols_.erase(iter);
  }

  int ret = proto->Uninit();
  if (ret != 0) {
    tlog_warn_(task_id) << "uninit protocol failed !";
    return -1;
  }

  tlog_info_(task_id) << "deactivate protocol ok, task: " << task_id;
  return 0;
}

void ProtocolManager::SetSaverModel(const SaverModel& model, const string& task_id/*=""*/) {
  _Check_Saver_Model(task_id, model);

  std::lock_guard<std::mutex> lock(protocol_mutex_);
  auto iter = working_protocols_.find(task_id);
  if (iter == working_protocols_.end()) {
    tlog_warn_(task_id) << "set saver model failed, task id: " << task_id << " not exists!";
    return;
  }
  
  iter->second->GetMpcContext()->SAVER_MODEL = model;
}

SaverModel ProtocolManager::GetSaverModel(const string& task_id/*=""*/) {
  std::lock_guard<std::mutex> lock(protocol_mutex_);
  auto iter = working_protocols_.find(task_id);
  if (iter == working_protocols_.end()) {
    tlog_warn_(task_id) << "get restore model failed, task id: " << task_id << " not exists!";
    return SaverModel();
  }

  return iter->second->GetMpcContext()->SAVER_MODEL;
}

void ProtocolManager::SetRestoreModel(const RestoreModel& model, const string& task_id/*=""*/) {
  _Check_Restore_Model(task_id, model);

  std::lock_guard<std::mutex> lock(protocol_mutex_);
  auto iter = working_protocols_.find(task_id);
  if (iter == working_protocols_.end()) {
    tlog_warn_(task_id) << "set restore model failed, task id: " << task_id << " not exists!";
    return;
  }

  iter->second->GetMpcContext()->RESTORE_MODEL = model;
}

RestoreModel ProtocolManager::GetRestoreModel(const string& task_id/*=""*/) {
  std::lock_guard<std::mutex> lock(protocol_mutex_);
  auto iter = working_protocols_.find(task_id);
  if (iter == working_protocols_.end()) {
    tlog_warn_(task_id) << "get restore model failed, task id: " << task_id << " not exists!";
    return RestoreModel();
  }

  return iter->second->GetMpcContext()->RESTORE_MODEL;
}

void ProtocolManager::SetFloatPrecision(int float_precision, const string& task_id/*=""*/) {
  std::lock_guard<std::mutex> lock(protocol_mutex_);
  auto iter = working_protocols_.find(task_id);
  if (iter == working_protocols_.end()) {
    tlog_warn_(task_id) << "set float precision failed, task id: " << task_id << " not exists!";
    return;
  }

  iter->second->GetMpcContext()->FLOAT_PRECISION = float_precision;
}

int ProtocolManager::GetFloatPrecision(const string& task_id/*=""*/) {
  std::lock_guard<std::mutex> lock(protocol_mutex_);
  auto iter = working_protocols_.find(task_id);
  if (iter == working_protocols_.end()) {
    tlog_warn_(task_id) << "get float precision failed, task id: " << task_id << " not exists!";
    return -1;
  }
  return iter->second->GetMpcContext()->FLOAT_PRECISION;
}

// Set/Get default protocol
void ProtocolManager::SetDefaultProtocolName(const string& protocol) {
  default_protocol_name_ = protocol;
}

string ProtocolManager::GetDefaultProtocolName() {
  return default_protocol_name_;
}

void ProtocolManager::MappingID(const uint64_t& unique_id, const string& task_id/*=""*/) {
  std::lock_guard<std::mutex> lock(protocol_mutex_);
  auto iter = unique_id_mapping_.find(unique_id);
  if (iter != unique_id_mapping_.end()) {
    tlog_warn_(task_id) << "mapping from unique id: " << unique_id << " to task id: " << task_id << " already existed!";
    return;    
  }

  tlog_info_(task_id) << "mapping task id: " << task_id << ", with unique id: "<< unique_id;
  unique_id_mapping_[unique_id] = task_id;
}

void ProtocolManager::UnmappingID(const uint64_t& unique_id) {
  std::lock_guard<std::mutex> lock(protocol_mutex_);
  unique_id_mapping_.erase(unique_id);
}

string ProtocolManager::QueryMappingID(const uint64_t& unique_id) {
  std::lock_guard<std::mutex> lock(protocol_mutex_);
  auto iter = unique_id_mapping_.find(unique_id);
  if (iter == unique_id_mapping_.end()) {
    log_warn << "cannot find mapping task id, with unique id: "<< unique_id;
    return "";
  }

  return iter->second;
}

void ProtocolManager::_Check_Saver_Model(const string& task_id, const SaverModel& model) {
  shared_ptr<NET_IO> net_io = IOManager::Instance()->GetIOWrapper(task_id);
  const map<string, int>& computation_nodes = net_io->GetComputationNodes();
  const vector<string>& result_nodes = net_io->GetResultNodes();
  if (model.is_ciphertext_mode()) {
    const map<string, int>& ciphertext_nodes = model.get_ciphertext_nodes();
    for (auto iter = ciphertext_nodes.begin(); iter != ciphertext_nodes.end(); iter++) {
      int send_party = iter->second;
      string recv_node = iter->first;
      if (std::find(result_nodes.begin(), result_nodes.end(), recv_node) != result_nodes.end()) {
        log_info << "Check Saver Model: " << recv_node << " is a valid result node";
      } else {
        auto citer = computation_nodes.find(recv_node);
        if (citer != computation_nodes.end() && send_party == citer->second) {
          log_info << "Check Saver Model: " << recv_node << " is a valid computation node and saves ciphertext model locally";
        } else {
          throw other_exp(recv_node + " check saver ciphertext model failed!");
        }
      }
    }
  } else if (model.is_plaintext_mode()) {
    const vector<string>& plaintext_nodes = model.get_plaintext_nodes();
    for (auto iter = plaintext_nodes.begin(); iter != plaintext_nodes.end(); iter++) {
      if (std::find(result_nodes.begin(), result_nodes.end(), *iter) != result_nodes.end()) {
        log_info << "Check Saver Model: " << *iter << " is a valid result node";
      } else {
        throw other_exp(*iter + " is not a valid result node, check saver plaintext model failed!");
      }
    }
  }
}

void ProtocolManager::_Check_Restore_Model(const string& task_id, const RestoreModel& model) {
  shared_ptr<NET_IO> net_io = IOManager::Instance()->GetIOWrapper(task_id);
  const vector<string>& data_nodes = net_io->GetDataNodes();
  const map<string, int>& computation_nodes = net_io->GetComputationNodes();
  if (model.is_ciphertext_mode()) {
    const map<string, int>& ciphertext_nodes = model.get_ciphertext_nodes();
    for (auto iter = ciphertext_nodes.begin(); iter != ciphertext_nodes.end(); iter++) {
      string send_node = iter->first;
      int recv_party = iter->second;
      if (std::find(data_nodes.begin(), data_nodes.end(), send_node) != data_nodes.end()) {
        log_info << "Check Restore Model: " << send_node << " is a valid data node";
      } else {
        auto citer = computation_nodes.find(send_node);
        if (citer != computation_nodes.end() && citer->second == recv_party) {
          log_info << "Check Restore Model: " << send_node << " is a valid computation node and restore ciphertext model locally";
        } else {
          throw other_exp(send_node + " check restore ciphertext model fail!");
        }
      }
    }
  } else if (model.is_private_plaintext_mode()) {
    const string& plaintext_node = model.get_plaintext_node();
    if (std::find(data_nodes.begin(), data_nodes.end(), plaintext_node) != data_nodes.end()) {
      log_info << "Check Restore Model: " << plaintext_node << " is a valid data node";
    } else {
      throw other_exp(plaintext_node + " is not a valid data node, check restore plaintext model fail!");
    }
  }
}

/**
 * @note: you can also register your customized cryptographic protocol factory here
 * if you has implemented your protocol according to our protocol interface.
 */
// Static protocol factory registering
template <typename ProtocolFactory>
struct ProtocolFactoryRegistrar {
  ProtocolFactoryRegistrar(const string& name) {
    auto impl_ptr = make_shared<ProtocolFactory>();
    auto base_ptr = std::dynamic_pointer_cast<IProtocolFactory>(impl_ptr);
    ProtocolManager::Instance()->RegisterProtocol(name, base_ptr);
  }
};

#define REGISTER_SECURE_PROTOCOL_FACTORY(Prot, Name) \
  static ProtocolFactoryRegistrar<Prot> _protocol_registrar_##Prot(Name)

#if ROSETTA_ENABLES_PROTOCOL_MPC_HELIX
REGISTER_SECURE_PROTOCOL_FACTORY(HelixProtocolFactory, "Helix");
#endif

#if ROSETTA_ENABLES_PROTOCOL_MPC_SECURENN
REGISTER_SECURE_PROTOCOL_FACTORY(SnnProtocolFactory, "SecureNN");
#endif

REGISTER_SECURE_PROTOCOL_FACTORY(NaiveProtocolFactory, "Naive");

}// rosetta
