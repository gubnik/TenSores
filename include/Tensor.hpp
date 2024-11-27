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

#include "AllocatorConcept.hpp"
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>

#include <shared_mutex>
#include <stdexcept>
#include <utility>
#include <vector>

namespace TenSore {

/**
 * @class Tensor
 * @brief Mathematical tensor type
 *
 * @tparam T The type of value contained in a tensor
 * @tparam Order Dimensions of a tensor (1 - vector, 2 - matrix, etc.)
 * @tparam A The type of allocator used to manage memory of inside field
 *
 * @details
 * A production-ready implementation of mathematical Tensor
 * with regards to thread-safety and ownership semantics.
 * Iterators of this class are to be invalidated on any size change.
 */
template<typename T, std::size_t Order, Allocator A = std::allocator<T>>
class Tensor //: public std::enable_shared_from_this<Tensor<T, Order, A>>
{

public:
  class Iterator;

  Tensor() = delete;

  ~Tensor() = default;

  /**
   * @brief A template constructor for initializer list
   *
   * @tparam p_Dimensions Dimensions of a tensor,
   * amount of dimesnions provided must be equal to Order
   */
  template<std::size_t... p_Dimensions>
  Tensor()
  {
    static_assert(sizeof...(p_Dimensions) == Order,
                  "Misaligned dimensions of tensor in a constructor");
    m_DimensionsData = { { p_Dimensions... } };
    fsize();
    m_Data.resize(m_Size);
  }

  /**
   * @brief A constructor with an rvalue array
   *
   * @param p_Dimensions Array of dimensions
   */
  Tensor(std::array<size_t, Order>&& p_Dimensions)
  {
    m_DimensionsData = std::move(p_Dimensions);
    fsize();
    m_Data.resize(m_Size);
  }

  /**
   * @brief Copy contructor
   *
   * @param p_Other Tensor to be copied from
   */
  Tensor(const Tensor& p_Other)
  {
    std::lock_guard<std::shared_mutex> lock(p_Other.mutex());
    m_DimensionsData = p_Other.m_DimensionsData;
    m_Data = p_Other.m_Data;
  }

  /**
   * @brief Move contructor
   *
   * @param p_Other Tensor to be moved from
   */
  Tensor(Tensor&& p_Other) noexcept
  {
    std::lock_guard<std::shared_mutex> lock(p_Other.mutex());
    m_DimensionsData = std::move(p_Other.m_DimensionsData);
    m_Data = std::move(p_Other.m_Data);
  }

  /**
   * @brief Copy assign operator
   *
   * @param p_Other Tensor to be copied from
   */
  Tensor& operator=(const Tensor& p_Other)
  {
    std::lock_guard<std::shared_mutex> lock(p_Other.mutex());
    m_DimensionsData = p_Other.m_DimensionsData;
    m_Data = p_Other.m_Data;
    return *this;
  }

  /**
   * @brief Move assign operator
   *
   * @param p_Other Tensor to be moved from
   */
  Tensor& operator=(Tensor&& p_Other) noexcept
  {
    std::lock_guard<std::shared_mutex> lock(p_Other.mutex());
    m_DimensionsData = std::move(p_Other.m_DimensionsData);
    m_Data = std::move(p_Other.m_Data);
    return *this;
  }

  /**
   * @brief Invalidates all iterators
   *
   * @details
   * Tensor implements a versioning system. This method
   * increments the version of the tensor, which makes iterators
   * throw an exception if they try to access an element
   */
  void invalidate_iterators() noexcept
  {
    std::lock_guard<std::shared_mutex> lock(m_Mutex);
    m_Version++;
  }

  /**
   * @brief Forced claculation of the size of a tensor
   *
   * @return Total size of a tensor
   */
  std::size_t size() const noexcept { return m_Size; }

  /**
   * @brief Forced calculation of the size of a tensor
   *
   * @return Total size of a tensor
   */
  std::size_t fsize() noexcept
  {
    std::size_t _retval = 1;
    for (const auto& it : m_DimensionsData) {
      _retval *= it;
    }
    if (m_Size != _retval) {
      invalidate_iterators();
    }
    m_Size = _retval;
    return _retval;
  }

  /**
   * @brief Accessor for m_Mutex
   *
   * @return Mutex reference
   */
  std::shared_mutex& mutex() const { return m_Mutex; }

  /**
   * @brief Element access operator
   *
   * @param N Global index to access
   *
   * @return Element at index
   */
  T& operator[](std::size_t N)
  {
    if (N > size()) {
      throw std::out_of_range("Accessed an element outside of tensor's size");
    }
    return m_Data[N];
  }

  /**
   * @brief Element access with calculated index
   *
   * @param p_Dimensions Dimension coordinates to access.
   *
   * @return Element at calculated index
   */
  T& at(const std::array<std::size_t, Order>& dims)
  {
    std::lock_guard<std::shared_mutex> lock(mutex());
    std::size_t index = calculateIndex(dims);
    return m_Data[index];
  }

  /**
   * @brief Const lement access with calculated index
   *
   * @param p_Dimensions Dimension coordinates to access.
   *
   * @return Const lement at calculated index
   */
  const T& at(const std::array<std::size_t, Order>& dims) const
  {
    std::lock_guard<std::shared_mutex> lock(mutex());
    std::size_t index = calculateIndex(dims);
    return m_Data[index];
  }

  /**
   * @brief Element access operator with calculated index
   *
   * @tparam p_Dimensions Dimension coordinates to access.
   * Amount of coordinates must be equal to Order.
   *
   * @return Element at calculated index
   */
  template<std::size_t... p_Dimensions>
  T& operator()()
  {
    static_assert(sizeof...(p_Dimensions) == Order,
                  "Misaligned dimensions in tensor's `()` operator");

    std::lock_guard<std::shared_mutex> lock(mutex());
    const std::array<std::size_t, Order> _dims = { { p_Dimensions... } };
    return at(_dims);
  }

  /**
   * @brief Const element access operator with calculated index
   *
   * @tparam p_Dimensions Dimension coordinates to access.
   * Amount of coordinates must be equal to Order.
   *
   * @return Const element at calculated index
   */
  template<std::size_t... p_Dimensions>
  const T& operator()() const
  {
    static_assert(sizeof...(p_Dimensions) == Order,
                  "Misaligned dimensions in tensor's `()` operator");

    std::lock_guard<std::shared_mutex> lock(mutex());
    const std::array<std::size_t, Order> _dims = { { p_Dimensions... } };
    return at(_dims);
  }

  /**
   * @brief Element access operator with calculated index
   *
   * @param p_Dimensions Dimension coordinates to access.
   *
   * @return Element at calculated index
   */
  T& operator()(const std::array<std::size_t, Order>& p_Dims)
  {
    return at(p_Dims);
  }

  /**
   * @brief Const element access operator with calculated index
   *
   * @param p_Dimensions Dimension coordinates to access.
   *
   * @return Const element at calculated index
   */
  const T& operator()(const std::array<std::size_t, Order>& p_Dims) const
  {
    return at(p_Dims);
  }

  /**
   * @brief Iterator to the first element
   *
   * @return Iterator to the first element
   */
  Iterator begin() { return Iterator(this, 0, m_Version); }

  /**
   * @brief Iterator to the last element
   *
   * @return Iterator to the last element
   */
  Iterator end() { return Iterator(this, size(), m_Version); }

  /**
   * @brief Constant iterator to first element
   *
   * @return Constant iterator to the first element
   */
  const Iterator cbegin() const { return Iterator(this, 0, m_Version); }

  /**
   * @brief Constant iterator to last element
   *
   * @return Constant iterator to the last element
   */
  const Iterator cend() const { return Iterator(this, size(), m_Version); }

  /**
   * @brief Reverse iterator to the first element
   *
   * @return Reverse iterator to the first element
   */
  Iterator rbegin() { return std::reverse_iterator<Iterator>(begin()); }

  /**
   * @brief Reverse iterator to the last element
   *
   * @return Reverse iterator to the last element
   */
  Iterator rend() { return std::reverse_iterator<Iterator>(end()); }

  /**
   * @brief Constant reverse iterator to the last element
   *
   * @return Constant reverse iterator to the last element
   */
  const Iterator crbegin() const
  {
    std::lock_guard<std::shared_mutex> lock(mutex());
    return std::reverse_iterator<Iterator>(begin());
  }

  /**
   * @brief Constant reverse iterator to the last element
   *
   * @return Constant reverse iterator to the last element
   */
  const Iterator crend() const
  {
    std::lock_guard<std::shared_mutex> lock(mutex());
    return std::reverse_iterator<Iterator>(end());
  }

private:
  /**
   * @brief Calculates the global index from provided dimensional indices
   *
   * @return Calculated global index
   */
  std::size_t calculateIndex(const std::array<std::size_t, Order>& dims) const
  {
    std::lock_guard<std::shared_mutex> lock(mutex());
    std::size_t index = 0;
    std::size_t multiplier = 1;
    for (std::size_t i = 0; i < Order; ++i) {
      if (dims[i] >= m_DimensionsData[i]) {
        throw std::out_of_range("Index out of bounds");
      }
      index += dims[i] * multiplier;
      multiplier *= m_DimensionsData[i];
    }
    return index;
  }

protected:
  /**
   * @brief Mutex to be managed
   */
  mutable std::shared_mutex m_Mutex;

  /**
   * @brief Array that conctains the sizes for each dimension of the tensor
   */
  std::array<std::size_t, Order> m_DimensionsData;

  /**
   * @brief Vector of all the elements of a tensor
   */
  std::vector<T, A> m_Data;

  /**
   * @brief Total size of a tensor
   */
  std::size_t m_Size = 0;

  /**
   * @brief Version of a tensor
   *
   * @details
   * Iterators are invalidated after any operation that changes its size.
   */
  std::size_t m_Version = 0;

public:
  /**
   * @brief Iterator class of Tensor
   *
   * @details
   * The Iterator class represents an iterator of Tensor. It contains
   * an index to which it points, and provides necessary operations
   * for STL integration, as well as thread-safety.
   */
  class Iterator
  {
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    Iterator() = delete;

    /**
     * @brief Constructor of tensor
     *
     * @param p_Tensor Raw pointer to a tensor
     * @param p_Idx Index of an element at which this iterator points
     * @param p_Version Version of a tensor
     */
    Iterator(Tensor* p_Tensor, std::size_t p_Idx, std::size_t p_Version)
      : m_TensorPtr(p_Tensor)
      , m_Index(p_Idx)
      , m_Version(p_Version)
    {
    }

    void test_for_invalidation()
    {
      if (m_TensorPtr->m_Version != m_Version) {
        throw std::runtime_error("Iterator to a censor was invalidated");
      }
    }

    /**
     * @brief Dereference operator to access the element
     *
     * @return Element to which the iterator points
     */
    reference operator*()
    {
      test_for_invalidation();
      if (!m_TensorPtr)
        throw std::runtime_error("Tensor has been destroyed");
      return (*m_TensorPtr)[m_Index];
    }

    /**
     * @brief Pointer to the element at which the iterator is pointing to
     *
     * @return Pointer to the element at which the iterator is pointing to
     */
    pointer operator->()
    {
      test_for_invalidation();
      return &(**this);
    }

    Iterator& operator++()
    {
      ++m_Index;
      return *this;
    }

    Iterator operator++(int)
    {
      Iterator temp = *this;
      ++(*this);
      return temp;
    }

    Iterator& operator+=(difference_type n)
    {
      m_Index += n;
      return *this;
    }

    Iterator operator+(difference_type n) const
    {
      Iterator temp = *this;
      temp += n;
      return temp;
    }

    friend Iterator operator+(difference_type n, const Iterator& it)
    {
      return it + n;
    }

    Iterator& operator--()
    {
      --m_Index;
      return *this;
    }

    Iterator operator--(int)
    {
      Iterator temp = *this;
      --(*this);
      return temp;
    }
    Iterator& operator-=(difference_type n)
    {
      m_Index -= n;
      return *this;
    }

    Iterator operator-(difference_type n) const
    {
      Iterator temp = *this;
      temp -= n;
      return temp;
    }

    difference_type operator-(const Iterator& other) const
    {
      return m_Index - other.m_Index;
    }

    reference operator[](difference_type n)
    {
      test_for_invalidation();
      return *(*this + n);
    }

    bool operator==(const Iterator& other) const
    {
      return m_Index == other.m_Index;
    }

    bool operator!=(const Iterator& other) const { return !(*this == other); }

    auto operator<=>(const Iterator& other) const = default;

  private:
    Tensor* m_TensorPtr;
    std::size_t m_Index;
    std::size_t m_Version;
  };
};

}
