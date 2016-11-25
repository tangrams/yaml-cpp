#include <sstream>

#include "exp.h"
#include "scanner.h"
#include "scanscalar.h"
#include "scantag.h"  // IWYU pragma: keep
#include "tag.h"      // IWYU pragma: keep
#include "token.h"
#include "yaml-cpp/exceptions.h"  // IWYU pragma: keep
#include "yaml-cpp/mark.h"

namespace YAML {
///////////////////////////////////////////////////////////////////////
// Specialization for scanning specific tokens

// Directive
// . Note: no semantic checking is done here (that's for the parser to do)
void Scanner::ScanDirective() {
  std::string name;
  std::vector<std::string> params;

  // pop indents and simple keys
  PopAllIndents();
  PopAllSimpleKeys();

  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = false;

  // store pos and eat indicator
  Token token(Token::DIRECTIVE, INPUT.mark());
  INPUT.eat();

  // read name
  while (INPUT && !Exp::BlankOrBreak::Matches(INPUT))
    token.value += INPUT.get();

  // read parameters
  while (1) {
    // first get rid of whitespace
    while (Exp::Blank::Matches(INPUT))
      INPUT.eat();

    // break on newline or comment
    if (!INPUT || Exp::Break::Matches(INPUT) || Exp::Comment::Matches(INPUT))
      break;

    // now read parameter
    std::string param;
    while (INPUT && !Exp::BlankOrBreak::Matches(INPUT))
      param += INPUT.get();

    token.params.push_back(param);
  }

  m_tokens.push_back(token);
}

// DocStart
void Scanner::ScanDocStart() {
  PopAllIndents();
  PopAllSimpleKeys();
  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = false;

  m_tokens.emplace_back(Token::DOC_START, INPUT.mark());
  // eat after marked
  INPUT.eat(3);
}

// DocEnd
void Scanner::ScanDocEnd() {
  PopAllIndents();
  PopAllSimpleKeys();
  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = false;

  m_tokens.emplace_back(Token::DOC_END, INPUT.mark());
  INPUT.eat(3);
}

// FlowStart
void Scanner::ScanFlowStart() {
  // flows can be simple keys
  InsertPotentialSimpleKey();
  m_simpleKeyAllowed = true;
  m_canBeJSONFlow = false;

  Mark mark = INPUT.mark();
  // eat
  char ch = INPUT.get();
  FLOW_MARKER flowType = (ch == Keys::FlowSeqStart ? FLOW_SEQ : FLOW_MAP);
  m_flows.push(flowType);
  Token::TYPE type =
      (flowType == FLOW_SEQ ? Token::FLOW_SEQ_START : Token::FLOW_MAP_START);
  m_tokens.emplace_back(type, mark);
}

// FlowEnd
void Scanner::ScanFlowEnd() {
  if (InBlockContext())
    throw ParserException(INPUT.mark(), ErrorMsg::FLOW_END);

  // we might have a solo entry in the flow context
  if (InFlowContext()) {
    if (m_flows.top() == FLOW_MAP && VerifySimpleKey())
      m_tokens.emplace_back(Token::VALUE, INPUT.mark());
    else if (m_flows.top() == FLOW_SEQ)
      InvalidateSimpleKey();
  }

  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = true;

  Mark mark = INPUT.mark();
  // eat
  char ch = INPUT.get();

  // check that it matches the start
  FLOW_MARKER flowType = (ch == Keys::FlowSeqEnd ? FLOW_SEQ : FLOW_MAP);
  if (m_flows.top() != flowType)
    throw ParserException(mark, ErrorMsg::FLOW_END);
  m_flows.pop();

  Token::TYPE type = (flowType ? Token::FLOW_SEQ_END : Token::FLOW_MAP_END);
  m_tokens.emplace_back(type, mark);
}

// FlowEntry
void Scanner::ScanFlowEntry() {
  // we might have a solo entry in the flow context
  if (InFlowContext()) {
    if (m_flows.top() == FLOW_MAP && VerifySimpleKey())
      m_tokens.emplace_back(Token::VALUE, INPUT.mark());
    else if (m_flows.top() == FLOW_SEQ)
      InvalidateSimpleKey();
  }

  m_simpleKeyAllowed = true;
  m_canBeJSONFlow = false;

  m_tokens.emplace_back(Token::FLOW_ENTRY, INPUT.mark());
  // eat after marked
  INPUT.eat();
}

// BlockEntry
void Scanner::ScanBlockEntry() {
  // we better be in the block context!
  if (InFlowContext())
    throw ParserException(INPUT.mark(), ErrorMsg::BLOCK_ENTRY);

  // can we put it here?
  if (!m_simpleKeyAllowed)
    throw ParserException(INPUT.mark(), ErrorMsg::BLOCK_ENTRY);

  PushIndentTo(INPUT.column(), IndentMarker::SEQ);
  m_simpleKeyAllowed = true;
  m_canBeJSONFlow = false;

  m_tokens.emplace_back(Token::BLOCK_ENTRY, INPUT.mark());
  // eat after marked
  INPUT.eat();
}

// Key
void Scanner::ScanKey() {
  // handle keys diffently in the block context (and manage indents)
  if (InBlockContext()) {
    if (!m_simpleKeyAllowed)
      throw ParserException(INPUT.mark(), ErrorMsg::MAP_KEY);

    PushIndentTo(INPUT.column(), IndentMarker::MAP);
  }

  // can only put a simple key here if we're in block context
  m_simpleKeyAllowed = InBlockContext();

  m_tokens.emplace_back(Token::KEY, INPUT.mark());
  // eat after marked
  INPUT.eat();
}

// Value
void Scanner::ScanValue() {
  // and check that simple key
  bool isSimpleKey = VerifySimpleKey();
  m_canBeJSONFlow = false;

  if (isSimpleKey) {
    // can't follow a simple key with another simple key (dunno why, though - it
    // seems fine)
    m_simpleKeyAllowed = false;
  } else {
    // handle values diffently in the block context (and manage indents)
    if (InBlockContext()) {
      if (!m_simpleKeyAllowed)
        throw ParserException(INPUT.mark(), ErrorMsg::MAP_VALUE);

      PushIndentTo(INPUT.column(), IndentMarker::MAP);
    }

    // can only put a simple key here if we're in block context
    m_simpleKeyAllowed = InBlockContext();
  }

  m_tokens.emplace_back(Token::VALUE, INPUT.mark());
  // eat after marked
  INPUT.eat();
}

// AnchorOrAlias
void Scanner::ScanAnchorOrAlias() {
  bool alias;
  std::string name;

  // insert a potential simple key
  InsertPotentialSimpleKey();
  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = false;

  // eat the indicator
  Mark mark = INPUT.mark();
  char indicator = INPUT.get();
  alias = (indicator == Keys::Alias);

  // now eat the content
  while (INPUT && Exp::Anchor::Matches(INPUT))
    name += INPUT.get();

  // we need to have read SOMETHING!
  if (name.empty())
    throw ParserException(INPUT.mark(), alias ? ErrorMsg::ALIAS_NOT_FOUND
                                              : ErrorMsg::ANCHOR_NOT_FOUND);

  // and needs to end correctly
  if (INPUT && !Exp::AnchorEnd::Matches(INPUT))
    throw ParserException(INPUT.mark(), alias ? ErrorMsg::CHAR_IN_ALIAS
                                              : ErrorMsg::CHAR_IN_ANCHOR);

  // and we're done
  m_tokens.emplace_back((alias ? Token::ALIAS : Token::ANCHOR), mark, name);
}

// Tag
void Scanner::ScanTag() {
  // insert a potential simple key
  InsertPotentialSimpleKey();
  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = false;

  Token token(Token::TAG, INPUT.mark());

  // eat the indicator
  INPUT.eat();

  if (INPUT && INPUT.peek() == Keys::VerbatimTagStart) {
    std::string tag = ScanVerbatimTag(INPUT);

    token.value = tag;
    token.data = Tag::VERBATIM;
  } else {
    bool canBeHandle;
    token.value = ScanTagHandle(INPUT, canBeHandle);
    if (!canBeHandle && token.value.empty())
      token.data = Tag::NON_SPECIFIC;
    else if (token.value.empty())
      token.data = Tag::SECONDARY_HANDLE;
    else
      token.data = Tag::PRIMARY_HANDLE;

    // is there a suffix?
    if (canBeHandle && INPUT.peek() == Keys::Tag) {
      // eat the indicator
      INPUT.eat();
      token.params.push_back(ScanTagSuffix(INPUT));
      token.data = Tag::NAMED_HANDLE;
    }
  }

  m_tokens.push_back(token);
}


// PlainScalar
void Scanner::ScanPlainScalar() {

  // set up the scanning parameters
  ScanScalarParams params;
  if (InFlowContext()) {
      params.end = ScanScalar::MatchScalarEndInFlow;
  } else {
      params.end = ScanScalar::MatchScalarEnd;
  }

  params.eatEnd = false;
  params.indent = (InFlowContext() ? 0 : GetTopIndent() + 1);
  params.fold = FOLD_FLOW;
  params.eatLeadingWhitespace = true;
  params.trimTrailingSpaces = true;
  params.chomp = STRIP;
  params.onDocIndicator = BREAK;
  params.onTabInIndentation = THROW;

  // insert a potential simple key
  InsertPotentialSimpleKey();

  Mark mark = INPUT.mark();

  m_tokens.emplace_back(Token::PLAIN_SCALAR, mark, ScanScalar::Apply(INPUT, params));

  // can have a simple key only if we ended the scalar by starting a new line
  m_simpleKeyAllowed = params.leadingSpaces;
  m_canBeJSONFlow = false;

  // finally, check and see if we ended on an illegal character
  // if(Exp::IllegalCharInScalar.Matches(INPUT))
  //	throw ParserException(INPUT.mark(), ErrorMsg::CHAR_IN_SCALAR);
}


// QuotedScalar
void Scanner::ScanQuotedScalar() {

  // peek at single or double quote (don't eat because we need to preserve (for
  // the time being) the input position)
  char quote = INPUT.peek();
  bool single = (quote == '\'');

  // setup the scanning parameters
  ScanScalarParams params;
  if (single) {
      params.end = ScanScalar::MatchScalarSingleQuoted;
  } else {
      params.end = ScanScalar::MatchScalarDoubleQuoted;
  }
  params.eatEnd = true;
  params.escape = (single ? '\'' : '\\');
  params.indent = 0;
  params.fold = FOLD_FLOW;
  params.eatLeadingWhitespace = true;
  params.trimTrailingSpaces = false;
  params.chomp = CLIP;
  params.onDocIndicator = THROW;

  // insert a potential simple key
  InsertPotentialSimpleKey();

  Mark mark = INPUT.mark();

  // now eat that opening quote
  INPUT.eat();

  // and scan
  m_tokens.emplace_back(Token::NON_PLAIN_SCALAR, mark, ScanScalar::Apply(INPUT, params));

  m_simpleKeyAllowed = false;
  m_canBeJSONFlow = true;
}



// BlockScalarToken
// . These need a little extra processing beforehand.
// . We need to scan the line where the indicator is (this doesn't count as part
// of the scalar),
//   and then we need to figure out what level of indentation we'll be using.
void Scanner::ScanBlockScalar() {

  ScanScalarParams params;
  params.indent = 1;
  params.detectIndent = true;

  params.end = ScanScalar::MatchScalarEmpty;

  // eat block indicator ('|' or '>')
  Mark mark = INPUT.mark();
  char indicator = INPUT.get();
  params.fold = (indicator == Keys::FoldedScalar ? FOLD_BLOCK : DONT_FOLD);

  // eat chomping/indentation indicators
  params.chomp = CLIP;
  int n = Exp::Chomp::Match(INPUT);
  for (int i = 0; i < n; i++) {
    char ch = INPUT.get();
    if (ch == '+')
      params.chomp = KEEP;
    else if (ch == '-')
      params.chomp = STRIP;
    else if (Exp::Digit::Matches(ch)) {
      if (ch == '0')
        throw ParserException(INPUT.mark(), ErrorMsg::ZERO_INDENT_IN_BLOCK);

      params.indent = ch - '0';
      params.detectIndent = false;
    }
  }

  // now eat whitespace
  while (Exp::Blank::Matches(INPUT))
    INPUT.eat();

  // and comments to the end of the line
  if (Exp::Comment::Matches(INPUT))
    while (INPUT && !Exp::Break::Matches(INPUT))
      INPUT.eat();

  // if it's not a line break, then we ran into a bad character inline
  if (INPUT && !Exp::Break::Matches(INPUT))
    throw ParserException(INPUT.mark(), ErrorMsg::CHAR_IN_BLOCK);

  // set the initial indentation
  if (GetTopIndent() >= 0)
    params.indent += GetTopIndent();

  params.eatLeadingWhitespace = false;
  params.trimTrailingSpaces = false;
  params.onTabInIndentation = THROW;

  m_tokens.emplace_back(Token::NON_PLAIN_SCALAR, mark, ScanScalar::Apply(INPUT, params));

  // simple keys always ok after block scalars (since we're gonna start a new
  // line anyways)
  m_simpleKeyAllowed = true;
  m_canBeJSONFlow = false;

}
}
