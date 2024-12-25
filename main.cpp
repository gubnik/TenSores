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

#include "include/Matrix.hpp"
#include <algorithm>
#include <iostream>
#include <numeric>
#include <ostream>

int
main(void)
{
  TenSore::Matrix<int> M({ 10, 10 });

  std::iota(M.begin(), M.end(), 0);

  std::cout << M << '\n';

  std::array<std::array<int, 10>, 10> _rows;

  for (std::size_t j = 0; j < M.dimensions()[1]; j++) {
    for (std::size_t i = 0; i < M.dimensions()[0]; i++) {
      _rows[i][j] = M({ j, i });
    }
  }

  for (std::size_t i = 0; i < _rows.size(); i++) {
    if (i % 2) {
      std::sort(_rows[i].begin(), _rows[i].end(), std::greater<int>());
    } else {
      std::sort(_rows[i].begin(), _rows[i].end(), std::less<int>());
    }
  }

  for (std::size_t j = 0; j < M.dimensions()[1]; j++) {
    for (std::size_t i = 0; i < M.dimensions()[0]; i++) {
      M({ M.dimensions()[1] - i - 1, M.dimensions()[0] - j - 1 }) = _rows[i][j];
    }
  }

  std::cout << '\n';

  std::cout << M << '\n';
}
