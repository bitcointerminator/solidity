/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <type_traits>
#include <array>

namespace dev
{

namespace details
{

template <bool B> using bool_constant = std::integral_constant<bool, B>;

template <class B> struct negation : bool_constant<!bool(B::value)> {};

template <class...> struct conjunction : std::true_type {};
template <class B1> struct conjunction<B1> : B1 {};
template <class B1, class... Bn> struct conjunction<B1, Bn...> : std::conditional_t<bool(B1::value), conjunction<Bn...>, B1> {};

template <class> struct is_ref_wrapper : std::false_type {};
template <class T> struct is_ref_wrapper<std::reference_wrapper<T>> : std::true_type {};

template <class T>
using not_ref_wrapper = negation<is_ref_wrapper<std::decay_t<T>>>;

template <class D, class...> struct return_type_helper { using type = D; };

template <class... Types>
struct return_type_helper<void, Types...> : std::common_type<Types...> {
	static_assert(conjunction<not_ref_wrapper<Types>...>::value, "Types cannot contain reference_wrappers when D is void");
};

template <class D, class... Types>
using return_type = std::array<typename return_type_helper<D, Types...>::type, sizeof...(Types)>;

} // namespace details

/// C++14 compatible implementation of std::make_array() from C++17.
template <class D = void, class... Types>
constexpr details::return_type<D, Types...> makeArray(Types&&... t)
{
	return { std::forward<Types>(t)... };
}

} // namespace dev
