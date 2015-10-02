#pragma once

#include "yaml-cpp/dll.h"
#include <memory>

namespace YAML {
namespace detail {
class node;
class node_ref;
class node_data;
class memory;
class memory_holder;

typedef std::shared_ptr<node> shared_node;
typedef std::shared_ptr<node_ref> shared_node_ref;
typedef std::shared_ptr<node_data> shared_node_data;
typedef std::shared_ptr<memory_holder> shared_memory_holder;
typedef std::shared_ptr<memory> shared_memory;
}
}
