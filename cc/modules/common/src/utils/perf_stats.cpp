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
#include "cc/modules/common/include/utils/perf_stats.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
using namespace rapidjson;

namespace rosetta {

std::string PerfStats::to_console() {
  //! @todo
  std::string sret;
  return sret;
}

template <typename WRITER_REF>
void write_obj(WRITER_REF& writer, const PerfStats& ps) {
  writer.StartObject();
  {
    writer.Key("name");
    writer.String(ps.name.c_str());

    writer.Key("elapsed(s)");
    writer.StartObject();
    {
      writer.Key("clock");
      writer.Double(ps.s.clock_seconds);
      writer.Key("cpu");
      writer.Double(ps.s.cpu_seconds);
      writer.Key("elapse");
      writer.Double(ps.s.elapse);
    }
    writer.EndObject();

    writer.Key("communication(B)");
    writer.StartObject();
    {
      writer.Key("bytes-sent");
      writer.Int64(ps.s.bytes_sent);
      writer.Key("bytes-recv");
      writer.Int64(ps.s.bytes_recv);
      writer.Key("msg-sent");
      writer.Int64(ps.s.msg_sent);
      writer.Key("msg-recv");
      writer.Int64(ps.s.msg_recv);
    }
    writer.EndObject();
  }
  writer.EndObject();
}

template <typename WRITER_REF>
void write_objs(WRITER_REF& writer, const std::vector<PerfStats>& ps) {
  writer.StartArray();
  for (auto& p : ps) {
    write_obj(writer, p);
  }
  writer.EndArray();
}

std::string PerfStats::to_json(bool pretty) const {
  StringBuffer strBuf;

  if (pretty) {
    PrettyWriter<StringBuffer> writer(strBuf);
    writer.SetIndent(' ', 2);
    write_obj(writer, *this);
  } else {
    Writer<StringBuffer> writer(strBuf);
    write_obj(writer, *this);
  }

  std::string sret = strBuf.GetString();
  return sret;
}

std::string PerfStats::to_json(const std::vector<PerfStats>& ps, bool pretty) {
  StringBuffer strBuf;

  if (pretty) {
    PrettyWriter<StringBuffer> writer(strBuf);
    writer.SetIndent(' ', 2);
    write_objs(writer, ps);
  } else {
    Writer<StringBuffer> writer(strBuf);
    write_objs(writer, ps);
  }

  std::string sret = strBuf.GetString();
  return sret;
}

PerfStats operator+(const PerfStats& l, const PerfStats& r) {
  PerfStats t;
#define _add(v) t.s.v = l.s.v + r.s.v
  _add(bytes_sent);
  _add(bytes_recv);
  _add(msg_sent);
  _add(msg_recv);
  _add(elapsed_sent);
  _add(elapsed_recv);

  _add(clock_seconds);
  _add(cpu_seconds);
  _add(elapse);
#undef _add
  return t;
}
PerfStats operator-(const PerfStats& l, const PerfStats& r) {
  PerfStats t;
#define _add(v) t.s.v = l.s.v - r.s.v
  _add(bytes_sent);
  _add(bytes_recv);
  _add(msg_sent);
  _add(msg_recv);
  _add(elapsed_sent);
  _add(elapsed_recv);

  _add(clock_seconds);
  _add(cpu_seconds);
  _add(elapse);
#undef _add
  return t;
}

PerfStats operator/(const PerfStats& l, int r) {
  PerfStats t;
  if (r <= 0)
    return t;

#define _add(v) t.s.v = l.s.v / r
  _add(bytes_sent);
  _add(bytes_recv);
  _add(msg_sent);
  _add(msg_recv);
  _add(elapsed_sent);
  _add(elapsed_recv);

  _add(clock_seconds);
  _add(cpu_seconds);
  _add(elapse);
#undef _add
  return t;
}

} // namespace rosetta