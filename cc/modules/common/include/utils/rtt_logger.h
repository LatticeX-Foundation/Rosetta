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

/**
 * thread-safe log based on spdlog
 */
#include "cc/modules/common/include/utils/logger_vector.h"
#include "spdlog/loggers.h"

enum LogLevel {
	Trace = 0,
	Debug,
	Audit,
	Info,
	Warn,
	Error,
	Fatal,
	Off
};

// log stream 
#if 1
#define log_trace tlog_trace_("")
#define log_debug tlog_debug_("")
#define log_audit tlog_audit_("")
#define log_info  tlog_info_("")
#define log_warn tlog_warn_("") 
#define log_error tlog_error_("")
#define log_fatal tlog_fatal_("")

#define tlog_trace tlog_trace_(context_->TASK_ID)
#define tlog_debug tlog_debug_(context_->TASK_ID)
#define tlog_audit tlog_audit_(context_->TASK_ID)
#define tlog_info  tlog_info_(context_->TASK_ID)
#define tlog_warn tlog_warn_(context_->TASK_ID) 
#define tlog_error tlog_error_(context_->TASK_ID)
#define tlog_fatal tlog_fatal_(context_->TASK_ID)
#else
#define tlog_trace tlog_trace_("")
#define tlog_debug tlog_debug_("")
#define tlog_audit tlog_audit_("")
#define tlog_info  tlog_info_("")
#define tlog_warn tlog_warn_("") 
#define tlog_error tlog_error_("")
#define tlog_fatal tlog_fatal_("")

#define log_trace tlog_trace_(context_->TASK_ID)
#define log_debug tlog_debug_(context_->TASK_ID)
#define log_audit tlog_audit_(context_->TASK_ID)
#define log_info  tlog_info_(context_->TASK_ID)
#define log_warn tlog_warn_(context_->TASK_ID) 
#define log_error tlog_error_(context_->TASK_ID)
#define log_fatal tlog_fatal_(context_->TASK_ID)
#endif

#define TRACE(...) TTRACE_(context_->TASK_ID, ##__VA_ARGS__)
#define DEB(...) TDEB_(context_->TASK_ID, ##__VA_ARGS__) 
#define AUDIT(...) TAUDIT_(context_->TASK_ID, ##__VA_ARGS__) 
#define INFO(...) TINFO_(context_->TASK_ID, ##__VA_ARGS__) 
#define WARN(...) TWARN_(context_->TASK_ID, ##__VA_ARGS__) 
#define ERROR(...) TERROR_(context_->TASK_ID, ##__VA_ARGS__) 
#define FATAL(...) TFATAL_(context_->TASK_ID, ##__VA_ARGS__) 

#define TTRACE(...) TTRACE_("", ##__VA_ARGS__)
#define TDEB(...) TDEB_("", ##__VA_ARGS__) 
#define TAUDIT(...) TAUDIT_("", ##__VA_ARGS__) 
#define TINFO(...) TINFO_("", ##__VA_ARGS__) 
#define TWARN(...) TWARN_("", ##__VA_ARGS__) 
#define TERROR(...) TERROR_("", ##__VA_ARGS__) 
#define TFATAL(...) TFATAL_("", ##__VA_ARGS__) 


#include "cc/modules/common/include/utils/rtt_logger.hpp"
