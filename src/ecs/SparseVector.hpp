/*
** Agartha-Software, 2023
** C++evy
** File description:
** SparseVector
*/

#pragma once

#include <optional>
#include <vector>

template <typename Type, typename Alloc = std::allocator<Type>>
class SparseVector {
  public:
  using value_type = std::optional<Type>;
  using reference_type = value_type &;
  using const_reference_type = value_type const &;
  using container_t = std::vector<value_type>;
  using size_type = typename container_t::size_type;
  using iterator = typename container_t::iterator;
  using const_iterator = typename container_t::const_iterator;

  private:
  container_t _data;

  public:
  SparseVector() : _data(container_t()) {};
  ~SparseVector() {};
  SparseVector(SparseVector const &other) : _data(other._data) {}; // copy constructor
  SparseVector(SparseVector &&other) noexcept
      : _data(std::move(other._data)) {}; // move constructor

  SparseVector &operator=(SparseVector const &other) { // copy assignment operator
    _data = other._data;
    return *this;
  };

  SparseVector &operator=(SparseVector &&other) noexcept { // move assignment operator
    _data = other._data;
    return *this;
  };

  reference_type operator[](std::size_t idx) { return _data.at(idx); };

  const_reference_type operator[](std::size_t idx) const { return _data.at(idx); };

  iterator begin() { return _data.begin(); };
  const_iterator begin() const { return _data.begin(); };
  const_iterator cbegin() const { return _data.cbegin(); };
  iterator end() { return _data.end(); };
  const_iterator end() const { return _data.end(); };
  const_iterator cend() const { return _data.cend(); };
  size_type size() const { return _data.size(); };
  void resize(size_type len) { _data.resize(len, std::nullopt); };

  size_type valid_size() const {
    size_type i = 0;
    for (auto it : _data) {
      if (it.has_value())
        i += 1;
    }
    return i;
  }

  reference_type insert_at(size_type pos, Type const &val) {
    if (pos >= _data.size())
      _data.resize(pos + 1, std::nullopt);
    _data[pos] = std::optional(val);
    return _data[pos];
  }

  reference_type insert_at(size_type pos, Type &&val) {
    if (pos >= _data.size())
      _data.resize(pos + 1, std::nullopt);
    _data[pos] = std::optional(val);
    return _data[pos];
  }

  template <class... Params>
  reference_type emplace_at(size_type pos, Params &&...args) {
    insert_at(pos, std::allocator_traits<Alloc>::construct(args...));
  }

  void erase(size_type pos) { _data[pos] = std::nullopt; }

  size_t first_free() {
    for (size_t i = 0; i != _data.size(); i++) {
      if (!_data[i]) {
        return i;
      }
    }
    return _data.size();
  }

  reference_type insert(Type &&val) { return insert_at(first_free(), val); }

  std::optional<size_type> get_index(value_type const &elem) const {
    value_type const *ptr = std::addressof(elem);

    for (size_type i = 0; i != _data.size(); i++) {
      if (ptr == std::addressof(_data[i])) {
        return i;
      }
    }
    return std::nullopt;
  }
};
