#include "yaml-cpp/exceptions.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/nodebuilder.h"
#include "nodeevents.h"

namespace YAML {
Node Clone(const Node& node) {
  NodeEvents events(node);
  NodeBuilder builder;
  events.Emit(builder);
  return builder.Root();
}
void Node::ThrowInvalidNode() const {
  throw InvalidNode();
}

namespace detail {
void node::mark_defined() {
  if (is_defined())
    return;

  m_pRef->mark_defined();

  if (m_dependencies) {
    for (auto& it : *m_dependencies) {
      it->mark_defined();
    }
    m_dependencies.reset();
  }
}

void node::add_dependency(node& rhs) {
  if (is_defined())
    rhs.mark_defined();
  else {
    if (!m_dependencies) {
      m_dependencies = std::unique_ptr<nodes>(new nodes);
    }
    m_dependencies->insert(&rhs);
  }
}
}
}
