#pragma once

#include "yaml.pb.h"

namespace YAML {

class Node;

struct Protobuf {
    static proto::Node Dump(const YAML::Node& node);
    static YAML::Node Load(const proto::Node& node);
};

}
