#pragma once

#include "yaml-cpp/dll.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/detail/iterator_fwd.h"
#include "yaml-cpp/node/detail/iterator.h"
#include <list>
#include <utility>
#include <vector>

namespace YAML {
namespace detail {
struct iterator_value : public Node, std::pair<Node, Node> {
  iterator_value() {}
  explicit iterator_value(const Node& rhs)
      : Node(rhs),
        std::pair<Node, Node>(Node(Node::ZombieNode), Node(Node::ZombieNode)) {}
  explicit iterator_value(const Node& key, const Node& value)
      : Node(Node::ZombieNode), std::pair<Node, Node>(key, value) {}

  explicit iterator_value(Node&& rhs)
      : Node(std::move(rhs)),
        std::pair<Node, Node>(Node(Node::ZombieNode), Node(Node::ZombieNode)) {}
  explicit iterator_value(Node&& key, Node&& value)
      : Node(Node::ZombieNode), std::pair<Node, Node>(std::move(key), std::move(value)) {}
};
}
}
