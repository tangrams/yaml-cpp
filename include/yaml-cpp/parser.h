#pragma once

#include <ios>
#include <memory>

#include "yaml-cpp/dll.h"
#include "yaml-cpp/noncopyable.h"

namespace YAML {
class EventHandler;
class Node;
class Scanner;
struct Directives;
struct Token;

class YAML_CPP_API Parser : private noncopyable {
 public:
  Parser();
  Parser(std::istream& in);
  ~Parser();

  operator bool() const;

  void Load(std::istream& in);
  bool HandleNextDocument(EventHandler& eventHandler);

  void PrintTokens(std::ostream& out);

 private:
  void ParseDirectives();
  void HandleDirective(const Token& token);
  void HandleYamlDirective(const Token& token);
  void HandleTagDirective(const Token& token);

 private:
  std::unique_ptr<Scanner> m_pScanner;
  std::unique_ptr<Directives> m_pDirectives;
};
}
