#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class CharsRef;
}

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

using BytesRef = org::apache::lucene::util::BytesRef;
using CharsRef = org::apache::lucene::util::CharsRef;

/**
 * Builds a minimal, deterministic {@link Automaton} that accepts a set of
 * strings. The algorithm requires sorted input data, but is very fast
 * (nearly linear with the input size).
 *
 * @see #build(std::deque)
 * @see Automata#makeStringUnion(std::deque)
 */
class DaciukMihovAutomatonBuilder final
    : public std::enable_shared_from_this<DaciukMihovAutomatonBuilder>
{
  GET_CLASS_NAME(DaciukMihovAutomatonBuilder)

  /**
   * The default constructor is private.  Use static methods directly.
   */
private:
  // C++ TODO: No base class can be determined:
  DaciukMihovAutomatonBuilder(); // super();

  /**
   * DFSA state with <code>char</code> labels on transitions.
   */
private:
  class State final : public std::enable_shared_from_this<State>
  {
    GET_CLASS_NAME(State)

    /** An empty set of labels. */
  private:
    static std::deque<int> const NO_LABELS;

    /** An empty set of states. */
    static std::deque<std::shared_ptr<State>> const NO_STATES;

    /**
     * Labels of outgoing transitions. Indexed identically to {@link #states}.
     * Labels must be sorted lexicographically.
     */
  public:
    std::deque<int> labels = NO_LABELS;

    /**
     * States reachable from outgoing transitions. Indexed identically to
     * {@link #labels}.
     */
    std::deque<std::shared_ptr<State>> states = NO_STATES;

    /**
     * <code>true</code> if this state corresponds to the end of at least one
     * input sequence.
     */
    bool is_final = false;

    /**
     * Returns the target state of a transition leaving this state and labeled
     * with <code>label</code>. If no such transition exists, returns
     * <code>null</code>.
     */
    std::shared_ptr<State> getState(int label);

    /**
     * Two states are equal if:
     * <ul>
     * <li>they have an identical number of outgoing transitions, labeled with
     * the same labels</li>
     * <li>corresponding outgoing transitions lead to the same states (to states
     * with an identical right-language).
     * </ul>
     */
    bool equals(std::any obj) override;

    /**
     * Compute the hash code of the <i>current</i> status of this state.
     */
    virtual int hashCode();

    /**
     * Return <code>true</code> if this state has any children (outgoing
     * transitions).
     */
    bool hasChildren();

    /**
     * Create a new outgoing transition labeled <code>label</code> and return
     * the newly created target state for this transition.
     */
    std::shared_ptr<State> newState(int label);

    /**
     * Return the most recent transitions's target state.
     */
    std::shared_ptr<State> lastChild();

    /**
     * Return the associated state if the most recent transition is labeled with
     * <code>label</code>.
     */
    std::shared_ptr<State> lastChild(int label);

    /**
     * Replace the last added outgoing transition's target state with the given
     * state.
     */
    void replaceLastChild(std::shared_ptr<State> state);

    /**
     * Compare two lists of objects for reference-equality.
     */
  private:
    static bool referenceEquals(std::deque<std::any> &a1,
                                std::deque<std::any> &a2);
  };

  /**
   * A "registry" for state interning.
   */
private:
  std::unordered_map<std::shared_ptr<State>, std::shared_ptr<State>>
      stateRegistry =
          std::unordered_map<std::shared_ptr<State>, std::shared_ptr<State>>();

  /**
   * Root automaton state.
   */
  std::shared_ptr<State> root = std::make_shared<State>();

  /**
   * Previous sequence added to the automaton in {@link #add(CharsRef)}.
   */
  std::shared_ptr<CharsRef> previous;

  /**
   * A comparator used for enforcing sorted UTF8 order, used in assertions only.
   */
  // C++ TODO: Most Java annotations will not have direct C++ equivalents:
  // ORIGINAL LINE: @SuppressWarnings("deprecation") private static final
  // Comparator<org.apache.lucene.util.CharsRef> comparator =
  // org.apache.lucene.util.CharsRef.getUTF16SortedAsUTF8Comparator();
  static const std::shared_ptr<Comparator<std::shared_ptr<CharsRef>>>
      comparator;

  /**
   * Add another character sequence to this automaton. The sequence must be
   * lexicographically larger or equal compared to any previous sequences added
   * to this automaton (the input must be sorted).
   */
public:
  void add(std::shared_ptr<CharsRef> current);

  /**
   * Finalize the automaton and return the root state. No more strings can be
   * added to the builder after this call.
   *
   * @return Root automaton state.
   */
  std::shared_ptr<State> complete();

  /**
   * Internal recursive traversal for conversion.
   */
private:
  static int convert(
      std::shared_ptr<Automaton::Builder> a, std::shared_ptr<State> s,
      std::shared_ptr<IdentityHashMap<std::shared_ptr<State>, int>> visited);

  /**
   * Build a minimal, deterministic automaton from a sorted deque of {@link
   * BytesRef} representing strings in UTF-8. These strings must be
   * binary-sorted.
   */
public:
  static std::shared_ptr<Automaton>
  build(std::shared_ptr<std::deque<std::shared_ptr<BytesRef>>> input);

  /**
   * Copy <code>current</code> into an internal buffer.
   */
private:
  bool setPrevious(std::shared_ptr<CharsRef> current);

  /**
   * Replace last child of <code>state</code> with an already registered state
   * or stateRegistry the last child state.
   */
  void replaceOrRegister(std::shared_ptr<State> state);

  /**
   * Add a suffix of <code>current</code> starting at <code>fromIndex</code>
   * (inclusive) to state <code>state</code>.
   */
  void addSuffix(std::shared_ptr<State> state,
                 std::shared_ptr<std::wstring> current, int fromIndex);
};

} // namespace org::apache::lucene::util::automaton
