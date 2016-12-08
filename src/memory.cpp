#include "yaml-cpp/node/detail/memory.h"
#include "yaml-cpp/node/detail/node.h"
#include "yaml-cpp/node/ptr.h"

#include <vector>

namespace YAML {
namespace detail {

struct node_bucket {
    static const size_t size = 8;
    node_bucket(node_bucket* next_, size_t capacity) : next(next_) {
        nodes.reserve(capacity);
    }

    ~node_bucket();
    void clear();

    struct value {
        node n;
        std::aligned_storage<sizeof(node_data),alignof(node_data)>::type data;

        value() {
            new (&data) node_data;

            n.set_data(reinterpret_cast<node_data*>(&data));
        }
    };
    std::vector<value> nodes;
    std::unique_ptr<node_bucket> next = nullptr;
};

node_bucket::~node_bucket() {}

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
        return insert->nodes.back().n;
    }

    if (!m_nodes) {
        m_nodes = std::unique_ptr<node_bucket>(new node_bucket(nullptr, 8));
    } else {
        m_nodes = std::unique_ptr<node_bucket>(new node_bucket(m_nodes.release(), node_bucket::size));
    }
    m_nodes->nodes.emplace_back();
    return m_nodes->nodes.back().n;
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
memory::~memory() {
  // Important:
  // First clear all node_data refs
  for (node_bucket* b = m_nodes.get(); b; b = b->next.get()) {
    b->nodes.clear();
  }
  // Then delete buckets
  while (m_nodes) {
    m_nodes = std::move(m_nodes->next);
  }
}
}
}
