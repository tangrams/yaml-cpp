#pragma once

namespace YAML {
struct EmitterNodeType {
  enum value { NoType, Property, Scalar, FlowSeq, BlockSeq, FlowMap, BlockMap };
};
}
