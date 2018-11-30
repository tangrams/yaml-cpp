#pragma once

#include <stddef.h>
#include <string>

#ifdef USE_GOOGLE_PROTOBUF
#include "yaml.pb.h"
#endif

namespace YAML {

class Node;

struct Protobuf {
#ifdef USE_GOOGLE_PROTOBUF
    static proto::Node GDump(const YAML::Node& node);
    static YAML::Node GLoad(const proto::Node& node);
#endif
    static std::string Dump(const YAML::Node& node);
    static YAML::Node Load(const char* data, size_t length);
};

}
