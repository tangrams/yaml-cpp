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
  void merge(memory& rhs);

  memory();
  ~memory();

 private:
  typedef std::list<node> Nodes;
  Nodes m_nodes;
};

typedef ref_holder<memory> shared_memory;

}
}
