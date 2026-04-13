// Copyright (c) 2015 Baidu.com, Inc. All Rights Reserved
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef BRAFT_BRAFT_LATENCY_H
#define BRAFT_BRAFT_LATENCY_H

#include <butil/time.h>
#include <butil/logging.h>
#include <gflags/gflags_declare.h>

DECLARE_int32(raft_latency_log_threshold_ms);

// ---------------------------------------------------------------------------
// Latency instrumentation macros
//
// Usage pattern:
//   BRAFT_LATENCY_BEGIN(flag)          -- place once at start of function/block
//   BRAFT_LATENCY_CONTINUE(my_var)     -- capture an intermediate timestamp
//   BRAFT_LATENCY_END(threshold_ms, msg) -- emit log if elapsed > threshold
//
// IMPORTANT — injected variable names:
//   BRAFT_LATENCY_BEGIN injects two variables into the enclosing scope:
//     • braft_latency_start_ts   (int64_t) — wall-clock start time in ms
//     • braft_latency_is_enabled (bool)    — whether logging is active
//   BRAFT_LATENCY_CONTINUE(var) injects `var` (int64_t) into the same scope.
//   BRAFT_LATENCY_END injects (inside its own if-block only):
//     • braft_latency_end, braft_latency_elapsed, braft_latency_start
//
//   Because CONTINUE and END need to access the variables declared by BEGIN,
//   the macros CANNOT be wrapped in do { } while(0) and must all reside in
//   the same lexical scope. If the call site already has local variables
//   named braft_latency_start_ts or braft_latency_is_enabled (or any name
//   passed to BRAFT_LATENCY_CONTINUE), the build will fail with a
//   "redeclaration" error — which is the safe outcome. Silent shadowing is
//   not possible here because the injected declarations use the same type.
// ---------------------------------------------------------------------------

#define BRAFT_LATENCY_BEGIN(flag)                                    \
    int64_t braft_latency_start_ts = 0;                            \
    const bool braft_latency_is_enabled = (flag);                  \
    if (braft_latency_is_enabled) {                                \
        braft_latency_start_ts = butil::monotonic_time_ms();      \
    }

#define BRAFT_LATENCY_CONTINUE(var)              \
    int64_t var = 0;                             \
    if (braft_latency_is_enabled) {            \
        var = butil::monotonic_time_ms();        \
    }

#define BRAFT_LATENCY_END(threshold_ms, msg)                                                    \
    if (braft_latency_is_enabled) {                                                           \
        const int64_t braft_latency_end = butil::monotonic_time_ms();                           \
        const int64_t braft_latency_elapsed = braft_latency_end - braft_latency_start_ts;     \
        const int64_t braft_latency_start = braft_latency_start_ts;                           \
        (void)braft_latency_start;                                                              \
        LOG_IF(INFO, braft_latency_elapsed > (threshold_ms)) << msg;                            \
    }

#endif  // BRAFT_BRAFT_LATENCY_H
