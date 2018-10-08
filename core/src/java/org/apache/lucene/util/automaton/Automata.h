#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Automaton;
}

namespace org::apache::lucene::util::automaton
{
class Builder;
}
namespace org::apache::lucene::util
{
class BytesRef;
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

using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * Construction of basic automata.
 *
 * @lucene.experimental
 */
class Automata final : public std::enable_shared_from_this<Automata>
{
  GET_CLASS_NAME(Automata)

private:
  Automata();

  /**
   * Returns a new (deterministic) automaton with the empty language.
   */
public:
  static std::shared_ptr<Automaton> makeEmpty();

  /**
   * Returns a new (deterministic) automaton that accepts only the empty string.
   */
  static std::shared_ptr<Automaton> makeEmptyString();

  /**
   * Returns a new (deterministic) automaton that accepts all strings.
   */
  static std::shared_ptr<Automaton> makeAnyString();

  /**
   * Returns a new (deterministic) automaton that accepts all binary terms.
   */
  static std::shared_ptr<Automaton> makeAnyBinary();

  /**
   * Returns a new (deterministic) automaton that accepts any single codepoint.
   */
  static std::shared_ptr<Automaton> makeAnyChar();

  /** Accept any single character starting from the specified state, returning
   * the new state */
  static int appendAnyChar(std::shared_ptr<Automaton> a, int state);

  /**
   * Returns a new (deterministic) automaton that accepts a single codepoint of
   * the given value.
   */
  static std::shared_ptr<Automaton> makeChar(int c);

  /** Appends the specified character to the specified state, returning a new
   * state. */
  static int appendChar(std::shared_ptr<Automaton> a, int state, int c);

  /**
   * Returns a new (deterministic) automaton that accepts a single codepoint
   * whose value is in the given interval (including both end points).
   */
  static std::shared_ptr<Automaton> makeCharRange(int min, int max);

  /**
   * Constructs sub-automaton corresponding to decimal numbers of length
   * x.substring(n).length().
   */
private:
  static int anyOfRightLength(std::shared_ptr<Automaton::Builder> builder,
                              const std::wstring &x, int n);

  /**
   * Constructs sub-automaton corresponding to decimal numbers of value at least
   * x.substring(n) and length x.substring(n).length().
   */
  static int atLeast(std::shared_ptr<Automaton::Builder> builder,
                     const std::wstring &x, int n,
                     std::shared_ptr<std::deque<int>> initials, bool zeros);

  /**
   * Constructs sub-automaton corresponding to decimal numbers of value at most
   * x.substring(n) and length x.substring(n).length().
   */
  static int atMost(std::shared_ptr<Automaton::Builder> builder,
                    const std::wstring &x, int n);

  /**
   * Constructs sub-automaton corresponding to decimal numbers of value between
   * x.substring(n) and y.substring(n) and of length x.substring(n).length()
   * (which must be equal to y.substring(n).length()).
   */
  static int between(std::shared_ptr<Automaton::Builder> builder,
                     const std::wstring &x, const std::wstring &y, int n,
                     std::shared_ptr<std::deque<int>> initials, bool zeros);

  static bool suffixIsZeros(std::shared_ptr<BytesRef> br, int len);

  /** Creates a new deterministic, minimal automaton accepting
   *  all binary terms in the specified interval.  Note that unlike
   *  {@link #makeDecimalInterval}, the returned automaton is infinite,
   *  because terms behave like floating point numbers leading with
   *  a decimal point.  However, in the special case where min == max,
   *  and both are inclusive, the automata will be finite and accept
   *  exactly one term. */
public:
  static std::shared_ptr<Automaton>
  makeBinaryInterval(std::shared_ptr<BytesRef> min, bool minInclusive,
                     std::shared_ptr<BytesRef> max, bool maxInclusive);

  /**
   * Returns a new automaton that accepts strings representing decimal (base 10)
   * non-negative integers in the given interval.
   *
   * @param min minimal value of interval
   * @param max maximal value of interval (both end points are included in the
   *          interval)
   * @param digits if &gt; 0, use fixed number of digits (strings must be
   * prefixed by 0's to obtain the right length) - otherwise, the number of
   *          digits is not fixed (any number of leading 0s is accepted)
   * @exception IllegalArgumentException if min &gt; max or if numbers in the
   *              interval cannot be expressed with the given fixed number of
   *              digits
   */
  static std::shared_ptr<Automaton>
  makeDecimalInterval(int min, int max,
                      int digits) ;

  /**
   * Returns a new (deterministic) automaton that accepts the single given
   * string.
   */
  static std::shared_ptr<Automaton> makeString(const std::wstring &s);

  /**
   * Returns a new (deterministic) automaton that accepts the single given
   * binary term.
   */
  static std::shared_ptr<Automaton> makeBinary(std::shared_ptr<BytesRef> term);

  /**
   * Returns a new (deterministic) automaton that accepts the single given
   * string from the specified unicode code points.
   */
  static std::shared_ptr<Automaton> makeString(std::deque<int> &word,
                                               int offset, int length);

  /**
   * Returns a new (deterministic and minimal) automaton that accepts the union
   * of the given collection of {@link BytesRef}s representing UTF-8 encoded
   * strings.
   *
   * @param utf8Strings
   *          The input strings, UTF-8 encoded. The collection must be in sorted
   *          order.
   *
   * @return An {@link Automaton} accepting all input strings. The resulting
   *         automaton is codepoint based (full unicode codepoints on
   *         transitions).
   */
  static std::shared_ptr<Automaton> makeStringUnion(
      std::shared_ptr<std::deque<std::shared_ptr<BytesRef>>> utf8Strings);
};

} // namespace org::apache::lucene::util::automaton
