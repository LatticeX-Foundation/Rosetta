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

#include <typeinfo>
#include <sstream>
#include <type_traits>
#include <utility>
#if defined(__GNUC__)
#include <memory>
#include <cxxabi.h>
#endif

// clang-format offx
namespace private_check_type {

template <typename T, bool IsBase = false>
struct check;

/**
 * Output state management
 */
class output {
  bool is_compact_ = true;

  template <typename T>
  bool check_empty(const T&) {
    return false;
  }
  bool check_empty(const char* val) { return (!val) || (val[0] == 0); }

  template <typename T>
  void out(const T& val) {
    if (check_empty(val))
      return;
    if (!is_compact_)
      sr_ += " ";
    using ss_t = std::ostringstream;
    sr_ += static_cast<ss_t&>(ss_t() << val).str();
    is_compact_ = false;
  }

  std::string& sr_;

 public:
  output(std::string& sr) : sr_(sr) {}

  output& operator()(void) { return (*this); }

  template <typename T1, typename... T>
  output& operator()(const T1& val, const T&... args) {
    out(val);
    return operator()(args...);
  }

  output& compact(void) {
    is_compact_ = true;
    return (*this);
  }
};

// ()

template <bool>
struct bracket {
  output& out_;

  bracket(output& out, const char* = nullptr) : out_(out) { out_("(").compact(); }

  ~bracket(void) { out_.compact()(")"); }
};

template <>
struct bracket<false> {
  bracket(output& out, const char* str = nullptr) { out(str); }
};

// [N]

template <size_t N = 0>
struct bound {
  output& out_;

  bound(output& out) : out_(out) {}
  ~bound(void) {
    if (N == 0)
      out_("[]");
    else
      out_("[").compact()(N).compact()("]");
  }
};

// (P1, P2, ...)

template <bool, typename... P>
struct parameter;

template <bool IsStart>
struct parameter<IsStart> {
  output& out_;

  parameter(output& out) : out_(out) {}
  ~parameter(void) { bracket<IsStart>{out_}; }
};

template <bool IsStart, typename P1, typename... P>
struct parameter<IsStart, P1, P...> {
  output& out_;

  parameter(output& out) : out_(out) {}
  ~parameter(void) {
    [this](bracket<IsStart>&&) {
      check<P1>{out_};
      parameter<false, P...>{out_.compact()};
    }(bracket<IsStart>{out_, ","});
  }
};

// Do output at destruct

struct at_destruct {
  output& out_;
  const char* str_;

  at_destruct(output& out, const char* str = nullptr) : out_(out), str_(str) {}
  ~at_destruct(void) { out_(str_); }

  void set_str(const char* str = nullptr) { str_ = str; }
};

/*
    CV-qualifiers, references, pointers
*/

template <typename T, bool IsBase>
struct check {
  output out_;
  check(const output& out) : out_(out) {
#if defined(__GNUC__)
    const char* typeid_name = typeid(T).name();
    auto deleter = [](char* p) {
      if (p)
        free(p);
    };
    std::unique_ptr<char, decltype(deleter)> real_name{
      abi::__cxa_demangle(typeid_name, nullptr, nullptr, nullptr), deleter};
    out_(real_name ? real_name.get() : typeid_name);
#else
    out_(typeid(T).name());
#endif
  }
};

#define CHECK_TYPE__(OPT)                                  \
  template <typename T, bool IsBase>                       \
  struct check<T OPT, IsBase> : check<T, true> {           \
    using base_t = check<T, true>;                         \
    using base_t::out_;                                    \
    check(const output& out) : base_t(out) { out_(#OPT); } \
  };

CHECK_TYPE__(const)
CHECK_TYPE__(volatile)
CHECK_TYPE__(const volatile)
CHECK_TYPE__(&)
CHECK_TYPE__(&&)
CHECK_TYPE__(*)

#undef CHECK_TYPE__

/*
    Arrays
*/

#define CHECK_TYPE_ARRAY__(CV_OPT, BOUND_OPT, ...)                                        \
  template <typename T, bool IsBase __VA_ARGS__>                                          \
  struct check<T CV_OPT[BOUND_OPT], IsBase> : check<T CV_OPT, !std::is_array<T>::value> { \
    using base_t = check<T CV_OPT, !std::is_array<T>::value>;                             \
    using base_t::out_;                                                                   \
                                                                                          \
    bound<BOUND_OPT> bound_ = out_;                                                       \
    bracket<IsBase> bracket_ = out_;                                                      \
                                                                                          \
    check(const output& out) : base_t(out) {}                                             \
  };

#define CHECK_TYPE_ARRAY_CV__(BOUND_OPT, ...)              \
  CHECK_TYPE_ARRAY__(, BOUND_OPT, , ##__VA_ARGS__)         \
  CHECK_TYPE_ARRAY__(const, BOUND_OPT, , ##__VA_ARGS__)    \
  CHECK_TYPE_ARRAY__(volatile, BOUND_OPT, , ##__VA_ARGS__) \
  CHECK_TYPE_ARRAY__(const volatile, BOUND_OPT, , ##__VA_ARGS__)

#define CHECK_TYPE_PLACEHOLDER__
CHECK_TYPE_ARRAY_CV__(CHECK_TYPE_PLACEHOLDER__)
#if defined(__GNUC__)
CHECK_TYPE_ARRAY_CV__(0)
#endif
CHECK_TYPE_ARRAY_CV__(N, size_t N)

#undef CHECK_TYPE_PLACEHOLDER__
#undef CHECK_TYPE_ARRAY_CV__
#undef CHECK_TYPE_ARRAY__

/*
    Functions
*/

template <typename T, bool IsBase, typename... P>
struct check<T(P...), IsBase> : check<T, true> {
  using base_t = check<T, true>;
  using base_t::out_;

  parameter<true, P...> parameter_ = out_;
  bracket<IsBase> bracket_ = out_;

  check(const output& out) : base_t(out) {}
};

/*
    Pointers to members
*/

template <typename T, bool IsBase, typename C>
struct check<T C::*, IsBase> : check<T, true> {
  using base_t = check<T, true>;
  using base_t::out_;

  check(const output& out) : base_t(out) {
    check<C>{out_};
    out_.compact()("::*");
  }
};

/*
    Pointers to member functions
*/

#define CHECK_TYPE_MEM_FUNC__(...)                              \
  template <typename T, bool IsBase, typename C, typename... P> \
  struct check<T (C::*)(P...) __VA_ARGS__, IsBase> {            \
    at_destruct cv_ = base_.out_;                               \
    check<T(P...), true> base_;                                 \
    output& out_ = base_.out_;                                  \
                                                                \
    check(const output& out) : base_(out) {                     \
      cv_.set_str(#__VA_ARGS__);                                \
      check<C>{out_};                                           \
      out_.compact()("::*");                                    \
    }                                                           \
  };

CHECK_TYPE_MEM_FUNC__()
CHECK_TYPE_MEM_FUNC__(const)
CHECK_TYPE_MEM_FUNC__(volatile)
CHECK_TYPE_MEM_FUNC__(const volatile)

#undef CHECK_TYPE_MEM_FUNC__

} // namespace private_check_type

/*
    Get the name of the given type

    check_type<const volatile void *>()
    -->
    void const volatile *
*/

template <typename T>
inline std::string check_type(void) {
  std::string str;
  private_check_type::check<T>{str};
  return str;
}
