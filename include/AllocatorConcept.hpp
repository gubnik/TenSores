/*
    TenSore, Mathematical tensor written in C++20
    Copyright (C) 2024, Nikolay Gubankov (aka nikgub)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <concepts>
#include <memory>
#include <type_traits>

namespace TenSore {

/**
 * @brief A concept for STL allocator 
 */
template<typename T>
concept Allocator =
  std::is_default_constructible_v<T> && std::is_copy_constructible_v<T> &&
  std::is_move_constructible_v<T> && std::is_copy_assignable_v<T> &&
  std::is_move_assignable_v<T> && requires() {
    typename T::value_type;
    typename T::size_type;
    typename T::difference_type;
  } && requires(T obj, std::allocator_traits<T>::pointer ptr, T::size_type N) {
    {
      obj.allocate(1)
    } -> std::same_as<typename std::allocator_traits<T>::pointer>;
    { obj.deallocate(ptr, N) };
  };

}
