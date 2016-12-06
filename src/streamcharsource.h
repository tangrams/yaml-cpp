#pragma once
#if 0
#include "yaml-cpp/noncopyable.h"
#include <cstddef>
#include "stream.h"

namespace YAML {
class StreamCharSource {
public:
    // Must be large enough for all regexp we use
    static constexpr size_t elements = 8;

    StreamCharSource() = delete;

    StreamCharSource(const Stream& stream)
        : m_available(0),
          m_stream(stream),
          m_streamPos(0) {}

    ~StreamCharSource() {
        // printf("%lu / %lu / %lu\n", reuse, move, fill);
    }

    std::array<char, elements>& buffer() { return m_buffer; }

#define DBG(X) printf(X)

    // size_t reuse = 0;
    // size_t fill = 0;
    // size_t move = 0;

    //__attribute__((noinline))
    void ensure(size_t lookahead) {

        size_t offset = m_stream.pos() - m_streamPos;

        if (offset == 0 && m_available >= lookahead) {
            //reuse++;
            return;
        }
        m_streamPos += offset;

        if (m_available > lookahead + offset) {
            m_available -= offset;

            uint64_t* buf = reinterpret_cast<uint64_t*>(m_buffer.data());
            buf[0] >>= (8 * offset);

            //move++;
            //printf("move from %d - %d - %d\n", (int)offset, (int)(m_available), int(lookahead));
        } else {
            //printf("fill from %d / %lu - %lu - %lu\n", m_stream.pos(), offset, m_available, lookahead);
            m_available = m_stream.init(m_buffer.data(), elements);
            //fill++;
        }
    }

    //__attribute__((noinline))
    void ensure(size_t lookahead, size_t offset) {

        if (offset == 0 && m_available >= lookahead) {
            //reuse++;
            return;
        }
        m_streamPos += offset;

        if (m_available > lookahead + offset) {
            m_available -= offset;

            uint64_t* buf = reinterpret_cast<uint64_t*>(m_buffer.data());
            buf[0] >>= (8 * offset);

            //move++;
            //printf("move from %d - %d - %d\n", (int)offset, (int)(m_available), int(lookahead));
        } else {
            //printf("fill from %d / %lu - %lu - %lu\n", m_stream.pos(), offset, m_available, lookahead);
            m_available = m_stream.init(m_buffer.data(), elements);
            //fill++;
        }
    }


private:
    std::array<char, elements> m_buffer;

    std::size_t m_available;

    const Stream& m_stream;
    std::size_t m_streamPos;

    StreamCharSource& operator=(const StreamCharSource&);  // non-assignable
};
}
#endif
