// ==============================================================================
// Copyright 2021 The LatticeX Foundation
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
#include "cc/modules/common/include/utils/rtt_logger.h"

// fmt
template void fmt::v6::internal::format_value<char, Vector<rosetta::helix::Share> >(fmt::v6::internal::buffer<char>&, Vector<rosetta::helix::Share> const&, fmt::v6::internal::locale_ref); 
template void fmt::v6::internal::format_value<char, rosetta::helix::BitShare>(fmt::v6::internal::buffer<char>&, rosetta::helix::BitShare const&, fmt::v6::internal::locale_ref);      
template void fmt::v6::internal::format_value<char, rosetta::helix::Share>(fmt::v6::internal::buffer<char>&, rosetta::helix::Share const&, fmt::v6::internal::locale_ref);      
template void fmt::v6::internal::format_value<char, Vector<unsigned char> >(fmt::v6::internal::buffer<char>&, Vector<unsigned char> const&, fmt::v6::internal::locale_ref);  
template void fmt::v6::internal::format_value<char, CStr>(fmt::v6::internal::buffer<char>&, CStr const&, fmt::v6::internal::locale_ref);
template void fmt::v6::internal::format_value<char, Vector<rosetta::helix::BitShare> >(fmt::v6::internal::buffer<char>&, Vector<rosetta::helix::BitShare> const&, fmt::v6::internal::locale_ref);    
template void fmt::v6::internal::format_value<char, Vector<mpc_t> >(fmt::v6::internal::buffer<char>&, Vector<mpc_t> const&, fmt::v6::internal::locale_ref);   
template void fmt::v6::internal::format_value<char, Vector<double> >(fmt::v6::internal::buffer<char>&, Vector<double> const&, fmt::v6::internal::locale_ref);     

// const char*, int(0-3), std::vector<Share>
template void spdlog::logger::log<char const*, std::vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, std::vector<rosetta::helix::Share> const&);
template void spdlog::logger::log<char const*, int, std::vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, std::vector<rosetta::helix::Share> const&);
template void spdlog::logger::log<char const*, int, int, std::vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&,int const&, std::vector<rosetta::helix::Share> const&);
template void spdlog::logger::log<char const*, int, int, int, std::vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&,int const&, int const&, std::vector<rosetta::helix::Share> const&);
// const char*, int(0-3), Share
template void spdlog::logger::log<char const*, rosetta::helix::Share>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, rosetta::helix::Share const&);
template void spdlog::logger::log<char const*, int, rosetta::helix::Share>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, rosetta::helix::Share const&);
template void spdlog::logger::log<char const*, int, int, rosetta::helix::Share>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, rosetta::helix::Share const&);
template void spdlog::logger::log<char const*, int, int, int, rosetta::helix::Share>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, int const&, rosetta::helix::Share const&);
//const char*, int(0-3), BitShare
template void spdlog::logger::log<char const*, rosetta::helix::BitShare>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, rosetta::helix::BitShare const&);
template void spdlog::logger::log<char const*, int, rosetta::helix::BitShare>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, rosetta::helix::BitShare const&);
template void spdlog::logger::log<char const*, int, int, rosetta::helix::BitShare>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, rosetta::helix::BitShare const&);
template void spdlog::logger::log<char const*, int, int, int, rosetta::helix::BitShare>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&,int const&, rosetta::helix::BitShare const&);

// CStr
// const char*, int, int, int, CStr
template void spdlog::logger::log<char const*, int, int, int, CStr>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, int const&, CStr const&);
// const char*, int, int, CStr
template void spdlog::logger::log<char const*, int, int, CStr>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, CStr const&);
// const char*, int, CStr
template void spdlog::logger::log<char const*, int, CStr>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, CStr const&);
// const char*, CStr
template void spdlog::logger::log<char const*, CStr>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, CStr const&);
// const char*, string, CStr
template void spdlog::logger::log<char const*, std::string, CStr>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, std::string const&, CStr const&);
// const char*, int, string, CStr
template void spdlog::logger::log<char const*, int, std::string, CStr>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, std::string const&, CStr const&);
// const char*, int, int, string, CStr
template void spdlog::logger::log<char const*, int, int, std::string, CStr>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, std::string const&, CStr const&);
// const char*, int, int, int, string, CStr
template void spdlog::logger::log<char const*, int, int, int, std::string, CStr>(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, int const&, std::string const&, CStr const&);

// Vector
template void spdlog::logger::log<char const*, int, int, int, int, Vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, int const&, int const&, Vector<rosetta::helix::Share> const&);
template void spdlog::logger::log<char const*, int, int, int, int, Vector<unsigned char> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, int const&, int const&, Vector<unsigned char> const&);
template void spdlog::logger::log<char const*, int, int, int, int, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, int const&, int const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, int, int, int, std::string, Vector<unsigned char> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, int const&, std::string const&, Vector<unsigned char> const&);
template void spdlog::logger::log<char const*, int, int, int, std::string, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, int const&, std::string const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, int, int, int, Vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, int const&, Vector<rosetta::helix::Share> const&);
template void spdlog::logger::log<char const*, int, int, std::string, Vector<unsigned char> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, std::string const&, Vector<unsigned char> const&);
template void spdlog::logger::log<char const*, int, int, Vector<unsigned char> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, Vector<unsigned char> const&);
template void spdlog::logger::log<char const*, int, Vector<unsigned char> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, Vector<unsigned char> const&);
template void spdlog::logger::log<char const*, std::string, Vector<double> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, std::string const&, Vector<double> const&);
template void spdlog::logger::log<char const*, std::string, Vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, std::string const&, Vector<rosetta::helix::Share> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, int, int, Vector<unsigned char> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, int const&, int const&, Vector<unsigned char> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, int, int, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, int const&, int const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, int, Vector<rosetta::helix::BitShare> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, int const&, Vector<rosetta::helix::BitShare> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, int, Vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, int const&, Vector<rosetta::helix::Share> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, int, Vector<unsigned char> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, int const&, Vector<unsigned char> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, int, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, int const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, mpc_t, int, int, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, mpc_t const&, int const&, int const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, mpc_t, int, Vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, mpc_t const&, int const&, Vector<rosetta::helix::Share> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, mpc_t, int, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, mpc_t const&, int const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, mpc_t, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, mpc_t const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, Vector<unsigned char> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, Vector<unsigned char> const&);
template void spdlog::logger::log<char const*, mpc_t, mpc_t, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, mpc_t const&, mpc_t const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, Vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, Vector<rosetta::helix::Share> const&);
template void spdlog::logger::log<char const*, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, int, int, std::string, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, std::string const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, int, int, Vector<rosetta::helix::BitShare> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, Vector<rosetta::helix::BitShare> const&);
template void spdlog::logger::log<char const*, int, int, Vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, Vector<rosetta::helix::Share> const&);
template void spdlog::logger::log<char const*, int, int, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, int const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, int, Vector<rosetta::helix::BitShare> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, Vector<rosetta::helix::BitShare> const&);
template void spdlog::logger::log<char const*, int, Vector<mpc_t> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, Vector<mpc_t> const&);
template void spdlog::logger::log<char const*, int, Vector<double> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, Vector<double> const&);
template void spdlog::logger::log<char const*, int, Vector<rosetta::helix::Share> >(spdlog::source_loc, spdlog::level::level_enum, fmt::v6::basic_string_view<char>, char const* const&, int const&, Vector<rosetta::helix::Share> const&);


