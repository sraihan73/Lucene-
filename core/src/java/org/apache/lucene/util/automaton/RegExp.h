#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Automaton;
}

namespace org::apache::lucene::util::automaton
{
class TooComplexToDeterminizeException;
}
namespace org::apache::lucene::util::automaton
{
class AutomatonProvider;
}

/*
 * dk.brics.automaton
 *
 * Copyright (c) 2001-2009 Anders Moeller
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this deque of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this deque of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

namespace org::apache::lucene::util::automaton
{

/**
 * Regular Expression extension to <code>Automaton</code>.
 * <p>
 * Regular expressions are built from the following abstract syntax:
 * <table border=0 summary="description of regular expression grammar">
 * <tr>
 * <td><i>regexp</i></td>
 * <td>::=</td>
 * <td><i>unionexp</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td></td>
 * <td></td>
 * <td></td>
 * </tr>
 *
 * <tr>
 * <td><i>unionexp</i></td>
 * <td>::=</td>
 * <td><i>interexp</i>&nbsp;<tt><b>|</b></tt>&nbsp;<i>unionexp</i></td>
 * <td>(union)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>interexp</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 *
 * <tr>
 * <td><i>interexp</i></td>
 * <td>::=</td>
 * <td><i>concatexp</i>&nbsp;<tt><b>&amp;</b></tt>&nbsp;<i>interexp</i></td>
 * <td>(intersection)</td>
 * <td><small>[OPTIONAL]</small></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>concatexp</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 *
 * <tr>
 * <td><i>concatexp</i></td>
 * <td>::=</td>
 * <td><i>repeatexp</i>&nbsp;<i>concatexp</i></td>
 * <td>(concatenation)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>repeatexp</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 *
 * <tr>
 * <td><i>repeatexp</i></td>
 * <td>::=</td>
 * <td><i>repeatexp</i>&nbsp;<tt><b>?</b></tt></td>
 * <td>(zero or one occurrence)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>repeatexp</i>&nbsp;<tt><b>*</b></tt></td>
 * <td>(zero or more occurrences)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>repeatexp</i>&nbsp;<tt><b>+</b></tt></td>
 * <td>(one or more occurrences)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>repeatexp</i>&nbsp;<tt><b>{</b><i>n</i><b>}</b></tt></td>
 * <td>(<tt><i>n</i></tt> occurrences)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>repeatexp</i>&nbsp;<tt><b>{</b><i>n</i><b>,}</b></tt></td>
 * <td>(<tt><i>n</i></tt> or more occurrences)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>repeatexp</i>&nbsp;<tt><b>{</b><i>n</i><b>,</b><i>m</i><b>}</b></tt></td>
 * <td>(<tt><i>n</i></tt> to <tt><i>m</i></tt> occurrences, including both)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>complexp</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 *
 * <tr>
 * <td><i>complexp</i></td>
 * <td>::=</td>
 * <td><tt><b>~</b></tt>&nbsp;<i>complexp</i></td>
 * <td>(complement)</td>
 * <td><small>[OPTIONAL]</small></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>charclassexp</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 *
 * <tr>
 * <td><i>charclassexp</i></td>
 * <td>::=</td>
 * <td><tt><b>[</b></tt>&nbsp;<i>charclasses</i>&nbsp;<tt><b>]</b></tt></td>
 * <td>(character class)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><tt><b>[^</b></tt>&nbsp;<i>charclasses</i>&nbsp;<tt><b>]</b></tt></td>
 * <td>(negated character class)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>simpleexp</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 *
 * <tr>
 * <td><i>charclasses</i></td>
 * <td>::=</td>
 * <td><i>charclass</i>&nbsp;<i>charclasses</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>charclass</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 *
 * <tr>
 * <td><i>charclass</i></td>
 * <td>::=</td>
 * <td><i>charexp</i>&nbsp;<tt><b>-</b></tt>&nbsp;<i>charexp</i></td>
 * <td>(character range, including end-points)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><i>charexp</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 *
 * <tr>
 * <td><i>simpleexp</i></td>
 * <td>::=</td>
 * <td><i>charexp</i></td>
 * <td></td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><tt><b>.</b></tt></td>
 * <td>(any single character)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><tt><b>#</b></tt></td>
 * <td>(the empty language)</td>
 * <td><small>[OPTIONAL]</small></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><tt><b>@</b></tt></td>
 * <td>(any string)</td>
 * <td><small>[OPTIONAL]</small></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><tt><b>"</b></tt>&nbsp;&lt;Unicode string without double-quotes&gt;&nbsp;
 * <tt><b>"</b></tt></td> <td>(a string)</td> <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><tt><b>(</b></tt>&nbsp;<tt><b>)</b></tt></td>
 * <td>(the empty string)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><tt><b>(</b></tt>&nbsp;<i>unionexp</i>&nbsp;<tt><b>)</b></tt></td>
 * <td>(precedence override)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><tt><b>&lt;</b></tt>&nbsp;&lt;identifier&gt;&nbsp;<tt><b>&gt;</b></tt></td>
 * <td>(named automaton)</td>
 * <td><small>[OPTIONAL]</small></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><tt><b>&lt;</b><i>n</i>-<i>m</i><b>&gt;</b></tt></td>
 * <td>(numerical interval)</td>
 * <td><small>[OPTIONAL]</small></td>
 * </tr>
 *
 * <tr>
 * <td><i>charexp</i></td>
 * <td>::=</td>
 * <td>&lt;Unicode character&gt;</td>
 * <td>(a single non-reserved character)</td>
 * <td></td>
 * </tr>
 * <tr>
 * <td></td>
 * <td>|</td>
 * <td><tt><b>\</b></tt>&nbsp;&lt;Unicode character&gt;&nbsp;</td>
 * <td>(a single character)</td>
 * <td></td>
 * </tr>
 * </table>
 * <p>
 * The productions marked <small>[OPTIONAL]</small> are only allowed if
 * specified by the syntax flags passed to the <code>RegExp</code> constructor.
 * The reserved characters used in the (enabled) syntax must be escaped with
 * backslash (<tt><b>\</b></tt>) or double-quotes (<tt><b>"..."</b></tt>). (In
 * contrast to other regexp syntaxes, this is required also in character
 * classes.) Be aware that dash (<tt><b>-</b></tt>) has a special meaning in
 * <i>charclass</i> expressions. An identifier is a string not containing right
 * angle bracket (<tt><b>&gt;</b></tt>) or dash (<tt><b>-</b></tt>). Numerical
 * intervals are specified by non-negative decimal integers and include both end
 * points, and if <tt><i>n</i></tt> and <tt><i>m</i></tt> have the same number
 * of digits, then the conforming strings must have that length (i.e. prefixed
 * by 0's).
 *
 * @lucene.experimental
 */
class RegExp : public std::enable_shared_from_this<RegExp>
{
  GET_CLASS_NAME(RegExp)

public:
  enum class Kind {
    GET_CLASS_NAME(Kind) REGEXP_UNION,
    REGEXP_CONCATENATION,
    REGEXP_INTERSECTION,
    REGEXP_OPTIONAL,
    REGEXP_REPEAT,
    REGEXP_REPEAT_MIN,
    REGEXP_REPEAT_MINMAX,
    REGEXP_COMPLEMENT,
    REGEXP_CHAR,
    REGEXP_CHAR_RANGE,
    REGEXP_ANYCHAR,
    REGEXP_EMPTY,
    REGEXP_STRING,
    REGEXP_ANYSTRING,
    REGEXP_AUTOMATON,
    REGEXP_INTERVAL
  };

  /**
   * Syntax flag, enables intersection (<tt>&amp;</tt>).
   */
public:
  static constexpr int INTERSECTION = 0x0001;

  /**
   * Syntax flag, enables complement (<tt>~</tt>).
   */
  static constexpr int COMPLEMENT = 0x0002;

  /**
   * Syntax flag, enables empty language (<tt>#</tt>).
   */
  static constexpr int EMPTY = 0x0004;

  /**
   * Syntax flag, enables anystring (<tt>@</tt>).
   */
  static constexpr int ANYSTRING = 0x0008;

  /**
   * Syntax flag, enables named automata (<tt>&lt;</tt>identifier<tt>&gt;</tt>).
   */
  static constexpr int AUTOMATON = 0x0010;

  /**
   * Syntax flag, enables numerical intervals (
   * <tt>&lt;<i>n</i>-<i>m</i>&gt;</tt>).
   */
  static constexpr int INTERVAL = 0x0020;

  /**
   * Syntax flag, enables all optional regexp syntax.
   */
  static constexpr int ALL = 0xffff;

  /**
   * Syntax flag, enables no optional regexp syntax.
   */
  static constexpr int NONE = 0x0000;

private:
  const std::wstring originalString;

public:
  Kind kind = static_cast<Kind>(0);
  std::shared_ptr<RegExp> exp1, exp2;
  std::wstring s;
  int c = 0;
  int min = 0, max = 0, digits = 0;
  int from = 0, to = 0;

  int flags = 0;
  int pos = 0;

  RegExp();

  /**
   * Constructs new <code>RegExp</code> from a string. Same as
   * <code>RegExp(s, ALL)</code>.
   *
   * @param s regexp string
   * @exception IllegalArgumentException if an error occured while parsing the
   *              regular expression
   */
  RegExp(const std::wstring &s) ;

  /**
   * Constructs new <code>RegExp</code> from a string.
   *
   * @param s regexp string
   * @param syntax_flags bool 'or' of optional syntax constructs to be
   *          enabled
   * @exception IllegalArgumentException if an error occured while parsing the
   *              regular expression
   */
  RegExp(const std::wstring &s, int syntax_flags) ;

  /**
   * Constructs new <code>Automaton</code> from this <code>RegExp</code>. Same
   * as <code>toAutomaton(null)</code> (empty automaton map_obj).
   */
  virtual std::shared_ptr<Automaton> toAutomaton();

  /**
   * Constructs new <code>Automaton</code> from this <code>RegExp</code>. The
   * constructed automaton is minimal and deterministic and has no transitions
   * to dead states.
   *
   * @param maxDeterminizedStates maximum number of states in the resulting
   *   automata.  If the automata would need more than this many states
   *   TooComplextToDeterminizeException is thrown.  Higher number require more
   *   space but can process more complex regexes.
   * @exception IllegalArgumentException if this regular expression uses a named
   *              identifier that is not available from the automaton provider
   * @exception TooComplexToDeterminizeException if determinizing this regexp
   *   requires more than maxDeterminizedStates states
   */
  virtual std::shared_ptr<Automaton> toAutomaton(
      int maxDeterminizedStates) throw(std::invalid_argument,
                                       TooComplexToDeterminizeException);

  /**
   * Constructs new <code>Automaton</code> from this <code>RegExp</code>. The
   * constructed automaton is minimal and deterministic and has no transitions
   * to dead states.
   *
   * @param automaton_provider provider of automata for named identifiers
   * @param maxDeterminizedStates maximum number of states in the resulting
   *   automata.  If the automata would need more than this many states
   *   TooComplextToDeterminizeException is thrown.  Higher number require more
   *   space but can process more complex regexes.
   * @exception IllegalArgumentException if this regular expression uses a named
   *   identifier that is not available from the automaton provider
   * @exception TooComplexToDeterminizeException if determinizing this regexp
   *   requires more than maxDeterminizedStates states
   */
  virtual std::shared_ptr<Automaton> toAutomaton(
      std::shared_ptr<AutomatonProvider> automaton_provider,
      int maxDeterminizedStates) throw(std::invalid_argument,
                                       TooComplexToDeterminizeException);

  /**
   * Constructs new <code>Automaton</code> from this <code>RegExp</code>. The
   * constructed automaton is minimal and deterministic and has no transitions
   * to dead states.
   *
   * @param automata a map_obj from automaton identifiers to automata (of type
   *          <code>Automaton</code>).
   * @param maxDeterminizedStates maximum number of states in the resulting
   *   automata.  If the automata would need more than this many states
   *   TooComplexToDeterminizeException is thrown.  Higher number require more
   *   space but can process more complex regexes.
   * @exception IllegalArgumentException if this regular expression uses a named
   *   identifier that does not occur in the automaton map_obj
   * @exception TooComplexToDeterminizeException if determinizing this regexp
   *   requires more than maxDeterminizedStates states
   */
  virtual std::shared_ptr<Automaton> toAutomaton(
      std::unordered_map<std::wstring, std::shared_ptr<Automaton>> &automata,
      int maxDeterminizedStates) throw(std::invalid_argument,
                                       TooComplexToDeterminizeException);

private:
  std::shared_ptr<Automaton> toAutomaton(
      std::unordered_map<std::wstring, std::shared_ptr<Automaton>> &automata,
      std::shared_ptr<AutomatonProvider> automaton_provider,
      int maxDeterminizedStates) throw(std::invalid_argument,
                                       TooComplexToDeterminizeException);

  std::shared_ptr<Automaton> toAutomatonInternal(
      std::unordered_map<std::wstring, std::shared_ptr<Automaton>> &automata,
      std::shared_ptr<AutomatonProvider> automaton_provider,
      int maxDeterminizedStates) ;

  void findLeaves(
      std::shared_ptr<RegExp> exp, Kind kind,
      std::deque<std::shared_ptr<Automaton>> &deque,
      std::unordered_map<std::wstring, std::shared_ptr<Automaton>> &automata,
      std::shared_ptr<AutomatonProvider> automaton_provider,
      int maxDeterminizedStates);

  /**
   * The string that was used to construct the regex.  Compare to toString.
   */
public:
  virtual std::wstring getOriginalString();

  /**
   * Constructs string from parsed regular expression.
   */
  virtual std::wstring toString();

  virtual void toStringBuilder(std::shared_ptr<StringBuilder> b);

  /**
   * Like to string, but more verbose (shows the higherchy more clearly).
   */
  virtual std::wstring toStringTree();

  virtual void toStringTree(std::shared_ptr<StringBuilder> b,
                            const std::wstring &indent);

  /**
   * Returns set of automaton identifiers that occur in this regular expression.
   */
  virtual std::shared_ptr<Set<std::wstring>> getIdentifiers();

  virtual void getIdentifiers(std::shared_ptr<Set<std::wstring>> set);

  static std::shared_ptr<RegExp> makeUnion(std::shared_ptr<RegExp> exp1,
                                           std::shared_ptr<RegExp> exp2);

  static std::shared_ptr<RegExp>
  makeConcatenation(std::shared_ptr<RegExp> exp1, std::shared_ptr<RegExp> exp2);

private:
  static std::shared_ptr<RegExp> makeString(std::shared_ptr<RegExp> exp1,
                                            std::shared_ptr<RegExp> exp2);

  static std::shared_ptr<RegExp> makeIntersection(std::shared_ptr<RegExp> exp1,
                                                  std::shared_ptr<RegExp> exp2);

  static std::shared_ptr<RegExp> makeOptional(std::shared_ptr<RegExp> exp);

  static std::shared_ptr<RegExp> makeRepeat(std::shared_ptr<RegExp> exp);

  static std::shared_ptr<RegExp> makeRepeat(std::shared_ptr<RegExp> exp,
                                            int min);

  static std::shared_ptr<RegExp> makeRepeat(std::shared_ptr<RegExp> exp,
                                            int min, int max);

  static std::shared_ptr<RegExp> makeComplement(std::shared_ptr<RegExp> exp);

  static std::shared_ptr<RegExp> makeChar(int c);

  static std::shared_ptr<RegExp> makeCharRange(int from, int to);

  static std::shared_ptr<RegExp> makeAnyChar();

  static std::shared_ptr<RegExp> makeEmpty();

  static std::shared_ptr<RegExp> makeString(const std::wstring &s);

  static std::shared_ptr<RegExp> makeAnyString();

  static std::shared_ptr<RegExp> makeAutomaton(const std::wstring &s);

  static std::shared_ptr<RegExp> makeInterval(int min, int max, int digits);

  bool peek(const std::wstring &s);

  bool match(int c);

  bool more();

  int next() ;

  bool check(int flag);

public:
  std::shared_ptr<RegExp> parseUnionExp() ;

  std::shared_ptr<RegExp> parseInterExp() ;

  std::shared_ptr<RegExp> parseConcatExp() ;

  std::shared_ptr<RegExp> parseRepeatExp() ;

  std::shared_ptr<RegExp> parseComplExp() ;

  std::shared_ptr<RegExp> parseCharClassExp() ;

  std::shared_ptr<RegExp> parseCharClasses() ;

  std::shared_ptr<RegExp> parseCharClass() ;

  std::shared_ptr<RegExp> parseSimpleExp() ;

  int parseCharExp() ;
};

} // namespace org::apache::lucene::util::automaton
