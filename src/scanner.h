#pragma once

#include <cstddef>
#include <ios>
#include <map>
#include <memory>
#include <deque>
#include <set>
#include <stack>
#include <string>
#include <list>

#include "scanscalar.h"
#include "stream.h"
#include "token.h"
#include "yaml-cpp/mark.h"

namespace YAML {
class Node;
class RegEx;

/**
 * A scanner transforms a stream of characters into a stream of tokens.
 */
class Scanner {
 public:
  explicit Scanner(std::istream &in);
  explicit Scanner(const std::string &in);
  ~Scanner();

  /** Returns true if there are no more tokens to be read. */
  bool empty();

  /** Removes the next token in the queue. */
  void pop();
  void pop_unsafe() {
    m_freeTokens.splice(m_freeTokens.begin(), m_tokens, m_tokens.begin());
  }
  void push(Token&& token) {
    if (m_freeTokens.empty()) {
      m_tokens.push_back(std::move(token));
    } else {
       m_tokens.splice(m_tokens.end(), m_freeTokens, m_freeTokens.begin());
       m_tokens.back() = std::move(token);
    }
  }
  /** Returns, but does not remove, the next token in the queue. */
  Token &peek();
  Token &peek_unsafe() { return m_tokens.front(); }

  /** Returns the current mark in the input stream. */
  Mark mark() const;

 private:
  struct IndentMarker {
    enum INDENT_TYPE { MAP, SEQ, NONE };
    enum STATUS { VALID, INVALID, UNKNOWN };
    IndentMarker(int column_, INDENT_TYPE type_)
        : column(column_), type(type_), status(VALID), pStartToken(0) {}

    int column;
    INDENT_TYPE type;
    STATUS status;
    Token *pStartToken;
  };

  enum FLOW_MARKER { FLOW_MAP, FLOW_SEQ };

 private:
  // scanning

  /**
   * Scans until there's a valid token at the front of the queue, or the queue
   * is empty. The state can be checked by {@link #empty}, and the next token
   * retrieved by {@link #peek}.
   */
  void EnsureTokensInQueue();

  /**
   * The main scanning function; this method branches out to scan whatever the
   * next token should be.
   */
  void ScanNextToken();

  /** Eats the input stream until it reaches the next token-like thing. */
  void ScanToNextToken();

  /** Sets the initial conditions for starting a stream. */
  void StartStream();

  /** Closes out the stream, finish up, etc. */
  void EndStream();

  Token *PushToken(Token::TYPE type);

  bool InFlowContext() const { return !m_flows.empty(); }
  bool InBlockContext() const { return m_flows.empty(); }
  std::size_t GetFlowLevel() const { return m_flows.size(); }

  Token::TYPE GetStartTokenFor(IndentMarker::INDENT_TYPE type) const;

  /**
   * Pushes an indentation onto the stack, and enqueues the proper token
   * (sequence start or mapping start).
   *
   * @return the indent marker it generates (if any).
   */
  IndentMarker *PushIndentTo(int column, IndentMarker::INDENT_TYPE type);

  /**
   * Pops indentations off the stack until it reaches the current indentation
   * level, and enqueues the proper token each time. Then pops all invalid
   * indentations off.
   */
  void PopIndentToHere();

  /**
   * Pops all indentations (except for the base empty one) off the stack, and
   * enqueues the proper token each time.
   */
  void PopAllIndents();

  /** Pops a single indent, pushing the proper token. */
  void PopIndent();
  int GetTopIndent() const;

  // checking input
  bool CanInsertPotentialSimpleKey() const;
  bool ExistsActiveSimpleKey() const;
  void InsertPotentialSimpleKey();
  void InvalidateSimpleKey();
  bool VerifySimpleKey();
  void PopAllSimpleKeys();

  /**
   * Throws a ParserException with the current token location (if available),
   * and does not parse any more tokens.
   */
  void ThrowParserException(const std::string &msg) const;

  bool IsWhitespaceToBeEaten(char ch);


  struct SimpleKey {
    SimpleKey(const Mark &mark_, std::size_t flowLevel_);

    void Validate();
    void Invalidate();

    Mark mark;
    std::size_t flowLevel;
    IndentMarker *pIndent;
    Token *pMapStart, *pKey;
  };

  // and the tokens
  void ScanDirective();
  void ScanDocStart();
  void ScanDocEnd();
  void ScanBlockSeqStart();
  void ScanBlockMapSTart();
  void ScanBlockEnd();
  void ScanBlockEntry();
  void ScanFlowStart();
  void ScanFlowEnd();
  void ScanFlowEntry();
  void ScanKey();
  void ScanValue();
  void ScanAnchorOrAlias();
  void ScanTag();
  void ScanPlainScalar();
  void ScanQuotedScalar();
  void ScanBlockScalar();

  // scanscalar.cpp
  std::string ScanScalar(ScanScalarParams& info);
  static int MatchScalarEmpty(const Stream& in);
  static int MatchScalarSingleQuoted(const Stream& in);
  static int MatchScalarDoubleQuoted(const Stream& in);
  static int MatchScalarEnd(const Stream& in);
  static int MatchScalarEndInFlow(const Stream& in);
  static bool MatchDocIndicator(const Stream& in);
  static bool CheckDocIndicator(Stream& INPUT, ScanScalarParams& params);

 private:
  // the stream
  Stream INPUT;

  // the output (tokens)
  std::list<Token> m_tokens;
  std::list<Token> m_freeTokens;

  // state info
  bool m_startedStream, m_endedStream;
  bool m_simpleKeyAllowed;
  bool m_canBeJSONFlow;
  std::stack<SimpleKey,std::vector<SimpleKey>> m_simpleKeys;
  std::stack<IndentMarker *,std::vector<IndentMarker *>> m_indents;
  std::vector<std::unique_ptr<IndentMarker>> m_indentRefs;  // for "garbage collection"
  std::stack<FLOW_MARKER, std::vector<FLOW_MARKER>> m_flows;

  std::string m_scalarBuffer;
};
}
