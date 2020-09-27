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
namespace py = pybind11;

#include "cc/python_export/input.h"
#include "cc/python_export/dataset.h"
#include "cc/python_export/msg_id_handle.h"
#include "cc/python_export/protocol_handler.h"

PYBIND11_MODULE(_rosetta, m) {
  m.doc() = R"pbdoc(
      Rosetta backend entry for python API.
    )pbdoc";

  // clang-format off
  py::module m_protocol_handler = m.def_submodule("protocol");
  py::class_<ProtocolHandler>(m_protocol_handler, "ProtocolHandler")
    .def(py::init<>())
    .def("get_default_protocol_name", &ProtocolHandler::get_default_protocol_name)
    .def("get_supported_protocols", &ProtocolHandler::get_supported_protocols)
    .def("activate", &ProtocolHandler::activate)
    .def("deactivate", &ProtocolHandler::deactivate)
    .def("is_activated", &ProtocolHandler::is_activated)
    .def("get_protocol_name", &ProtocolHandler::get_protocol_name)
    .def("get_party_id", &ProtocolHandler::get_party_id)
    .def("log_to_stdout", &ProtocolHandler::log_to_stdout)
    .def("set_logfile", &ProtocolHandler::set_logfile)
    .def("set_loglevel", &ProtocolHandler::set_loglevel)
    .def("rand_seed", &ProtocolHandler::rand_seed)
    .def("start_perf_stats", &ProtocolHandler::start_perf_stats)
    .def("get_perf_stats", &ProtocolHandler::get_perf_stats)
    ;

  py::module m_dataset = m.def_submodule("dataset");
  py::class_<DataSet>(m_dataset, "DataSet")
    .def(py::init<const vector<int>&, int, int>())
    .def("private_input_x", &DataSet::private_input_x)
    .def("private_input_y", &DataSet::private_input_y);

  py::module m_input = m.def_submodule("input");
  py::class_<Input>(m_input, "Input")
    .def(py::init<>())
    .def("private_input", (py::array_t<np_str_t>(Input::*)(int, const py::array_t<double>&)) & Input::private_input);

  py::module m_msgid_handle = m.def_submodule("msgid_handle");
  py::class_<MsgIdHandle>(m_msgid_handle, "MsgIdHandle")
    .def(py::init<>())
    .def("update_message_id_info", &MsgIdHandle::update_message_id_info);
  // clang-format on
}
