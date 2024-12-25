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

#include "Tensor.hpp"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <cmath>

namespace TenSore {

/**
 * @brief Matrix definition
 */
template<std::integral T>
using Matrix = Tensor<T, 2>;

template<typename T>
std::ostream&
operator<<(std::ostream& out, const Matrix<T>& M)
{
  const auto _mlen = 1 + std::log10(*std::max_element(M.begin(), M.end()));
  const std::size_t _rows = M.dimensions()[0];
  std::size_t _c = 0;
  for (const T& it : M)
  {
    std::cout << std::setw(_mlen) << it << ' ';
    if (_c++ % _rows == _rows - 1)
    {
       std::cout << std::endl;
    }
  }
  return out;
}

}
