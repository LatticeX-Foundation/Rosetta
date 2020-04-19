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

#include "player.h"
#include "input.h"
#include "dataset.h"

PYBIND11_MODULE(_rosetta, m) {
  m.doc() = R"pbdoc(
      SOME DOCUMENT DESCRIPTIONS HERE!
    )pbdoc";

  // clang-format off
  py::module m_player = m.def_submodule("player");
  py::class_<Player>(m_player, "Player")
    .def(py::init<>())
    .def_readonly("id", &Player::id)
    .def("init_mpc", &Player::init_mpc)
    .def("private_input", &Player::private_input)
    .def("rand_seed", &Player::rand_seed, "", py::arg("op_seed") = 0)
    .def("redirect_stdout", &Player::redirect_stdout)
    .def("restore_stdout", &Player::restore_stdout);

  py::module m_dataset = m.def_submodule("dataset");
  py::class_<DataSet>(m_dataset, "DataSet")
    .def(py::init<const Player&, bool, int, int>())
    .def("private_input_x", &DataSet::private_input_x)
    .def("private_input_y", &DataSet::private_input_y);

  py::module m_input = m.def_submodule("input");
  py::class_<Input>(m_input, "Input")
    .def(py::init<const Player&>())
    .def("private_input", (py::array_t<double>(Input::*)(int, const py::array_t<double>&)) & Input::private_input)
    .def("private_input", (double(Input::*)(int, double)) & Input::private_input);
  // clang-format on
}
