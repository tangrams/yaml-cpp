#pragma once

namespace YAML {
struct NodeType {
  enum value : char { Undefined, Null, Scalar, Sequence, Map };
};
}
