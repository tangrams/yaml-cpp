#include "yaml-cpp/node/detail/memory.h"
#include "yaml-cpp/node/detail/node.h"  // IWYU pragma: keep
#include "yaml-cpp/node/ptr.h"

#include <vector>

namespace YAML {
namespace detail {

struct node_bucket {
    static const size_t size = 512;
    node_bucket(node_bucket* next_, size_t capacity) : next(next_) {
        nodes.reserve(capacity);
    }
    ~node_bucket() {}
    std::vector<node> nodes;
    std::unique_ptr<node_bucket> next = nullptr;
};

node& memory::create_node() {
    node_bucket* insert = m_nodes.get();

    for (node_bucket* b = insert; b; b = b->next.get()) {
        if (b->nodes.size() == b->nodes.capacity()) {
            b = nullptr;
            break;
        }
        insert = b;
    }

    if (insert && insert->nodes.size() < insert->nodes.capacity()) {
        insert->nodes.emplace_back();
        return insert->nodes.back();
    }

    if (!m_nodes) {
        m_nodes = std::unique_ptr<node_bucket>(new node_bucket(nullptr, 8));
    } else {
        m_nodes = std::unique_ptr<node_bucket>(new node_bucket(m_nodes.release(), node_bucket::size));
    }
    m_nodes->nodes.emplace_back();
    return m_nodes->nodes.back();
}


void memory::merge(memory& rhs) {

    if (rhs.m_nodes.get() == m_nodes.get()) {
        return;
    }
    if (!rhs.m_nodes) {
        return;
    }

    if (!m_nodes) {
        m_nodes.reset(rhs.m_nodes.release());
        return;
    }

    // last before filled bucket
    node_bucket* insert = nullptr;
    for (node_bucket* b = m_nodes.get(); b; b = b->next.get()) {
        if (b->nodes.size() == b->nodes.capacity()) {
            break;
        }
        insert = b;
    }

    node_bucket* last = rhs.m_nodes.get();
    for (node_bucket* b = last; b; b = b->next.get()) {
        last = b;
    }

    node_bucket* appendix = nullptr;
    if (insert) {
        appendix = insert->next.release();
        insert->next.reset(rhs.m_nodes.release());
    } else {
        appendix = m_nodes.release();
        m_nodes.reset(rhs.m_nodes.release());

    }

    if (appendix) {
        last->next.reset(appendix);
    }
}

memory::memory() {}
memory::~memory() {}
}
}
