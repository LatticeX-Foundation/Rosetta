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
#include "cc/modules/common/include/utils/helper.h"
#include "cc/modules/common/include/utils/perf_stats.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/writer.h>
using namespace rapidjson;

#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>

namespace rosetta {

/**
 * /proc/meminfo
 * /proc/pid/statm
 * /proc/pid/status
 */

static inline int64_t getVmStatus(std::string stag = "VmRSS:") {
  std::string proc_pid_status("/proc/" + std::to_string(getpid()) + "/status");
  std::string line, tag;
  std::ifstream ifile(proc_pid_status);
  int64_t vmrss = 0;
  if (ifile.good() && ifile.is_open()) {
    while (std::getline(ifile, line)) {
      std::istringstream iss(line);
      iss >> tag;
      if (tag == stag) {
        iss >> tag;
        vmrss = std::atoll(tag.c_str());
        return vmrss;
      }
    }
    ifile.close();
  }
  return vmrss;
}

// cpu
/*
/proc/stat
cpu time = user + system + nice + idle + iowait + irq + softirq
cat /proc/stat
cpu  21821017 1088 1773070 129595151 11548 0 107088 0 0 0

/proc/pid/stat
*/

static inline void getCPUInfo(
  double& sys_cputime,
  double& pid_cputime,
  double& uptime,
  double& progstarttime) {
  //double uptime; // /proc/uptime
  double user, system, nice, idle, iowait, irq, softirq; // /proc/stat
  double utime, stime, cutime, cstime, starttime; // /proc/pid/stat
  std::string line, tag;
  {
    // uptime
    std::string proc_pid_status("/proc/uptime");
    std::ifstream ifile(proc_pid_status);
    if (ifile.good() && ifile.is_open()) {
      std::getline(ifile, line);
      std::istringstream iss(line);
      iss >> uptime;
      ifile.close();
    }
  }
  {
    std::string proc_pid_status("/proc/stat");
    std::ifstream ifile(proc_pid_status);
    if (ifile.good() && ifile.is_open()) {
      std::getline(ifile, line);
      std::istringstream iss(line);
      iss >> tag;
      iss >> user;
      iss >> system;
      iss >> nice;
      iss >> idle;
      iss >> iowait;
      iss >> irq;
      iss >> softirq;
      ifile.close();

      // std::cout << std::this_thread::get_id() << " " << tag << " " << user << " " << system << " "
      //           << nice << " " << idle << " " << iowait << " " << irq << " " << softirq
      //           << std::endl;
    }
  }
  {
    std::string proc_pid_status("/proc/" + std::to_string(getpid()) + "/stat");
    std::ifstream ifile(proc_pid_status);
    if (ifile.good() && ifile.is_open()) {
      std::getline(ifile, line);
      std::istringstream iss(line);
      for (int i = 1; i < 14; i++) {
        iss >> tag;
      }
      iss >> utime; /* 14 */
      iss >> stime; /* 15 */
      iss >> cutime; /* 16 */
      iss >> cstime; /* 17 */
      for (int i = 18; i < 22; i++) {
        iss >> tag;
      }
      iss >> starttime; /* 22 */
      progstarttime = starttime;

      ifile.close();

      //std::cout << std::this_thread::get_id() << " " << utime << " " << stime << std::endl;
    }
  }

  sys_cputime = user + system + nice + idle + iowait + irq + softirq;
  pid_cputime = utime + stime;
}

void memcpu_stats_fn(void* stat_ptr, bool sampling) {
  PerfStats* pstat = (PerfStats*)stat_ptr;

  int64_t max_vmrss = 0;
  double max_cpuusage_sys = 0;
  double max_cpuusage = 0;
  double avg_cpuusage = 0;

  double sys_cputime1, pid_cputime1;
  double sys_cputime2, pid_cputime2;
  double uptime1, uptime2;
  double progstarttime;
  long hz = sysconf(_SC_CLK_TCK);
  int cpucores = sysconf(_SC_NPROCESSORS_ONLN);

  ofstream ofile;
  std::string sampletag;
  if (sampling) {
    std::stringstream stag;
    stag << time(0) << "." << std::this_thread::get_id();
    sampletag = stag.str();
    ofile.open("/tmp/memcpu_sampling-" + sampletag + ".txt");
    std::string head =
      "tag tid time index interval vmrss cpuusage_of_sys cpuusage cpuusage_of_avg max_vmrss max_cpuusage_of_sys max_cpuusage\n";
    ofile.write(head.c_str(), head.length());
    ofile.flush();
  }

  int64_t i = 0;
  while (pstat->do_memcpu_stats) {
    getCPUInfo(sys_cputime1, pid_cputime1, uptime1, progstarttime);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    getCPUInfo(sys_cputime2, pid_cputime2, uptime2, progstarttime);

    // %CPU = PID/SYS
    double cpuper0 = 100.0 * ((pid_cputime2 - pid_cputime1) / (sys_cputime2 - sys_cputime1));
    max_cpuusage_sys = std::max(cpuper0, max_cpuusage_sys);

    // %CPU = INTERVAL
    double total_time = (pid_cputime2 - pid_cputime1);
    double seconds = uptime2 - uptime1;
    double cpuper1 = 100.0 * total_time / hz / seconds;
    max_cpuusage = std::max(cpuper1, max_cpuusage);

    // %CPU = TOTAL
    total_time = pid_cputime2;
    seconds = uptime2 - (progstarttime / hz);
    double cpuper2 = 100.0 * total_time / hz / seconds;
    avg_cpuusage = cpuper2;

    // MEM
    int64_t vmrss = getVmStatus();
    max_vmrss = std::max(vmrss, max_vmrss);

    if (sampling) {
      // sampling to file
      // tag tid time index interval vmrss cpuusage_of_sys cpuusage cpuusage_of_avg
      // max_vmrss max_cpuusage_of_sys max_cpuusage
      std::stringstream ss;
      ss << "memcpu " << std::this_thread::get_id() << " " << time(0) << " " << i++ << " "
         << uptime2 - uptime1 << " " << vmrss << " " << cpuper0 << " " << cpuper1 << " " << cpuper2
         << " " << max_vmrss << " " << max_cpuusage_sys << " " << max_cpuusage << std::endl;
      ofile << ss.str();
      ofile.flush();
      //std::cout << ss.str();
    }

    // assign
    pstat->s.max_vmrss = max_vmrss;
    pstat->s.max_cpuusage = max_cpuusage;
    pstat->s.avg_cpuusage = avg_cpuusage;
  }
}

void PerfStats::start_perf_stats(bool sampling /* = false */) {
  reset();

  do_memcpu_stats = true;
  timer.start();
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &process_cpu_time);
  // [YYL] comment to fix multiple-task environment resource release issues.
  // std::thread tt = std::thread(memcpu_stats_fn, this, sampling);
  // tt.detach();
}

PerfStats::__stat PerfStats::get_perf_stats(bool stop /* = false */) {
  do_memcpu_stats = !stop;
  if (!do_memcpu_stats) {
    timer.stop();
  }

  // clock timer
  s.elapse = timer.elapse();

  // process cpu time
  struct timespec process_cpu_time_end;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &process_cpu_time_end);
  s.cpu_seconds = process_cpu_time_end - process_cpu_time;

  return s;
}
void PerfStats::reset() {
  name = "default";
  do_memcpu_stats = false;
  memset(&s, 0, sizeof(__stat));
}

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
      // writer.Key("clock");
      // writer.Double(ps.s.clock_seconds);
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

    writer.Key("memory(kB)");
    writer.StartObject();
    {
      writer.Key("max-rss");
      writer.Int64(ps.s.max_vmrss);
    }
    writer.EndObject();

    writer.Key("cpu");
    writer.StartObject();
    {
      int cpucores = sysconf(_SC_NPROCESSORS_ONLN);
      writer.Key("cores");
      writer.Int64(cpucores);
      writer.Key("max-usage(%)");
      writer.Double(ps.s.max_cpuusage);
      // writer.Key("avg-usage(%)");
      // writer.Double(ps.s.avg_cpuusage);
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