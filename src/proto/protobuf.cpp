#include "yaml-cpp/proto/protobuf.h"

#include "yaml-cpp/node/detail/memory.h"
#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/node/impl.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/type.h"

namespace YAML {



void dumpNode(proto::Node& pNode, const YAML::Node& yNode) {

    switch(yNode.Type()) {
    case YAML::NodeType::Scalar:
        pNode.set_scalar(yNode.Scalar());
        break;

    case YAML::NodeType::Sequence: {
        proto::Sequence& seq = *pNode.mutable_sequence();
        seq.set_size(yNode.size());

        for (auto& n : yNode) {
            dumpNode(*seq.add_item(), n);
        }
        break;
    }
    case YAML::NodeType::Map: {
        proto::Map& map = *pNode.mutable_map();
        map.set_size(yNode.size());

        for (auto& n : yNode) {

            if (!n.first.IsScalar()) {
                // not supported
                exit(-1);
            }
            auto& p = *map.add_entry();

            p.set_key(n.first.Scalar());
            dumpNode(*p.mutable_val(), n.second);
        }
        break;
    }
    case YAML::NodeType::Null:
        break;
    default:
        break;
    }
}

void loadNode(detail::node& yNode, const proto::Node& pNode, detail::shared_memory& memory) {

    if (pNode.has_scalar()) {
        yNode.set_scalar(pNode.scalar());

    } else if (pNode.has_sequence()) {
        auto &seq = pNode.sequence();
        size_t size = seq.size();
        yNode.set_type(YAML::NodeType::Sequence, size);

        for (size_t i = 0; i < size; i++) {
            detail::node& out = memory->create_node();
            yNode.push_back(out, memory);
            loadNode(out, seq.item(i), memory);
        }

    } else if (pNode.has_map()) {
        auto &map = pNode.map();
        size_t size = map.size();
        yNode.set_type(YAML::NodeType::Map, size);

        for (size_t i = 0; i < size; i++) {
            detail::node& key = memory->create_node();
            key.set_scalar(map.entry(i).key());

            detail::node& val = memory->create_node();
            yNode.insert(key, val, memory);
            loadNode(val, map.entry(i).val(), memory);
        }
    }
}

proto::Node Protobuf::Dump(const YAML::Node& node) {
    proto::Node root;
    dumpNode(root, node);
    return root;
}

YAML::Node Protobuf::Load(const proto::Node& node) {
    detail::shared_memory memory(new detail::memory_ref);
    detail::node& root = memory->create_node();
    loadNode(root, node, memory);
    return YAML::Node(root, memory);
}

} // YAML
