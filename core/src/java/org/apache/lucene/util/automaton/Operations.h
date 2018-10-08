#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <memory>
#include <optional>
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
class Transition;
}
namespace org::apache::lucene::util
{
class IntsRef;
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
using IntsRef = org::apache::lucene::util::IntsRef;

/**
 * Automata operations.
 *
 * @lucene.experimental
 */
class Operations final : public std::enable_shared_from_this<Operations>
{
  GET_CLASS_NAME(Operations)
  /**
   * Default maximum number of states that {@link Operations#determinize} should
   * create.
   */
public:
  static constexpr int DEFAULT_MAX_DETERMINIZED_STATES = 10000;

  /**
   * Maximum level of recursion allowed in recursive operations.
   */
  static constexpr int MAX_RECURSION_LEVEL = 1000;

private:
  Operations();

  /**
   * Returns an automaton that accepts the concatenation of the languages of the
   * given automata.
   * <p>
   * Complexity: linear in total number of states.
   */
public:
  static std::shared_ptr<Automaton> concatenate(std::shared_ptr<Automaton> a1,
                                                std::shared_ptr<Automaton> a2);

  /**
   * Returns an automaton that accepts the concatenation of the languages of the
   * given automata.
   * <p>
   * Complexity: linear in total number of states.
   */
  static std::shared_ptr<Automaton>
  concatenate(std::deque<std::shared_ptr<Automaton>> &l);

  /**
   * Returns an automaton that accepts the union of the empty string and the
   * language of the given automaton.  This may create a dead state.
   * <p>
   * Complexity: linear in number of states.
   */
  static std::shared_ptr<Automaton> optional(std::shared_ptr<Automaton> a);

  /**
   * Returns an automaton that accepts the Kleene star (zero or more
   * concatenated repetitions) of the language of the given automaton. Never
   * modifies the input automaton language.
   * <p>
   * Complexity: linear in number of states.
   */
  static std::shared_ptr<Automaton> repeat(std::shared_ptr<Automaton> a);

  /**
   * Returns an automaton that accepts <code>min</code> or more concatenated
   * repetitions of the language of the given automaton.
   * <p>
   * Complexity: linear in number of states and in <code>min</code>.
   */
  static std::shared_ptr<Automaton> repeat(std::shared_ptr<Automaton> a,
                                           int count);

  /**
   * Returns an automaton that accepts between <code>min</code> and
   * <code>max</code> (including both) concatenated repetitions of the language
   * of the given automaton.
   * <p>
   * Complexity: linear in number of states and in <code>min</code> and
   * <code>max</code>.
   */
  static std::shared_ptr<Automaton> repeat(std::shared_ptr<Automaton> a,
                                           int min, int max);

private:
  static std::shared_ptr<Set<int>> toSet(std::shared_ptr<Automaton> a,
                                         int offset);

  /**
   * Returns a (deterministic) automaton that accepts the complement of the
   * language of the given automaton.
   * <p>
   * Complexity: linear in number of states if already deterministic and
   *  exponential otherwise.
   * @param maxDeterminizedStates maximum number of states determinizing the
   *  automaton can result in.  Set higher to allow more complex queries and
   *  lower to prevent memory exhaustion.
   */
public:
  static std::shared_ptr<Automaton> complement(std::shared_ptr<Automaton> a,
                                               int maxDeterminizedStates);

  /**
   * Returns a (deterministic) automaton that accepts the intersection of the
   * language of <code>a1</code> and the complement of the language of
   * <code>a2</code>. As a side-effect, the automata may be determinized, if not
   * already deterministic.
   * <p>
   * Complexity: quadratic in number of states if a2 already deterministic and
   *  exponential in number of a2's states otherwise.
   */
  static std::shared_ptr<Automaton> minus(std::shared_ptr<Automaton> a1,
                                          std::shared_ptr<Automaton> a2,
                                          int maxDeterminizedStates);

  /**
   * Returns an automaton that accepts the intersection of the languages of the
   * given automata. Never modifies the input automata languages.
   * <p>
   * Complexity: quadratic in number of states.
   */
  static std::shared_ptr<Automaton> intersection(std::shared_ptr<Automaton> a1,
                                                 std::shared_ptr<Automaton> a2);

  /** Returns true if these two automata accept exactly the
   *  same language.  This is a costly computation!  Both automata
   *  must be determinized and have no dead states! */
  static bool sameLanguage(std::shared_ptr<Automaton> a1,
                           std::shared_ptr<Automaton> a2);

  // TODO: move to test-framework?
  /** Returns true if this automaton has any states that cannot
   *  be reached from the initial state or cannot reach an accept state.
   *  Cost is O(numTransitions+numStates). */
  static bool hasDeadStates(std::shared_ptr<Automaton> a);

  // TODO: move to test-framework?
  /** Returns true if there are dead states reachable from an initial state. */
  static bool hasDeadStatesFromInitial(std::shared_ptr<Automaton> a);

  // TODO: move to test-framework?
  /** Returns true if there are dead states that reach an accept state. */
  static bool hasDeadStatesToAccept(std::shared_ptr<Automaton> a);

  /**
   * Returns true if the language of <code>a1</code> is a subset of the language
   * of <code>a2</code>. Both automata must be determinized and must have no
   * dead states. <p> Complexity: quadratic in number of states.
   */
  static bool subsetOf(std::shared_ptr<Automaton> a1,
                       std::shared_ptr<Automaton> a2);

  /**
   * Returns an automaton that accepts the union of the languages of the given
   * automata.
   * <p>
   * Complexity: linear in number of states.
   */
  static std::shared_ptr<Automaton> union_(std::shared_ptr<Automaton> a1,
                                           std::shared_ptr<Automaton> a2);

  /**
   * Returns an automaton that accepts the union of the languages of the given
   * automata.
   * <p>
   * Complexity: linear in number of states.
   */
  static std::shared_ptr<Automaton>
  union_(std::shared_ptr<std::deque<std::shared_ptr<Automaton>>> l);

  // Simple custom ArrayList<Transition>
private:
  class TransitionList final
      : public std::enable_shared_from_this<TransitionList>
  {
    GET_CLASS_NAME(TransitionList)
    // dest, min, max
  public:
    std::deque<int> transitions = std::deque<int>(3);
    int next = 0;

    void add(std::shared_ptr<Transition> t);
  };

  // Holds all transitions that start on this int point, or
  // end at this point-1
private:
  class PointTransitions final
      : public std::enable_shared_from_this<PointTransitions>,
        public Comparable<std::shared_ptr<PointTransitions>>
  {
    GET_CLASS_NAME(PointTransitions)
  public:
    int point = 0;
    const std::shared_ptr<TransitionList> ends =
        std::make_shared<TransitionList>();
    const std::shared_ptr<TransitionList> starts =
        std::make_shared<TransitionList>();

    int compareTo(std::shared_ptr<PointTransitions> other) override;

    void reset(int point);

    bool equals(std::any other) override;

    virtual int hashCode();
  };

private:
  class PointTransitionSet final
      : public std::enable_shared_from_this<PointTransitionSet>
  {
    GET_CLASS_NAME(PointTransitionSet)
  public:
    int count = 0;
    std::deque<std::shared_ptr<PointTransitions>> points =
        std::deque<std::shared_ptr<PointTransitions>>(5);

  private:
    static constexpr int HASHMAP_CUTOVER = 30;
    const std::unordered_map<int, std::shared_ptr<PointTransitions>> map_obj =
        std::unordered_map<int, std::shared_ptr<PointTransitions>>();
    bool useHash = false;

    std::shared_ptr<PointTransitions> next(int point);

    std::shared_ptr<PointTransitions> find(int point);

  public:
    void reset();

    void sort();

    void add(std::shared_ptr<Transition> t);

    virtual std::wstring toString();
  };

  /**
   * Determinizes the given automaton.
   * <p>
   * Worst case complexity: exponential in number of states.
   * @param maxDeterminizedStates Maximum number of states created when
   *   determinizing.  Higher numbers allow this operation to consume more
   *   memory but allow more complex automatons.  Use
   *   DEFAULT_MAX_DETERMINIZED_STATES as a decent default if you don't know
   *   how many to allow.
   * @throws TooComplexToDeterminizeException if determinizing a creates an
   *   automaton with more than maxDeterminizedStates
   */
public:
  static std::shared_ptr<Automaton> determinize(std::shared_ptr<Automaton> a,
                                                int maxDeterminizedStates);

  /**
   * Returns true if the given automaton accepts no strings.
   */
  static bool isEmpty(std::shared_ptr<Automaton> a);

  /**
   * Returns true if the given automaton accepts all strings.  The automaton
   * must be minimized.
   */
  static bool isTotal(std::shared_ptr<Automaton> a);

  /**
   * Returns true if the given automaton accepts all strings for the specified
   * min/max range of the alphabet.  The automaton must be minimized.
   */
  static bool isTotal(std::shared_ptr<Automaton> a, int minAlphabet,
                      int maxAlphabet);

  /**
   * Returns true if the given string is accepted by the automaton.  The input
   * must be deterministic. <p> Complexity: linear in the length of the string.
   * <p>
   * <b>Note:</b> for full performance, use the {@link RunAutomaton} class.
   */
  static bool run(std::shared_ptr<Automaton> a, const std::wstring &s);

  /**
   * Returns true if the given string (expressed as unicode codepoints) is
   * accepted by the automaton.  The input must be deterministic. <p>
   * Complexity: linear in the length of the string.
   * <p>
   * <b>Note:</b> for full performance, use the {@link RunAutomaton} class.
   */
  static bool run(std::shared_ptr<Automaton> a, std::shared_ptr<IntsRef> s);

  /**
   * Returns the set of live states. A state is "live" if an accept state is
   * reachable from it and if it is reachable from the initial state.
   */
private:
  static std::shared_ptr<BitSet> getLiveStates(std::shared_ptr<Automaton> a);

  /** Returns bitset marking states reachable from the initial state. */
  static std::shared_ptr<BitSet>
  getLiveStatesFromInitial(std::shared_ptr<Automaton> a);

  /** Returns bitset marking states that can reach an accept state. */
  static std::shared_ptr<BitSet>
  getLiveStatesToAccept(std::shared_ptr<Automaton> a);

  /**
   * Removes transitions to dead states (a state is "dead" if it is not
   * reachable from the initial state or no accept state is reachable from it.)
   */
public:
  static std::shared_ptr<Automaton>
  removeDeadStates(std::shared_ptr<Automaton> a);

  /**
   * Returns true if the language of this automaton is finite.  The
   * automaton must not have any dead states.
   */
  static bool isFinite(std::shared_ptr<Automaton> a);

  /**
   * Checks whether there is a loop containing state. (This is sufficient since
   * there are never transitions to dead states.)
   */
  // TODO: not great that this is recursive... in theory a
  // large automata could exceed java's stack so the maximum level of recursion
  // is bounded to 1000
private:
  static bool isFinite(std::shared_ptr<Transition> scratch,
                       std::shared_ptr<Automaton> a, int state,
                       std::shared_ptr<BitSet> path,
                       std::shared_ptr<BitSet> visited, int level);

  /**
   * Returns the longest string that is a prefix of all accepted strings and
   * visits each state at most once.  The automaton must be deterministic.
   *
   * @return common prefix, which can be an empty (length 0) std::wstring (never null)
   */
public:
  static std::wstring getCommonPrefix(std::shared_ptr<Automaton> a);

  // TODO: this currently requites a determinized machine,
  // but it need not -- we can speed it up by walking the
  // NFA instead.  it'd still be fail fast.
  /**
   * Returns the longest BytesRef that is a prefix of all accepted strings and
   * visits each state at most once.  The automaton must be deterministic.
   *
   * @return common prefix, which can be an empty (length 0) BytesRef (never
   * null)
   */
  static std::shared_ptr<BytesRef>
  getCommonPrefixBytesRef(std::shared_ptr<Automaton> a);

  /** If this automaton accepts a single input, return it.  Else, return null.
   *  The automaton must be deterministic. */
  static std::shared_ptr<IntsRef> getSingleton(std::shared_ptr<Automaton> a);

  /**
   * Returns the longest BytesRef that is a suffix of all accepted strings.
   * Worst case complexity: exponential in number of states (this calls
   * determinize).
   * @param maxDeterminizedStates maximum number of states determinizing the
   *  automaton can result in.  Set higher to allow more complex queries and
   *  lower to prevent memory exhaustion.
   * @return common suffix, which can be an empty (length 0) BytesRef (never
   * null)
   */
  static std::shared_ptr<BytesRef>
  getCommonSuffixBytesRef(std::shared_ptr<Automaton> a,
                          int maxDeterminizedStates);

private:
  static void reverseBytes(std::shared_ptr<BytesRef> ref);

  /** Returns an automaton accepting the reverse language. */
public:
  static std::shared_ptr<Automaton> reverse(std::shared_ptr<Automaton> a);

  /** Reverses the automaton, returning the new initial states. */
  static std::shared_ptr<Automaton>
  reverse(std::shared_ptr<Automaton> a,
          std::shared_ptr<Set<int>> initialStates);

  /** Returns a new automaton accepting the same language with added
   *  transitions to a dead state so that from every state and every label
   *  there is a transition. */
  static std::shared_ptr<Automaton> totalize(std::shared_ptr<Automaton> a);

  /** Returns the topological sort of all states reachable from
   *  the initial state.  Behavior is undefined if this
   *  automaton has cycles.  CPU cost is O(numTransitions),
   *  and the implementation is recursive so an automaton
   *  matching long strings may exhaust the java stack. */
  static std::deque<int> topoSortStates(std::shared_ptr<Automaton> a);

  // TODO: not great that this is recursive... in theory a
  // large automata could exceed java's stack so the maximum level of recursion
  // is bounded to 1000
private:
  static int topoSortStatesRecurse(std::shared_ptr<Automaton> a,
                                   std::shared_ptr<BitSet> visited,
                                   std::deque<int> &states, int upto,
                                   int state, int level);
};

} // namespace org::apache::lucene::util::automaton
