#pragma once

#include "yaml-cpp/dll.h"

namespace YAML {
namespace detail {

template <typename T>
struct ref_holder {
  ~ref_holder<T>() { release(); }
  ref_holder<T>(T* ptr) {
    if (ptr) {
      ptr->hold();
    }
    m_ptr = ptr;
  }

  ref_holder<T>(const ref_holder<T>& ref) {
    if (ref.m_ptr) {
      ref.m_ptr->hold();
    }
    m_ptr = ref.m_ptr;
  }

  ref_holder<T>(ref_holder<T>&& ref) {
    m_ptr = ref.m_ptr;
    ref.m_ptr = nullptr;
  }

  ref_holder<T>& operator=(const ref_holder<T>& ref) {
    if (ref.m_ptr == m_ptr) {
      return *this;
    }
    if (ref.m_ptr) {
      ref.m_ptr->hold();
    }
    release();

    m_ptr = ref.m_ptr;
    return *this;
  }

  ref_holder<T>& operator=(ref_holder<T>&& ref) {
    if (ref.m_ptr == m_ptr) {
      return *this;
    }
    release();

    m_ptr = ref.m_ptr;
    ref.m_ptr = nullptr;
    return *this;
  }

  bool operator==(const ref_holder<T>& ref) const { return m_ptr == ref.m_ptr; }
  bool operator!=(const ref_holder<T>& ref) const { return m_ptr != ref.m_ptr; }

  const T* operator->() const { return m_ptr; }
  T* operator->() { return m_ptr; }

  const T& operator*() const { return *m_ptr; }
  T& operator*() { return *m_ptr; }

  const T* get() { return m_ptr; }

  void reset(T* ptr) {
    if (ptr == m_ptr) {
      return;
    }
    if (ptr) {
      ptr->hold();
    }
    release();

    m_ptr = ptr;
  }

  operator bool() const { return m_ptr != nullptr; }

 private:
  void release() {
    if (m_ptr && m_ptr->release()) {
      delete m_ptr;
      m_ptr = nullptr;
    }
  }

  T* m_ptr;
};

struct ref_counted {

  void hold() { m_refs++; }
  bool release() { return (--m_refs == 0); }

 private:
  std::size_t m_refs = 0;
};
}
}
