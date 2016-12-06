#pragma once

#include <ios>
#include <string>
#include <array>

#include "stream.h"
#include "stringsource.h"
#include "streamcharsource.h"

#define REGEXP_INLINE inline __attribute__((always_inline))
#define TEST_INLINE inline __attribute__((always_inline))
//#define TEST_INLINE __attribute__((noinline))
//#define TEST_INLINE

namespace YAML {

namespace Exp {


template <std::size_t A, std::size_t... B>
struct static_sum  {
    static const std::size_t value = A + static_sum<B...>::value;
};
template <std::size_t A>
struct static_sum<A> {
    static const std::size_t value = A;
};

template <std::size_t A, std::size_t... C>
struct static_max;

template <std::size_t A>
struct static_max<A> {
    static const std::size_t value = A;
};
template <std::size_t A, std::size_t B, std::size_t... C>
struct static_max<A, B, C...> {
    static const std::size_t value = A >= B ?
        static_max<A, C...>::value : static_max<B, C...>::value;
};


#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)       __builtin_expect(!!(x), 0)

template <char A>
struct Char {
  template <std::size_t N>
  REGEXP_INLINE static int match(std::array<char, N> source, const size_t pos) {
      //return (source[pos] == A) ? 1 : -1;
      if (likely(source[pos] != A)) { return -1;  } else { return 1; }
  }
  static const std::size_t lookahead = 1;
};

template <typename A, typename... B>
struct OR {
  template <std::size_t N>
  REGEXP_INLINE static int match(std::array<char, N> source, const size_t pos) {
    int match = A::match(source, pos);
    if (match >= 0) {
      return match;
    }
    return OR<B...>::match(source, pos);
  }
  static const std::size_t lookahead = static_max<A::lookahead, B::lookahead...>::value;
};

template <typename A>
struct OR<A> {
  template <std::size_t N>
  REGEXP_INLINE static int match(std::array<char, N> source, const size_t pos) {
    return A::match(source, pos);
  }
  static const std::size_t lookahead = A::lookahead;
};

template <typename A, typename... B>
struct SEQ {
  template <std::size_t N>
  REGEXP_INLINE static int match(std::array<char, N> source, const size_t pos) {

    int a = A::match(source, pos);
    if (a < 0) { return -1; }

    int b = SEQ<B...>::match(source, pos + a);
    if (b < 0) { return -1; }

    return a + b;
  }
  static const std::size_t lookahead = static_sum<A::lookahead, B::lookahead...>::value;
};

template <typename A>
struct SEQ<A> {
  template <std::size_t N>
  REGEXP_INLINE static int match(std::array<char, N> source, const size_t pos) {
    return A::match(source, pos);
  }
  static const std::size_t lookahead = A::lookahead;
};

// TODO empty???
template <typename A>
struct NOT {
  template <std::size_t N>
  REGEXP_INLINE static int match(std::array<char, N> source, const size_t pos) {
     return A::match(source, pos) >= 0 ? -1 : 1;
  }
  static const std::size_t lookahead = A::lookahead;
};

template <char A, char Z>
struct Range {
  static_assert(A <= Z, "Invalid Range");
  template <std::size_t N>
  REGEXP_INLINE static int match(std::array<char, N> source, const size_t pos) {
    return (source[pos] < A || source[pos] > Z) ? -1 : 1;
  }
  static const std::size_t lookahead = 1;
};

struct Empty {
  template <std::size_t N>
  REGEXP_INLINE static int match(std::array<char, N> source, const size_t pos) {
    return source[pos] == Stream::eof() ? 0 : -1;
  }
  // REGEXP_INLINE static int match(const StringCharSource& source) {
  //   // the empty regex only is successful on the empty string
  //   // return c == '\0' ? 0 : -1;
  //   return !source ? 0 : -1;
  // }
  static const std::size_t lookahead = 1;
};
template <typename E>
struct Matcher {

  static const std::size_t lookahead = E::lookahead;

  template <std::size_t N>
  TEST_INLINE static int Match(std::array<char, N> source) {
    // return IsValidSource(source) ? Exp::match(source, source[0]) : -1;
    static_assert(N >= E::lookahead, "Passing too small matcher source ");
    return E::match(source, 0);
  }

  template <std::size_t N>
  TEST_INLINE static int Matches(std::array<char, N> source) {
    return !(likely(Match(source) < 0));
  }

  TEST_INLINE static int Match(const Stream& in) {
    return Match(in.LookaheadBuffer(lookahead));
  }

  TEST_INLINE static bool Matches(const Stream& in) {
    return !(likely(Match(in) < 0));
  }

  TEST_INLINE static int Match(const StringCharSource& str) {
    std::array<char, lookahead> source;

    for (size_t i = 0; i < lookahead; i++) {
        source[i] = str[i];
    }

    return Match(source);
  }

  TEST_INLINE static int Matches(const StringCharSource& source) {
    return Match(source) >= 0;
  }

  TEST_INLINE static int Match(const std::string& str) {
    std::array<char, lookahead> source;

    for (size_t i = 0; i < std::min(source.size(), str.size()); i++) {
        source[i] = str[i];
    }
    for (size_t i = std::min(source.size(), str.size()); i < source.size(); i++) {
        source[i] = Stream::eof();
    }

    return Match(source);
  }

  TEST_INLINE static bool Matches(const std::string& str) {
    std::array<char, lookahead> source;

    for (size_t i = 0; i < std::min(source.size(), str.size()); i++) {
        source[i] = str[i];
    }
    for (size_t i = std::min(source.size(), str.size()); i < source.size(); i++) {
        source[i] = Stream::eof();
    }

    return Match(source) >= 0;
  }

  TEST_INLINE static bool Matches(char ch) {
    std::array<char, lookahead> source;
    source[0] = ch;
    if (lookahead > 1) {
        source[1] = Stream::eof();
    }
    return Match(source) >= 0;
  }
};

////////////////////////////////////////////////////////////////////////////////
// Here we store a bunch of expressions for matching different parts of the
// file.

namespace detail {

using Space = Char<' '>;

using Tab = Char<'\t'>;

using Blank = OR < Space, Tab >;

using Break =
  OR < Char<'\n'>,
       SEQ < Char<'\r'>,
             Char<'\n'> >>;

using BlankOrBreak = OR < Blank, Break >;

using Digit = Range<'0', '9'>;

using Alpha =
  OR < Range<'a', 'z'>,
       Range<'A', 'Z'> >;

using AlphaNumeric = OR < Alpha, Digit >;

using Word = OR < AlphaNumeric, Char<'-'> >;

using Hex = OR < Digit, Range<'a','f'>, Range<'A', 'F'>>;

// why not range?
using NotPrintable =
  OR < Char<0>, Char<'\x01'>,
       Char<'\x02'>, Char<'\x03'>,
       Char<'\x04'>, Char<'\x05'>,
       Char<'\x06'>, Char<'\x07'>,
       Char<'\x08'>, Char<'\x0B'>,
       Char<'\x0C'>, Char<'\x7F'>,
       Range<0x0E, 0x1F>,
       SEQ < Char<'\xC2'>,
             OR < Range<'\x80', '\x84'>,
                  Range<'\x86', '\x9F'>>>>;

using Utf8_ByteOrderMark =
  SEQ < Char<'\xEF'>,
        Char<'\xBB'>,
        Char<'\xBF'>>;

using DocStart =
  SEQ < Char<'-'>,
        Char<'-'>,
        Char<'-'>,
        OR < BlankOrBreak, Empty >>;

using DocEnd =
  SEQ < Char<'.'>,
        Char<'.'>,
        Char<'.'>,
        OR < BlankOrBreak, Empty>>;

using BlockEntry =
  SEQ < Char<'-'>,
        OR < BlankOrBreak, Empty >>;

using Key = SEQ<Char<'?'>, BlankOrBreak>;

using KeyInFlow = SEQ<Char<'?'>, BlankOrBreak>;

using Value =
  SEQ < Char<':'>,
        OR < BlankOrBreak, Empty >>;

using ValueInFlow =
  SEQ < Char<':'>,
        OR < BlankOrBreak,
             Char<','>,
             Char<'}'>>>;

using ValueInJSONFlow = Char<':'>;

using Comment = Char<'#'>;

using Anchor = NOT<
  OR < Char<'['>, Char<']'>,
       Char<'{'>, Char<'}'>,
       Char<','>,
       BlankOrBreak>>;

using AnchorEnd =
  OR < Char<'?'>, Char<':'>,
       Char<','>, Char<']'>,
       Char<'}'>, Char<'%'>,
       Char<'@'>, Char<'`'>,
       BlankOrBreak>;

using URI =
  OR < Word,
       Char<'#'>, Char<';'>, Char<'/'>, Char<'?'>, Char<':'>,
       Char<'@'>, Char<'&'>, Char<'='>, Char<'+'>, Char<'$'>,
       Char<','>, Char<'_'>, Char<'.'>, Char<'!'>, Char<'~'>,
       Char<'*'>, Char<'\''>, Char<'('>, Char<')'>, Char<'['>,
       Char<']'>,
       SEQ < Char<'%'>, Hex, Hex>>;

using Tag =
  OR < Word,
       Char<'#'>, Char<';'>, Char<'/'>, Char<'?'>, Char<':'>,
       Char<'@'>, Char<'&'>, Char<'='>, Char<'+'>, Char<'$'>,
       Char<'_'>, Char<'.'>, Char<'~'>, Char<'*'>, Char<'\''>,
       SEQ < Char <'%'>, Hex, Hex>>;

// Plain scalar rules:
// . Cannot start with a blank.
// . Can never start with any of , [ ] { } # & * ! | > \' \" % @ `
// . In the block context - ? : must be not be followed with a space.
// . In the flow context ? is illegal and : and - must not be followed with a
// space.
using PlainScalarCommon =
  NOT < OR < BlankOrBreak,
             Char<','>, Char<'['>, Char<']'>, Char<'{'>, Char<'}'>,
             Char<'#'>, Char<'&'>, Char<'*'>, Char<'!'>, Char<'|'>,
             Char<'>'>, Char<'\''>, Char<'\"'>, Char<'%'>, Char<'@'>,
             Char<'`'>>>;

using PlainScalar =
  NOT < SEQ < OR < Char<'-'>,
                   Char<'?'>,
                   Char<':'>>,
              OR < BlankOrBreak,
                   Empty >>>;

using PlainScalarInFlow =
  NOT < OR < Char<'?'>,
             SEQ < OR < Char<'-'>,
                        Char<':'>>,
                    Blank >>>;
using EndScalar =
  SEQ < Char<':'>,
        OR < BlankOrBreak, Empty >>;

using EndScalarInFlow =
  OR < SEQ < Char<':'>,
             OR < BlankOrBreak,
                  Empty,
                  Char<','>,
                  Char<']'>,
                  Char<'}'>>>,
       Char<','>,
       Char<'?'>,
       Char<'['>,
       Char<']'>,
       Char<'{'>,
       Char<'}'>>;



using ChompIndicator = OR < Char<'+'>, Char<'-'> >;

using Chomp =
  OR < SEQ < ChompIndicator, Digit >,
       SEQ < Digit,ChompIndicator >,
       ChompIndicator,
       Digit>;

} // end detail

using Tab = Matcher<detail::Tab>;
using Blank = Matcher<detail::Blank>;
using Break = Matcher<detail::Break>;
using Digit = Matcher<detail::Digit>;
using BlankOrBreak = Matcher<detail::BlankOrBreak>;
using Word = Matcher<detail::Word>;
using DocStart = Matcher<detail::DocStart>;
using DocEnd = Matcher<detail::DocEnd>;
using BlockEntry = Matcher<detail::BlockEntry>;
using Key = Matcher<detail::Key>;
using KeyInFlow = Matcher<detail::KeyInFlow>;
using Value = Matcher<detail::Value>;
using ValueInFlow = Matcher<detail::ValueInFlow>;
using ValueInJSONFlow = Matcher<detail::ValueInJSONFlow>;
using Comment = Matcher<detail::Comment>;
using Anchor = Matcher<detail::Anchor>;
using AnchorEnd = Matcher<detail::AnchorEnd>;
using URI = Matcher<detail::URI>;
using Tag = Matcher<detail::Tag>;
using PlainScalarCommon = Matcher<detail::PlainScalarCommon>;
using PlainScalar = Matcher<detail::PlainScalar>;
using PlainScalarInFlow = Matcher<detail::PlainScalarInFlow>;
using EscSingleQuote = Matcher<SEQ < Char<'\''>, Char<'\''> >>;
using EscBreak = Matcher<SEQ < Char<'\\'>, detail::Break >>;
using Chomp = Matcher<detail::Chomp>;

std::string Escape(Stream& in);
}

namespace Keys {
const char Directive = '%';
const char FlowSeqStart = '[';
const char FlowSeqEnd = ']';
const char FlowMapStart = '{';
const char FlowMapEnd = '}';
const char FlowEntry = ',';
const char Alias = '*';
const char Anchor = '&';
const char Tag = '!';
const char LiteralScalar = '|';
const char FoldedScalar = '>';
const char VerbatimTagStart = '<';
const char VerbatimTagEnd = '>';
}
}
