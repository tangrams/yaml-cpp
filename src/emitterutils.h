#pragma once

#include <string>

#include "emitterstate.h"
#include "yaml-cpp/emittermanip.h"
#include "yaml-cpp/ostream_wrapper.h"

namespace YAML {
class ostream_wrapper;
}  // namespace YAML

namespace YAML {
class Binary;

struct StringFormat {
  enum value { Plain, SingleQuoted, DoubleQuoted, Literal };
};

namespace Utils {
StringFormat::value ComputeStringFormat(const std::string& str,
                                        EMITTER_MANIP strFormat,
                                        FlowType::value flowType,
                                        bool escapeNonAscii);

bool WriteSingleQuotedString(ostream_wrapper& out, const std::string& str);
bool WriteDoubleQuotedString(ostream_wrapper& out, const std::string& str,
                             bool escapeNonAscii);
bool WriteLiteralString(ostream_wrapper& out, const std::string& str,
                        int indent);
bool WriteChar(ostream_wrapper& out, char ch);
bool WriteComment(ostream_wrapper& out, const std::string& str,
                  int postCommentIndent);
bool WriteAlias(ostream_wrapper& out, const std::string& str);
bool WriteAnchor(ostream_wrapper& out, const std::string& str);
bool WriteTag(ostream_wrapper& out, const std::string& str, bool verbatim);
bool WriteTagWithPrefix(ostream_wrapper& out, const std::string& prefix,
                        const std::string& tag);
bool WriteBinary(ostream_wrapper& out, const Binary& binary);
}
}
