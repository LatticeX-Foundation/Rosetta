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
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/functional.h>
namespace py = pybind11;

#include "cc/python_export/input.h"
#include "cc/python_export/dataset.h"
#include "cc/python_export/msg_id_handle.h"
#include "cc/python_export/protocol_handler.h"
#include "cc/third_party/io/include/io/internal/channel_interface.h"
#include "cc/python_export/io_handler.h"

PYBIND11_MODULE(_rosetta, m) {
  m.doc() = R"pbdoc(
      Rosetta backend entry for python API.
    )pbdoc";

  py::enum_<LogLevel>(m, "LogLevel")
    .value("Trace", LogLevel::Trace)
    .value("Debug", LogLevel::Debug)
    .value("Audit", LogLevel::Audit)
    .value("Info",  LogLevel::Info)
    .value("Warn",  LogLevel::Warn)
    .value("Error", LogLevel::Error)
    .value("Fatal", LogLevel::Fatal)
    .value("Off", LogLevel::Off)
    .export_values();

  // clang-format off
  py::module m_protocol_handler = m.def_submodule("protocol");
  py::class_<ProtocolHandler>(m_protocol_handler, "ProtocolHandler")
    .def(py::init<>())
    .def("get_default_protocol_name", &ProtocolHandler::get_default_protocol_name)
    .def("get_supported_protocols", &ProtocolHandler::get_supported_protocols)
    .def("activate", &ProtocolHandler::activate, py::arg("protocol_name"), py::arg("task_id") = "", py::call_guard<py::gil_scoped_release>())
    .def("deactivate", &ProtocolHandler::deactivate, py::arg("task_id") = "", py::call_guard<py::gil_scoped_release>())
    .def("is_activated", &ProtocolHandler::is_activated, py::arg("task_id") = "")
    .def("get_protocol_name", &ProtocolHandler::get_protocol_name, py::arg("task_id") = "")
    .def("get_party_id", &ProtocolHandler::get_party_id, py::arg("task_id") = "")
    .def("log_to_stdout", &ProtocolHandler::log_to_stdout)
    .def("set_logfile", &ProtocolHandler::set_logfile)
    .def("set_logpattern", &ProtocolHandler::set_logpattern)
    .def("set_loglevel", &ProtocolHandler::set_loglevel, py::arg("level")=LogLevel::Info)
    .def("set_float_precision", &ProtocolHandler::set_float_precision, py::arg("float_precision"), py::arg("task_id") = "")
    .def("get_float_precision", &ProtocolHandler::get_float_precision, py::arg("task_id") = "")
    .def("set_saver_model", &ProtocolHandler::set_saver_model, py::arg("model"), py::arg("task_id") = "")
    .def("get_saver_model", &ProtocolHandler::get_saver_model, py::arg("task_id") = "")
    .def("set_restore_model", &ProtocolHandler::set_restore_model, py::arg("model"), py::arg("task_id") = "")
    .def("get_restore_model", &ProtocolHandler::get_restore_model, py::arg("task_id") = "")
    .def("rand_seed", &ProtocolHandler::rand_seed, py::arg("task_id") = "")
    .def("start_perf_stats", &ProtocolHandler::start_perf_stats, py::arg("task_id") = "")
    .def("get_perf_stats", &ProtocolHandler::get_perf_stats, py::arg("pretty")=true, py::arg("task_id") = "")
    .def("mapping_id", &ProtocolHandler::mapping_id, py::arg("unique_id"), py::arg("task_id") = "")
    .def("unmapping_id", &ProtocolHandler::unmapping_id, py::arg("unique_id"))
    .def("query_mapping_id", &ProtocolHandler::query_mapping_id)
    ;

  py::module m_io_handler = m.def_submodule("io");
  py::class_<IOHandler>(m_io_handler, "IOHandler")
    .def(py::init<>())
    .def("create_io", &IOHandler::create_io)
    .def("get_io_wrapper", &IOHandler::get_io_wrapper)
    .def("set_channel", &IOHandler::set_channel)
    ;
  
  py::class_<IChannel,  shared_ptr<IChannel> >(m_io_handler, "IChannel")
    ;

  py::class_<IOWrapper,  shared_ptr<IOWrapper> >(m_io_handler, "IOWrapper")
    .def("party_id_to_node_id", &IOWrapper::GetNodeId)
    .def("node_id_to_party_id", &IOWrapper::GetPartyId)
    .def("get_current_node_id", &IOWrapper::GetCurrentNodeId)
    .def("get_current_party_id", &IOWrapper::GetCurrentPartyId)
    .def("get_data_node_ids", &IOWrapper::GetDataNodes)
    .def("get_computation_node_ids", &IOWrapper::GetComputationNodes)
    .def("get_result_node_ids", &IOWrapper::GetResultNodes)
    .def("get_connected_node_ids", &IOWrapper::GetConnectedNodes)
    .def("recv_msg", &IOWrapper::recv_msg)
    .def("send_msg", &IOWrapper::send_msg)
    ;

  py::module m_dataset = m.def_submodule("dataset");
  py::class_<DataSet>(m_dataset, "DataSet")
    .def(py::init<const vector<string>&, const string&, int, const string&>())
    .def("private_input_x", &DataSet::private_input_x, py::arg("input")) // (np.array)
    .def("private_input_y", &DataSet::private_input_y, py::arg("input"));// (np.array)

  py::module m_input = m.def_submodule("input");
  py::class_<PrivateInput>(m_input, "PrivateInput")
    .def(py::init<const string&>())
    .def("input", (py::array_t<np_str_t>(Input::*)(const string&, const py::array_t<double>&)) & Input::input, 
                                                  py::arg("node_id"), py::arg("input"));

  py::class_<PublicInput>(m_input, "PublicInput")
    .def(py::init<const string&>())
    .def("input", (py::array_t<np_str_t>(Input::*)(const string&, const py::array_t<double>&)) & Input::input, 
                                                  py::arg("node_id"), py::arg("input"));

  py::module m_msgid_handle = m.def_submodule("msgid_handle");
  py::class_<MsgIdHandle>(m_msgid_handle, "MsgIdHandle")
    .def(py::init<>())
    .def("update_message_id_info", &MsgIdHandle::update_message_id_info);

  //py::module m_netutil = m.def_submodule("netutil");
  //m_netutil.def("enable_ssl_socket",    &netutil::enable_ssl_socket, "");
  //m_netutil.def("is_enable_ssl_socket",    &netutil::is_enable_ssl_socket, "");
  // clang-format on
}
