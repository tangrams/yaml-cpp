#pragma once

#include <string>
#include <iosfwd>

#include "yaml-cpp/dll.h"

namespace YAML {
class Emitter;
class Node;

YAML_CPP_API Emitter& operator<<(Emitter& out, const Node& node);
YAML_CPP_API std::ostream& operator<<(std::ostream& out, const Node& node);

YAML_CPP_API std::string Dump(const Node& node);
}
