#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/automaton/ByteRunAutomaton.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/util/BytesRefBuilder.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/automaton/CompiledAutomaton.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Transition.h"
#include  "core/src/java/org/apache/lucene/util/IntsRefBuilder.h"

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
namespace org::apache::lucene::index
{

using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;
using IntsRefBuilder = org::apache::lucene::util::IntsRefBuilder;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using ByteRunAutomaton = org::apache::lucene::util::automaton::ByteRunAutomaton;
using CompiledAutomaton =
    org::apache::lucene::util::automaton::CompiledAutomaton;
using Transition = org::apache::lucene::util::automaton::Transition;

/**
 * A FilteredTermsEnum that enumerates terms based upon what is accepted by a
 * DFA.
 * <p>
 * The algorithm is such:
 * <ol>
 *   <li>As long as matches are successful, keep reading sequentially.
 *   <li>When a match fails, skip to the next string in lexicographic order that
 * does not enter a reject state.
 * </ol>
 * <p>
 * The algorithm does not attempt to actually skip to the next string that is
 * completely accepted. This is not possible when the language accepted by the
 * FSM is not finite (i.e. * operator).
 * </p>
 * @lucene.internal
 */
class AutomatonTermsEnum : public FilteredTermsEnum
{
  GET_CLASS_NAME(AutomatonTermsEnum)
  // a tableized array-based form of the DFA
private:
  const std::shared_ptr<ByteRunAutomaton> runAutomaton;
  // common suffix of the automaton
  const std::shared_ptr<BytesRef> commonSuffixRef;
  // true if the automaton accepts a finite language
  const bool finite;
  // array of sorted transitions for each state, indexed by state number
  const std::shared_ptr<Automaton> automaton;
  // for path tracking: each long records gen when we last
  // visited the state; we use gens to avoid having to clear
  std::deque<int64_t> const visited;
  int64_t curGen = 0;
  // the reference used for seeking forwards through the term dictionary
  const std::shared_ptr<BytesRefBuilder> seekBytesRef =
      std::make_shared<BytesRefBuilder>();
  // true if we are enumerating an infinite portion of the DFA.
  // in this case it is faster to drive the query based on the terms dictionary.
  // when this is true, linearUpperBound indicate the end of range
  // of terms where we should simply do sequential reads instead.
  bool linear = false;
  const std::shared_ptr<BytesRef> linearUpperBound =
      std::make_shared<BytesRef>(10);

  /**
   * Construct an enumerator based upon an automaton, enumerating the specified
   * field, working on a supplied TermsEnum
   *
   * @lucene.experimental
   * @param compiled CompiledAutomaton
   */
public:
  AutomatonTermsEnum(std::shared_ptr<TermsEnum> tenum,
                     std::shared_ptr<CompiledAutomaton> compiled);

  /**
   * Returns true if the term matches the automaton. Also stashes away the term
   * to assist with smart enumeration.
   */
protected:
  AcceptStatus accept(std::shared_ptr<BytesRef> term) override;

  std::shared_ptr<BytesRef>
  nextSeekTerm(std::shared_ptr<BytesRef> term)  override;

private:
  std::shared_ptr<Transition> transition = std::make_shared<Transition>();

  /**
   * Sets the enum to operate in linear fashion, as we have found
   * a looping transition at position: we set an upper bound and
   * act like a TermRangeQuery for this portion of the term space.
   */
  void setLinear(int position);

  const std::shared_ptr<IntsRefBuilder> savedStates =
      std::make_shared<IntsRefBuilder>();

  /**
   * Increments the byte buffer to the next std::wstring in binary order after s that
   * will not put the machine into a reject state. If such a string does not
   * exist, returns false.
   *
   * The correctness of this method depends upon the automaton being
   * deterministic, and having no transitions to dead states.
   *
   * @return true if more possible solutions exist for the DFA
   */
  bool nextString();

  /**
   * Returns the next std::wstring in lexicographic order that will not put
   * the machine into a reject state.
   *
   * This method traverses the DFA from the given position in the std::wstring,
   * starting at the given state.
   *
   * If this cannot satisfy the machine, returns false. This method will
   * walk the minimal path, in lexicographic order, as long as possible.
   *
   * If this method returns false, then there might still be more solutions,
   * it is necessary to backtrack to find out.
   *
   * @param state current non-reject state
   * @param position useful portion of the string
   * @return true if more possible solutions exist for the DFA from this
   *         position
   */
  bool nextString(int state, int position);

  /**
   * Attempts to backtrack thru the string after encountering a dead end
   * at some given position. Returns false if no more possible strings
   * can match.
   *
   * @param position current position in the input std::wstring
   * @return {@code position >= 0} if more possible solutions exist for the DFA
   */
  int backtrack(int position);

protected:
  std::shared_ptr<AutomatonTermsEnum> shared_from_this()
  {
    return std::static_pointer_cast<AutomatonTermsEnum>(
        FilteredTermsEnum::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/index/
