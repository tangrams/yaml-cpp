#pragma once

#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/ptr.h"
#include "yaml-cpp/node/detail/memory.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/node/detail/node_iterator.h"
#include "yaml-cpp/node/detail/iterator.h"
#include <cstddef>
#include <iterator>

// Iterator over Node, pair<Node,Node> (iterator_value)

namespace YAML {
namespace detail {

struct iterator_value : public Node, std::pair<Node, Node> {
  iterator_value()
      : Node(Node::ZombieNode),
        std::pair<Node, Node>(Node(Node::ZombieNode), Node(Node::ZombieNode)) {
  }

  void set_seq(detail::node& rhs, shared_memory& memory) {
    set(rhs, memory);
  }

  void set_map(detail::node& key, detail::node& value, shared_memory& memory) {
    first.set(key, memory);
    second.set(value, memory);
  }
};

template <typename V>
class iterator_base : public std::iterator<std::forward_iterator_tag, V,
                                           std::ptrdiff_t, V*, V> {

 private:
  template <typename>
  friend class iterator_base;
  struct enabler {};
  typedef node_iterator base_type;

  struct proxy {
    explicit proxy(const V& x) : m_ref(x) {}
    V* operator->() { return std::addressof(m_ref); }
    operator V*() { return std::addressof(m_ref); }

    V m_ref;
  };

 public:
  typedef typename iterator_base::value_type value_type;

 public:
  iterator_base() : m_iterator(), m_pMemory(nullptr) {}
  explicit iterator_base(base_type rhs, shared_memory& pMemory)
      : m_iterator(rhs),
        m_pMemory(&pMemory) {}

  template <class W>
  iterator_base(const iterator_base<W>& rhs,
                typename std::enable_if<std::is_convertible<W*, V*>::value,
                                        enabler>::type = enabler())
      : m_iterator(rhs.m_iterator),
        m_pMemory(rhs.m_pMemory) {
  }

  iterator_base<V>& operator++() {
    ++m_iterator;
    return *this;
  }

  iterator_base<V> operator++(int) {
    iterator_base<V> iterator_pre(*this);
    ++(*this);
    return iterator_pre;
  }

  template <typename W>
  bool operator==(const iterator_base<W>& rhs) {
    return m_iterator == rhs.m_iterator;
  }

  template <typename W>
  bool operator!=(const iterator_base<W>& rhs) {
    return m_iterator != rhs.m_iterator;
  }

  value_type& operator*() const {
    const typename base_type::value_type& v = *m_iterator;

    if (!v.pNode) {
      if (!v.first || !v.second) {
        m_elem = iterator_value();
      } else {
        m_elem.set_map(*v.first, *v.second, *m_pMemory);
      }
    } else  {
      m_elem.set_seq(*v, *m_pMemory);
    }
    return m_elem;
  }

  proxy operator->() const { return proxy(**this); }

 private:

  base_type m_iterator;
  shared_memory* m_pMemory;

  mutable iterator_value m_elem;

};
}
}
