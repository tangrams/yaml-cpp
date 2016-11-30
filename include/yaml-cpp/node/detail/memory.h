#pragma once

#include <set>

#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/ptr.h"

namespace YAML {
namespace detail {
class node;
}  // namespace detail
}  // namespace YAML

namespace YAML {
namespace detail {
class YAML_CPP_API memory {
 public:
  node& create_node();
  void merge(const memory& rhs);

 private:
  typedef std::set<shared_node> Nodes;
  Nodes m_nodes;
};

class YAML_CPP_API memory_holder {
 public:
  memory_holder() : m_pMemory(new memory) {}

  node& create_node() { return m_pMemory->create_node(); }
  void merge(memory_holder& rhs);

 private:
  shared_memory m_pMemory;
};
}
}
