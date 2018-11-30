#include "yaml-cpp/proto/protobuf.h"

#include "yaml-cpp/node/detail/memory.h"
#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/node/impl.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/type.h"

#include "pbf_reader.hpp"
#include "pbf_writer.hpp"

#include <functional>
#include <cassert>

namespace YAML {


#ifdef USE_GOOGLE_PROTOBUF
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

proto::Node Protobuf::GDump(const YAML::Node& node) {
    proto::Node root;
    dumpNode(root, node);
    return root;
}

YAML::Node Protobuf::GLoad(const proto::Node& node) {
    detail::shared_memory memory(new detail::memory_ref);
    detail::node& root = memory->create_node();
    loadNode(root, node, memory);
    return YAML::Node(root, memory);
}
#endif

const int32_t tag_node_scalar = 1;
const int32_t tag_node_sequence = 2;
const int32_t tag_node_map = 3;
const int32_t tag_sequence_size = 1;
const int32_t tag_sequence_item = 2;
const int32_t tag_map_size = 1;
const int32_t tag_map_entry = 2;
const int32_t tag_entry_key = 1;
const int32_t tag_entry_val = 2;

using namespace mapbox::util;

void dumpNode(pbf_writer& pNode, const YAML::Node& yNode) {

    switch(yNode.Type()) {
    case YAML::NodeType::Scalar:
        pNode.add_string(tag_node_scalar, yNode.Scalar());
        break;

    case YAML::NodeType::Sequence: {
        auto pos = pNode.open_sub(tag_node_sequence);

        pNode.add_int32(tag_sequence_size, yNode.size());

        for (auto& n : yNode) {
            auto pos = pNode.open_sub(tag_sequence_item);
            dumpNode(pNode, n);
            pNode.close_sub(pos);
        }
        pNode.close_sub(pos);
        break;

    }
    case YAML::NodeType::Map: {
        auto pos = pNode.open_sub(tag_node_map);
        pNode.add_int32(tag_map_size, yNode.size());

        for (auto& n : yNode) {
            if (!n.first.IsScalar()) {
                // not supported yet
                exit(-1);
            }

            auto pos = pNode.open_sub(tag_map_entry);

            pNode.add_string(tag_entry_key, n.first.Scalar());
            {
                auto pos = pNode.open_sub(tag_entry_val);
                dumpNode(pNode, n.second);
                pNode.close_sub(pos);
            }
            pNode.close_sub(pos);
        }

        pNode.close_sub(pos);
        break;
    }
    case YAML::NodeType::Null:
        break;
    default:
        break;
    }
}

std::string Protobuf::Dump(const YAML::Node& node) {
    std::string data;

    pbf_writer root(data);
    dumpNode(root, node);

    return data;
}

static void loadNode(detail::node& yNode, pbf pNode, detail::shared_memory& memory) {

    while (pNode.next()) {

        switch(pNode.tag()) {

        case tag_node_scalar:
            yNode.set_scalar(pNode.get_string());
            break;

        case tag_node_sequence: {
            pbf pSeq = pNode.get_message();
            pSeq.next();

            assert(pSeq.tag() == tag_sequence_size);
            size_t size = pSeq.get_uint32();
            if (size == 0) { break; }

            yNode.set_type(YAML::NodeType::Sequence, size);

            for (size_t i = 0; i < size; i++) {
                pSeq.next();
                assert(pSeq.tag() == tag_sequence_item);

                detail::node& out = memory->create_node();
                yNode.push_back(out, memory);

                loadNode(out, pSeq.get_message(), memory);
            }
            break;
        }
        case tag_node_map: {
            pbf pMap = pNode.get_message();
            pMap.next();

            assert(pMap.tag() == tag_map_size);
            size_t size = pMap.get_uint32();
            if (size == 0) { break; }

            yNode.set_type(YAML::NodeType::Map, size);

            for (size_t i = 0; i < size; i++) {
                pMap.next();
                assert(pMap.tag() == tag_map_entry);

                pbf pEntry = pMap.get_message();

                pEntry.next();
                assert(pEntry.tag() == tag_entry_key);

                detail::node& key = memory->create_node();
                key.set_scalar(pEntry.get_string());

                detail::node& val = memory->create_node();
                yNode.insert(key, val, memory);

                pEntry.next();
                assert(pEntry.tag() == tag_entry_val);

                loadNode(val, pEntry.get_message(), memory);
            }
            break;
        }
        default:
            break;
        }
    }
}

YAML::Node Protobuf::Load(const char* data, size_t length) {
    using namespace YAML::detail;

    shared_memory memory(new memory_ref);
    node& root = memory->create_node();
    pbf pNode(data, length);

    loadNode(root, pNode, memory);

    return YAML::Node(root, memory);
}

} // YAML
