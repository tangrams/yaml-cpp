#pragma once

#include <forward_list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/detail/node_iterator.h"
#include "yaml-cpp/node/iterator.h"
#include "yaml-cpp/node/ptr.h"
#include "yaml-cpp/node/type.h"

namespace YAML {
namespace detail {
class node;
}  // namespace detail
}  // namespace YAML

namespace YAML {
namespace detail {
class YAML_CPP_API node_data : public ref_counted {
 public:
  node_data();
  node_data(const node_data&) = delete;
  node_data& operator=(const node_data&) = delete;
  node_data& operator=(node_data&&) = default;

  void mark_defined();
  void set_mark(const Mark& mark);
  void set_type(NodeType::value type);
  void set_tag(const std::string& tag);
  void set_null();
  void set_scalar(const std::string& scalar);
  void set_scalar(std::string&& scalar);
  void set_style(EmitterStyle::value style);

  bool is_defined() const { return m_type != NodeType::Undefined; }
  const Mark& mark() const { return m_mark; }
  NodeType::value type() const {
    return m_type;
  }
  const std::string& scalar() const { return m_scalar; }
  const std::string& tag() const { return m_tag; }
  EmitterStyle::value style() const { return m_style; }

  // size/iterator
  std::size_t size() const;

  const_node_iterator begin() const;
  node_iterator begin();

  const_node_iterator end() const;
  node_iterator end();

  // sequence
  void push_back(node& node, shared_memory pMemory);
  void insert(node& key, node& value, shared_memory pMemory);

  // indexing
  template <typename Key>
  node* get(const Key& key, shared_memory pMemory) const;
  template <typename Key>
  node& get(const Key& key, shared_memory pMemory);
  template <typename Key>
  bool remove(const Key& key, shared_memory pMemory);

  node* get(node& key, shared_memory pMemory) const;
  node& get(node& key, shared_memory pMemory);
  bool remove(node& key, shared_memory pMemory);

  // map
  template <typename Key, typename Value>
  void force_insert(const Key& key, const Value& value, shared_memory pMemory);

 public:
  static std::string empty_scalar;

 private:
  std::size_t compute_seq_size() const;
  std::size_t compute_map_size() const;

  void reset_sequence();
  void reset_map();

  void insert_map_pair(node& key, node& value);
  void convert_to_map(shared_memory pMemory);
  void convert_sequence_to_map(shared_memory pMemory);

  template <typename T>
  static node& convert_to_node(const T& rhs, shared_memory pMemory);

 private:
  // 3 byte
  NodeType::value m_type;
  EmitterStyle::value m_style;
  mutable bool m_hasUndefined;
  // 3 * 4 byte
  Mark m_mark;

  // scalar
  // 32 byte (GCC)
  std::string m_scalar;

  // sequence
  // 24 byte (GCC)
  typedef std::vector<node*> node_seq;
  node_seq m_sequence;

  // map
  // 24 byte (GCC)
  typedef std::vector<std::pair<node*, node*>> node_map;
  node_map m_map;

  // 32 byte (GCC)
  std::string m_tag;

  // 8 byte - pointer
  typedef std::pair<node*, node*> kv_pair;
  typedef std::forward_list<kv_pair> kv_pairs;
  mutable kv_pairs m_undefinedPairs;
};
}
}
