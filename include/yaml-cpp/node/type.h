#pragma once

namespace YAML {
struct NodeType {
  enum value { Undefined, Null, Scalar, Sequence, Map };
};
}
