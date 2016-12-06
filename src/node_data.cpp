#include <assert.h>
#include <iterator>
#include <sstream>

#include "yaml-cpp/exceptions.h"
#include "yaml-cpp/node/detail/memory.h"
#include "yaml-cpp/node/detail/node.h"  // IWYU pragma: keep
#include "yaml-cpp/node/detail/node_data.h"
#include "yaml-cpp/node/detail/node_iterator.h"
#include "yaml-cpp/node/ptr.h"
#include "yaml-cpp/node/type.h"

namespace YAML {
namespace detail {

std::string node_data::empty_scalar;
std::string node_data::tag_none = "";
std::string node_data::tag_other = "!";
std::string node_data::tag_non_plain_scalar = "?";

node_data::node_data()
    : m_type(NodeType::Undefined),
      m_style(EmitterStyle::Default),
      m_hasUndefined(false),
      m_mark(Mark::null_mark()),
      m_tag(&tag_none) {}

void node_data::mark_defined() {
  if (m_type == NodeType::Undefined)
    m_type = NodeType::Null;
}

node_data::~node_data() {
    if (m_tag != &tag_none &&
        m_tag != &tag_other &&
        m_tag != &tag_non_plain_scalar) {
        delete m_tag;
    }
}

void node_data::set_mark(const Mark& mark) { m_mark = mark; }

void node_data::set_type(NodeType::value type) {
  if (type == NodeType::Undefined) {
    m_type = type;
    return;
  }

  if (type == m_type)
    return;

  m_type = type;

  switch (m_type) {
    case NodeType::Null:
      break;
    case NodeType::Scalar:
      m_scalar.clear();
      break;
    case NodeType::Sequence:
      reset_sequence();
      break;
    case NodeType::Map:
      reset_map();
      break;
    case NodeType::Undefined:
      assert(false);
      break;
  }
}

void node_data::set_tag(const std::string& tag) {
  if (m_tag != &tag_none &&
      m_tag != &tag_other &&
      m_tag != &tag_non_plain_scalar) {
    delete m_tag;
  }

  if (tag == "") {
      m_tag = &tag_none;
  } else if (tag == "!") {
      m_tag = &tag_other;
  } else if (tag == "?") {
      m_tag = &tag_non_plain_scalar;
  } else {
      m_tag = new std::string(tag);
  }
}

void node_data::set_style(EmitterStyle::value style) { m_style = style; }

void node_data::set_null() {
  m_type = NodeType::Null;
}

void node_data::set_scalar(const std::string& scalar) {
  m_type = NodeType::Scalar;
  m_scalar = scalar;
}

void node_data::set_scalar(std::string&& scalar) {
  m_type = NodeType::Scalar;
  m_scalar = std::move(scalar);
}

// size/iterator
std::size_t node_data::size() const {
  if (!is_defined())
    return 0;

  switch (m_type) {
    case NodeType::Sequence:
      return compute_seq_size();;
    case NodeType::Map:
      return compute_map_size();
    default:
      return 0;
  }
  return 0;
}

std::size_t node_data::compute_seq_size() const {
  if (!m_hasUndefined) { return m_sequence.size(); }
  std::size_t seqSize = 0;
  while (seqSize < m_sequence.size() && m_sequence[seqSize]->is_defined())
    seqSize++;

  if (seqSize == 0) { m_hasUndefined = false; }
  return seqSize;
}

std::size_t node_data::compute_map_size() const {
  std::size_t seqSize = m_map.size();
  if (!m_hasUndefined) { return seqSize; }

  m_undefinedPairs.remove_if([&](kv_pair& it){
          if (it.first->is_defined() && it.second->is_defined()) {
              return true;
          } else {
              seqSize--;
              return false;
          }
      });
  if (seqSize == 0) { m_hasUndefined = false; }
  return seqSize;
}

const_node_iterator node_data::begin() const {
  if (!is_defined())
    return const_node_iterator();

  switch (m_type) {
    case NodeType::Sequence:
      return const_node_iterator(m_sequence.begin());
    case NodeType::Map:
      return const_node_iterator(m_map.begin(), m_map.end());
    default:
      return const_node_iterator();
  }
}

node_iterator node_data::begin() {
  switch (m_type) {
    case NodeType::Sequence:
      return node_iterator(m_sequence.begin());
    case NodeType::Map:
      return node_iterator(m_map.begin(), m_map.end());
    default:
      return node_iterator();
  }
}

const_node_iterator node_data::end() const {
  switch (m_type) {
    case NodeType::Sequence:
      return const_node_iterator(m_sequence.end());
    case NodeType::Map:
      return const_node_iterator(m_map.end(), m_map.end());
    default:
      return const_node_iterator();
  }
}

node_iterator node_data::end() {
  switch (m_type) {
    case NodeType::Sequence:
      return node_iterator(m_sequence.end());
    case NodeType::Map:
      return node_iterator(m_map.end(), m_map.end());
    default:
      return node_iterator();
  }
}

// sequence
void node_data::push_back(node& node, shared_memory /* pMemory */) {
  if (m_type == NodeType::Undefined || m_type == NodeType::Null) {
    m_type = NodeType::Sequence;
    reset_sequence();
  }

  if (m_type != NodeType::Sequence)
    throw BadPushback();

  m_sequence.push_back(&node);
  m_hasUndefined = true;
}

void node_data::insert(node& key, node& value, shared_memory pMemory) {
  switch (m_type) {
    case NodeType::Map:
      break;
    case NodeType::Undefined:
    case NodeType::Null:
    case NodeType::Sequence:
      convert_to_map(pMemory);
      break;
    case NodeType::Scalar:
      throw BadSubscript();
  }

  insert_map_pair(key, value);
}

// indexing
node* node_data::get(node& key, shared_memory /* pMemory */) const {
  if (m_type != NodeType::Map) {
    return NULL;
  }

  for (node_map::const_iterator it = m_map.begin(); it != m_map.end(); ++it) {
    if (it->first->is(key))
      return it->second;
  }

  return NULL;
}

node& node_data::get(node& key, shared_memory pMemory) {
  switch (m_type) {
    case NodeType::Map:
      break;
    case NodeType::Undefined:
    case NodeType::Null:
    case NodeType::Sequence:
      convert_to_map(pMemory);
      break;
    case NodeType::Scalar:
      throw BadSubscript();
  }

  for (node_map::const_iterator it = m_map.begin(); it != m_map.end(); ++it) {
    if (it->first->is(key))
      return *it->second;
  }

  node& value = pMemory->create_node();
  insert_map_pair(key, value);
  return value;
}

bool node_data::remove(node& key, shared_memory /* pMemory */) {
  if (m_type != NodeType::Map)
    return false;

  for (node_map::iterator it = m_map.begin(); it != m_map.end(); ++it) {
    if (it->first->is(key)) {
      m_map.erase(it);
      return true;
    }
  }

  return false;
}

void node_data::reset_sequence() {
  m_sequence.clear();
}

void node_data::reset_map() {
  m_map.clear();
  m_undefinedPairs.clear();
}

void node_data::insert_map_pair(node& key, node& value) {
  m_map.emplace_back(&key, &value);

  if (!key.is_defined() || !value.is_defined()) {
    m_undefinedPairs.emplace_front(&key, &value);
    m_hasUndefined = true;
  }
}

void node_data::convert_to_map(shared_memory pMemory) {
  switch (m_type) {
    case NodeType::Undefined:
    case NodeType::Null:
      reset_map();
      m_type = NodeType::Map;
      break;
    case NodeType::Sequence:
      convert_sequence_to_map(pMemory);
      break;
    case NodeType::Map:
      break;
    case NodeType::Scalar:
      assert(false);
      break;
  }
}

void node_data::convert_sequence_to_map(shared_memory pMemory) {
  assert(m_type == NodeType::Sequence);

  reset_map();
  for (std::size_t i = 0; i < m_sequence.size(); i++) {
    std::stringstream stream;
    stream << i;

    node& key = pMemory->create_node();
    key.set_scalar(stream.str());
    insert_map_pair(key, *m_sequence[i]);
  }

  reset_sequence();
  m_type = NodeType::Map;
}
}
}
