#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <deque>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class Transition;
}

namespace org::apache::lucene::util::automaton
{
class Automaton;
}
namespace org::apache::lucene::util
{
class IntsRef;
}
namespace org::apache::lucene::util
{
class IntsRefBuilder;
}

/*
 * Licensed to the Syed Mamun Raihan (sraihan.com) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * sraihan.com licenses this file to You under GPLv3 License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
namespace org::apache::lucene::util::automaton
{

using IntsRef = org::apache::lucene::util::IntsRef;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;

/**
 * Utilities for testing automata.
 * <p>
 * Capable of generating random regular expressions,
 * and automata, and also provides a number of very
 * basic unoptimized implementations (*slow) for testing.
 */
class AutomatonTestUtil : public std::enable_shared_from_this<AutomatonTestUtil>
{
  GET_CLASS_NAME(AutomatonTestUtil)
  /**
   * Default maximum number of states that {@link Operations#determinize} should
   * create.
   */
public:
  static constexpr int DEFAULT_MAX_DETERMINIZED_STATES = 1000000;

  /** Returns random string, including full unicode range. */
  static std::wstring randomRegexp(std::shared_ptr<Random> r);

private:
  static std::wstring randomRegexpString(std::shared_ptr<Random> r);

  /** picks a random int code point, avoiding surrogates;
   * throws IllegalArgumentException if this transition only
   * accepts surrogates */
  static int getRandomCodePoint(std::shared_ptr<Random> r, int min, int max);

  /**
   * Lets you retrieve random strings accepted
   * by an Automaton.
   * <p>
   * Once created, call {@link #getRandomAcceptedString(Random)}
   * to get a new string (in UTF-32 codepoints).
   */
public:
  class RandomAcceptedStrings
      : public std::enable_shared_from_this<RandomAcceptedStrings>
  {
    GET_CLASS_NAME(RandomAcceptedStrings)

  private:
    const std::unordered_map<std::shared_ptr<Transition>, bool> leadsToAccept;
    const std::shared_ptr<Automaton> a;
    std::deque<std::deque<std::shared_ptr<Transition>>> const transitions;

  private:
    class ArrivingTransition
        : public std::enable_shared_from_this<ArrivingTransition>
    {
      GET_CLASS_NAME(ArrivingTransition)
    public:
      const int from;
      const std::shared_ptr<Transition> t;

      ArrivingTransition(int from, std::shared_ptr<Transition> t);
    };

  public:
    RandomAcceptedStrings(std::shared_ptr<Automaton> a);

    virtual std::deque<int> getRandomAcceptedString(std::shared_ptr<Random> r);
  };

private:
  static std::shared_ptr<Automaton>
  randomSingleAutomaton(std::shared_ptr<Random> random);

  /** return a random NFA/DFA for testing */
public:
  static std::shared_ptr<Automaton>
  randomAutomaton(std::shared_ptr<Random> random);

  /**
   * below are original, unoptimized implementations of DFA operations for
   * testing. These are from brics automaton, full license (BSD) below:
   */

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

  /**
   * Simple, original brics implementation of Brzozowski minimize()
   */
  static std::shared_ptr<Automaton>
  minimizeSimple(std::shared_ptr<Automaton> a);

  /**
   * Simple, original brics implementation of determinize()
   */
  static std::shared_ptr<Automaton>
  determinizeSimple(std::shared_ptr<Automaton> a);

  /**
   * Simple, original brics implementation of determinize()
   * Determinizes the given automaton using the given set of initial states.
   */
  static std::shared_ptr<Automaton>
  determinizeSimple(std::shared_ptr<Automaton> a,
                    std::shared_ptr<Set<int>> initialset);

  /**
   * Simple, original implementation of getFiniteStrings.
   *
   * <p>Returns the set of accepted strings, assuming that at most
   * <code>limit</code> strings are accepted. If more than <code>limit</code>
   * strings are accepted, the first limit strings found are returned. If
   * <code>limit</code>&lt;0, then the limit is infinite.
   *
   * <p>This implementation is recursive: it uses one stack
   * frame for each digit in the returned strings (ie, max
   * is the max length returned string).
   */
  static std::shared_ptr<Set<std::shared_ptr<IntsRef>>>
  getFiniteStringsRecursive(std::shared_ptr<Automaton> a, int limit);

  /**
   * Returns the strings that can be produced from the given state, or
   * false if more than <code>limit</code> strings are found.
   * <code>limit</code>&lt;0 means "infinite".
   */
private:
  static bool
  getFiniteStrings(std::shared_ptr<Automaton> a, int s,
                   std::unordered_set<int> &pathstates,
                   std::unordered_set<std::shared_ptr<IntsRef>> &strings,
                   std::shared_ptr<IntsRefBuilder> path, int limit);

  /**
   * Returns true if the language of this automaton is finite.
   * <p>
   * WARNING: this method is slow, it will blow up if the automaton is large.
   * this is only used to test the correctness of our faster implementation.
   */
public:
  static bool isFiniteSlow(std::shared_ptr<Automaton> a);

  /**
   * Checks whether there is a loop containing s. (This is sufficient since
   * there are never transitions to dead states.)
   */
  // TODO: not great that this is recursive... in theory a
  // large automata could exceed java's stack
private:
  static bool isFiniteSlow(std::shared_ptr<Automaton> a, int s,
                           std::unordered_set<int> &path);

  /**
   * Checks that an automaton has no detached states that are unreachable
   * from the initial state.
   */
public:
  static void assertNoDetachedStates(std::shared_ptr<Automaton> a);

  /** Returns true if the automaton is deterministic. */
  static bool isDeterministicSlow(std::shared_ptr<Automaton> a);
};

} // namespace org::apache::lucene::util::automaton
