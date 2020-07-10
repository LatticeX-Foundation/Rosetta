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

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#include <random>
#include <regex>
#include <string>
using namespace std;

/**
 * @todo optimize
 * vector<T> to file, eg.
 * a[0]\n
 * a[1]\n
 * ...
 * a.back()\n
 */
template <typename T>
inline void tofile(
  const vector<T>& v,
  const string& filename,
  std::ios_base::openmode mode = ios::out) {
  ofstream ofile(filename, mode);
  if (!ofile.good()) {
    cerr << "open " << filename << " failed!" << endl;
    return;
  }
  stringstream sss;
  if (is_same<T, double>::value) {
    int prec = std::numeric_limits<double>::digits10;
    sss.precision(prec);
  }
  for (int i = 0; i < v.size(); i++) {
    sss << v[i] << "\n";
    ofile.write(sss.str().c_str(), sss.str().length());
    sss.str("");
  }
  ofile.close();
}

/**
 * binary version
 * 
 * vector<vector<T>> to file, eg.
 * a[0][0]a[0][1]...a[0].back()\n
 * a[1][0]a[1][1]...a[1].back()\n
 * ...
 */
template <typename T>
inline void tofile(const vector<vector<T>>& v, const string& filename) {
  int rows = v.size();
  if (rows == 0) {
    cerr << "no any data" << endl;
    return;
  }
  int cols = v[0].size();
  bool do_check = false;
  if (do_check) {
    for (int r = 0; r < rows; r++) {
      if (cols != v[r].size()) {
        cerr << "v[" << r << "] expected " << cols << " but got " << v[r].size() << endl;
        return;
      }
    }
  }

  ofstream ofile(filename, ios::binary | ios::out | ios::trunc);
  if (!ofile.good()) {
    cerr << "open " << filename << " failed!" << endl;
    return;
  }
  ofile.write((const char*)&rows, sizeof(int));
  ofile.write((const char*)&cols, sizeof(int));

  int linesize = cols * sizeof(T);
  for (int r = 0; r < rows; r++) {
    ofile.write((const char*)&v[r][0], linesize);
  }
  ofile.close();
}

static inline void tofile(const string& s, const string& filename) {
  ofstream ofile(filename);
  if (!ofile.good()) {
    cerr << "open " << filename << " failed!" << endl;
    return;
  }
  ofile.write(s.data(), s.length());
  ofile.close();
}

template <typename T>
static void fromfile(vector<vector<T>>& v, const string& filename) {
  v.clear();
  v.resize(0);

  ifstream ifile(filename, ios::binary | ios::in);
  if (!ifile.good()) {
    cerr << "open " << filename << " failed!" << endl;
    return;
  }

  int rows, cols;
  ifile.read((char*)&rows, sizeof(int));
  ifile.read((char*)&cols, sizeof(int));
  v.resize(rows);

  int linesize = cols * sizeof(T);
  for (int r = 0; r < rows; r++) {
    v[r].resize(cols);
    ifile.read((char*)&v[r][0], linesize);
  }
  ifile.close();
}

