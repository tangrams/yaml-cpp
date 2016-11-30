#pragma once

#include <cstddef>
#include <cstdlib>
#include <memory>
#include <vector>

#include "yaml-cpp/noncopyable.h"

namespace YAML {

// TODO: This class is no longer needed
template <typename T>
class ptr_vector : private YAML::noncopyable {
 public:
  ptr_vector() {}

  void clear() { m_data.clear(); }

  std::size_t size() const { return m_data.size(); }
  bool empty() const { return m_data.empty(); }

  void push_back(std::unique_ptr<T>&& t) { m_data.push_back(std::move(t)); }
  T& operator[](std::size_t i) { return *m_data[i]; }
  const T& operator[](std::size_t i) const { return *m_data[i]; }

  T& back() { return *(m_data.back().get()); }

  const T& back() const { return *(m_data.back().get()); }

 private:
  std::vector<std::unique_ptr<T>> m_data;
};
}
