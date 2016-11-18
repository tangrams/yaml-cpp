#pragma once

#include <list>

#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/ptr.h"

namespace YAML {
namespace detail {
class node;

}  // namespace detail
}  // namespace YAML

namespace YAML {
namespace detail {

class YAML_CPP_API memory : public ref_counted {
 public:
  node& create_node();
  void merge(memory&& rhs);

  memory();
  ~memory();

 private:
  typedef std::list<node> Nodes;
  Nodes m_nodes;
};

typedef ref_holder<memory> shared_memory;

class YAML_CPP_API memory_holder : public ref_counted {
 public:
   memory_holder();
  ~memory_holder();

  node& create_node() { return m_pMemory->create_node(); }
  void merge(memory_holder& rhs);

 private:
  shared_memory m_pMemory;

};

typedef ref_holder<memory_holder> shared_memory_holder;

}
}
