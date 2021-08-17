#pragma once

#include "cc/modules/protocol/public/include/protocol_base.h"

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace rosetta {
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;


// Manage Protocols for tasks, each task binds a protocol object.
class ProtocolManager
{
private:
  ProtocolManager() = default;
  ProtocolManager(const ProtocolManager&) = delete;
  ProtocolManager(ProtocolManager&&) = delete;
  ProtocolManager& operator=(const ProtocolManager&) = delete;
  ProtocolManager& operator=(ProtocolManager&&) = delete;

 public:
  static ProtocolManager* Instance();

  /**
   * @desc: Get all registered protocols
   */
  vector<string> GetSupportedProtocols();

  /**
   * @desc: Regsiter a backend protocol factory
   * @param:
   *     protocol_name, the name that identify this protocol.
   * @note:
   *     when create a support protocol, a corralated factory should be registered
   */
  void RegisterProtocol(const string& protocol, shared_ptr<IProtocolFactory> factory);

  /**
   * @desc: Deregsiter a registered backend protocol. Used by C++ internally.
   * @param:
   *     protocol, the name that identify this protocol.
   * @note:
   *     normally, there is no reason to use this interface.
   */
  void DeRegisterProtocol(const string& protocol);

  /**
   * @desc: Get a working protocol object in use. 
   * @returns:
   *     The pointer to current protocol in use.
   */
  shared_ptr<ProtocolBase> GetProtocol(const string& task_id="");

  /**
   * @desc: Get the name of the protocol currently in use
   */
  string GetProtocolName(const string& task_id="");

  // Set/Get default protocol name
  void SetDefaultProtocolName(const string& protocol);
  string GetDefaultProtocolName();

  /**
   * @desc: Activate the specific protocol, which first create a protocol object, and then init the underlying
   *         network and core protocol stuff.
   * @param:
   *     protocol, the name of the protocol to activate.
   *     task_id, task id binded to the protocol object. 
   */
  int ActivateProtocol(const string& protocol, const string& task_id="");

  /**
   *  @desc: To deactivate the current protocol. This will close the underlying
   *      network and other resource.
   *  @param:
   *      task_id, task id binded to the protocol object.
   */
  int DeactivateProtocol(const string& task_id="");

  /**
   * @brief Mapping session unique id to task id.
   * @param
   *    unique_id, session unique id for device.
   *    task_id, task id the current task.
   * @note
   *    Rosetta inner interface for STATIC PASS, users should not user
   */
  void MappingID(const uint64_t& unique_id, const string& task_id="");

  /**
   * @brief Erase Mapping session unique id to task id.
   * @param
   *    unique_id, session unique id for device.
   * @note
   *    Rosetta inner interface for STATIC PASS, users should not user
   */
  void UnmappingID(const uint64_t& unique_id);

  /**
   * @brief Query Mapping task id with session unique id.
   * @param
   *    unique_id, session unique id for device.
   *    task_id, task id the current task.
   * @return Task id for the session.
   * @note
   *    Rosetta inner interface for STATIC PASS, users should not user, 
   *    please MappingID first, then QueryMappingID.
   */
  string QueryMappingID(const uint64_t& unique_id);

  /**
   * @brief Set float precision for fixpoint of Rosetta.
   * @param
   *    float_precision, precision bit count.
   *    task_id, task id the current task.
   */
  void SetFloatPrecision(int float_precision, const string& task_id="");
  int GetFloatPrecision(const string& task_id="");

  /**
   * @brief Set up the way of saving Rosetta training model.
   * @param 
   *    mode, it is a 3-bit bitmap:[P2 P1 P0] (deprecate)
              0: none, all private model
              1: P0 owns plain model, others got nothing
              2: P1 owns plain model, others got nothing
              4: P2 owns plain model, others got nothing
              3: P0 and P1 owns plain model, others got nothing
              5: P0 and P2 owns plain model, others got nothing
              6: P1 and P2 owns plain model, others got nothing
              7: P0, P1 and P2 owns plain model.
   *    model_nodes, save model node ids.
   *    task_id, task id the current task.
   * @note
        By default, the local ciphertext values are saved in model(mode=0)
  */
  void SetSaverModel(const SaverModel& model, const string& task_id="");
  SaverModel GetSaverModel(const string& task_id="");

  /**
   * @brief Set up the way of restoring Rosetta training model.
   * @param 
   *    mode, it is a 3-bit bitmap:[P2 P1 P0] ((deprecate))
              0: none, all local ciphertext
              1: P0 owns plain model, others got nothing
              2: P1 owns plain model, others got nothing
              4: P2 owns plain model, others got nothing
              3: P0 and P1 owns plain model, others got nothing
              5: P0 and P2 owns plain model, others got nothing
              6: P1 and P2 owns plain model, others got nothing
              7: P0, P1 and P2 owns plain model.
   *    model_nodes, restore model node ids.
   *    task_id, task id the current task.
   * @note
        By default, the local ciphertext values are saved in model (mode=0).
  */
  void SetRestoreModel(const RestoreModel& model_nodes, const string& task_id="");
  RestoreModel GetRestoreModel(const string& task_id="");


private:
  // Checking model nodes for legitimacy
  void _Check_Saver_Model(const string& task_id, const SaverModel& model);
  void _Check_Restore_Model(const string& task_id, const RestoreModel& model);


 private:
  // For now, we assume that the interfaces of this class will be called infrequent,
  // So for all resources, we use the same mutex lock to control access.
  std::string default_protocol_name_ = "SecureNN";
  std::mutex protocol_mutex_;
  // Note[GeorgeShi]: static registered different type protocols.
  unordered_map<string, std::shared_ptr<IProtocolFactory>> registered_factories_;
  // Note[GeorgeShi]: concurrent tasks running independent protocols.
  unordered_map<string, std::shared_ptr<ProtocolBase>> working_protocols_;
  unordered_map<uint64_t, string> unique_id_mapping_;

  
};//ProtocolManager

}//rosetta

