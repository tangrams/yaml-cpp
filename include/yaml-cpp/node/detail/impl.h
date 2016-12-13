#pragma once

#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/node/detail/node_data.h"
#include <type_traits>

namespace YAML {
namespace detail {
template <typename Key, typename Enable = void>
struct get_idx {
  static node* get(const std::vector<node*>& /* sequence */,
                   const Key& /* key */, shared_memory& /* pMemory */) {
    return 0;
  }
};

template <typename Key>
struct get_idx<Key,
               typename std::enable_if<std::is_unsigned<Key>::value &&
                                       !std::is_same<Key, bool>::value>::type> {
  static node* get(const std::vector<node*>& sequence, const Key& key,
                   shared_memory& /* pMemory */) {
    return key < sequence.size() ? sequence[key] : 0;
  }

  static node* get(std::vector<node*>& sequence, const Key& key,
                   shared_memory& pMemory) {
    if (key > sequence.size())
      return 0;
    if (key == sequence.size())
      sequence.push_back(&pMemory->create_node());
    return sequence[key];
  }
};

template <typename Key>
struct get_idx<Key, typename std::enable_if<std::is_signed<Key>::value>::type> {
  static node* get(const std::vector<node*>& sequence, const Key& key,
                   shared_memory& pMemory) {
    return key >= 0 ? get_idx<std::size_t>::get(
                          sequence, static_cast<std::size_t>(key), pMemory)
                    : 0;
  }
  static node* get(std::vector<node*>& sequence, const Key& key,
                   shared_memory& pMemory) {
    return key >= 0 ? get_idx<std::size_t>::get(
                          sequence, static_cast<std::size_t>(key), pMemory)
                    : 0;
  }
};

template <typename T>
inline bool node::equals(const T& rhs, shared_memory& pMemory) {
  T lhs;
  if (convert<T>::decode(Node(*this, pMemory), lhs)) {
    return lhs == rhs;
  }
  return false;
}

inline bool node::equals(const char* rhs, shared_memory& pMemory) {
  return equals<std::string>(rhs, pMemory);
}


// indexing
//template <typename Key, typename std::enable_if<is_string_comparable<Key>::value, int>::type>
template <typename Key, typename std::enable_if<std::is_same<Key, detail::string_view>::value, int>::type>
inline node* node_data::get(const Key& key, shared_memory&) const {
  if (m_type == NodeType::Scalar) {
      throw BadSubscript();
  }
  if (m_type != NodeType::Map) {
    return nullptr;
  }
  for (const auto& it : map()) {
    if (it.first->type() == NodeType::Scalar && key.equals(it.first->scalar())) {
      return it.second;
    }
  }
  return nullptr;
}

template <typename Key, typename std::enable_if<!std::is_same<Key, detail::string_view>::value, int>::type>
inline node* node_data::get(const Key& key, shared_memory& pMemory) const {
  switch (m_type) {
    case NodeType::Map:
      break;
    case NodeType::Undefined:
    case NodeType::Null:
      return nullptr;
    case NodeType::Sequence:
      if (node* pNode = get_idx<Key>::get(seq(), key, pMemory)) {
        return pNode;
      }
      return nullptr;
    case NodeType::Scalar:
      throw BadSubscript();
  }
  for (const auto& it : map()) {
    if (it.first->equals(key, pMemory)) {
      return it.second;
    }
  }
  return nullptr;
}

template <typename Key, typename std::enable_if<std::is_same<Key, detail::string_view>::value, int>::type>
inline node& node_data::get(const Key& key, shared_memory& pMemory) {
  if (m_type == NodeType::Scalar) {
    throw BadSubscript();
  }
  if (m_type != NodeType::Map) {
    convert_to_map(pMemory);
  }
  for (const auto& it : map()) {
    if (it.first->type() == NodeType::Scalar && key.equals(it.first->scalar())) {
      return *it.second;
    }
  }

  node& k = convert_to_node(key, pMemory);
  node& v = pMemory->create_node();
  insert_map_pair(k, v);
  return v;
}

template <typename Key, typename std::enable_if<!std::is_same<Key, detail::string_view>::value, int>::type>
inline node& node_data::get(const Key& key, shared_memory& pMemory) {
  switch (m_type) {
    case NodeType::Map:
      break;
    case NodeType::Undefined:
    case NodeType::Null:
      set_type(NodeType::Sequence);
    case NodeType::Sequence:
      if (node* pNode = get_idx<Key>::get(seq(), key, pMemory)) {
        return *pNode;
      }
      convert_to_map(pMemory);
      break;
    case NodeType::Scalar:
      throw BadSubscript();
  }
  for (const auto& it : map()) {
    if (it.first->equals(key, pMemory)) {
      return *it.second;
    }
  }

  node& k = convert_to_node(key, pMemory);
  node& v = pMemory->create_node();
  insert_map_pair(k, v);
  return v;
}

template <typename Key, typename std::enable_if<std::is_same<Key, detail::string_view>::value, int>::type>
inline bool node_data::remove(const Key& key, shared_memory& pMemory) {
  if (m_type != NodeType::Map)
    return false;

  for (node_map::iterator it = map().begin(); it != map().end(); ++it) {
    if (it->first->type() == NodeType::Scalar &&
        key.equals(it->first->scalar())) {
      map().erase(it);
      return true;
    }
  }

  return false;
}

template <typename Key, typename std::enable_if<!std::is_same<Key, detail::string_view>::value, int>::type>
inline bool node_data::remove(const Key& key, shared_memory& pMemory) {
  if (m_type != NodeType::Map)
    return false;

  for (node_map::iterator it = map().begin(); it != map().end(); ++it) {
    if (it->first->equals(key, pMemory)) {
      map().erase(it);
      return true;
    }
  }

  return false;
}

// map
template <typename Key, typename Value>
inline void node_data::force_insert(const Key& key, const Value& value,
                                    shared_memory& pMemory) {
  switch (m_type) {
    case NodeType::Map:
      break;
    case NodeType::Undefined:
    case NodeType::Null:
    case NodeType::Sequence:
      convert_to_map(pMemory);
      break;
    case NodeType::Scalar:
      throw BadInsert();
  }

  node& k = convert_to_node(key, pMemory);
  node& v = convert_to_node(value, pMemory);
  insert_map_pair(k, v);
}

template <typename T>
inline node& node_data::convert_to_node(const T& rhs, shared_memory& pMemory) {
  return *Node(rhs, pMemory).m_pNode;
}
}
}
