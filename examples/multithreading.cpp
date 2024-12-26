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
#include <TenSores/Tensor.hpp>
#include <cstddef>
#include <functional>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

using BigTensor = TenSore::Tensor<double, 4>;

void psum (const BigTensor& T, double& res, std::size_t s, std::size_t e)
{
  res = std::accumulate(T.begin() + s, T.begin() + e, 0.0);
  std::cout << "Partial sum : " << res << '\n';
}

int main (void)
{
  std::vector<std::thread> thread_pool;
  constexpr std::size_t thread_num = 8;

  BigTensor T1 = BigTensor({100, 100, 100, 100});
  const std::size_t tsz = T1.size();
  std::iota(T1.begin(), T1.end(), 0);

  std::vector<double> results (thread_num);

  std::size_t chsz = tsz / thread_num;
  for (std::size_t ti = 0; ti < thread_num; ti++)
  {
    std::size_t _s = ti * chsz;
    std::size_t _e = (ti == thread_num - 1) ? tsz : _s + chsz;
    thread_pool.emplace_back(psum, std::ref(T1), std::ref(results.at(ti)), _s, _e);
  }

  for (auto& it : thread_pool)
  {
    it.join();
  }
}
