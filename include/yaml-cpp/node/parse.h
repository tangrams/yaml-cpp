#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "yaml-cpp/dll.h"

namespace YAML {
class Node;

YAML_CPP_API Node Load(const std::string& input);
YAML_CPP_API Node Load(const char* input);
YAML_CPP_API Node Load(std::istream& input);
YAML_CPP_API Node LoadFile(const std::string& filename);

YAML_CPP_API std::vector<Node> LoadAll(const std::string& input);
YAML_CPP_API std::vector<Node> LoadAll(const char* input);
YAML_CPP_API std::vector<Node> LoadAll(std::istream& input);
YAML_CPP_API std::vector<Node> LoadAllFromFile(const std::string& filename);
}
