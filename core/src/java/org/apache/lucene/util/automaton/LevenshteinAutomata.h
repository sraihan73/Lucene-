#pragma once
#include "stringhelper.h"
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::automaton
{
class ParametricDescription;
}

namespace org::apache::lucene::util::automaton
{
class Automaton;
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

/**
 * Class to construct DFAs that match a word within some edit distance.
 * <p>
 * Implements the algorithm described in:
 * Schulz and Mihov: Fast std::wstring Correction with Levenshtein Automata
 * @lucene.experimental
 */
class LevenshteinAutomata
    : public std::enable_shared_from_this<LevenshteinAutomata>
{
  GET_CLASS_NAME(LevenshteinAutomata)
  /** Maximum edit distance this class can generate an automaton for.
   *  @lucene.internal */
public:
  static constexpr int MAXIMUM_SUPPORTED_DISTANCE = 2;
  /* input word */
  std::deque<int> const word;
  /* the automata alphabet. */
  std::deque<int> const alphabet;
  /* the maximum symbol in the alphabet (e.g. 255 for UTF-8 or 10FFFF for
   * UTF-32) */
  const int alphaMax;

  /* the ranges outside of alphabet */
  std::deque<int> const rangeLower;
  std::deque<int> const rangeUpper;
  int numRanges = 0;

  std::deque<std::shared_ptr<ParametricDescription>> descriptions;

  /**
   * Create a new LevenshteinAutomata for some input std::wstring.
   * Optionally count transpositions as a primitive edit.
   */
  LevenshteinAutomata(const std::wstring &input, bool withTranspositions);

  /**
   * Expert: specify a custom maximum possible symbol
   * (alphaMax); default is Character.MAX_CODE_POINT.
   */
  LevenshteinAutomata(std::deque<int> &word, int alphaMax,
                      bool withTranspositions);

private:
  static std::deque<int> codePoints(const std::wstring &input);

  /**
   * Compute a DFA that accepts all strings within an edit distance of
   * <code>n</code>. <p> All automata have the following properties: <ul>
   * <li>They are deterministic (DFA).
   * <li>There are no transitions to dead states.
   * <li>They are not minimal (some transitions could be combined).
   * </ul>
   */
public:
  virtual std::shared_ptr<Automaton> toAutomaton(int n);

  /**
   * Compute a DFA that accepts all strings within an edit distance of
   * <code>n</code>, matching the specified exact prefix. <p> All automata have
   * the following properties: <ul> <li>They are deterministic (DFA). <li>There
   * are no transitions to dead states. <li>They are not minimal (some
   * transitions could be combined).
   * </ul>
   */
  virtual std::shared_ptr<Automaton> toAutomaton(int n,
                                                 const std::wstring &prefix);

  /**
   * Get the characteristic deque <code>X(x, V)</code>
   * where V is <code>substring(pos, end)</code>
   */
  virtual int getVector(int x, int pos, int end);

  /**
   * A ParametricDescription describes the structure of a Levenshtein DFA for
   * some degree n. <p> There are four components of a parametric description,
   * all parameterized on the length of the word <code>w</code>: <ol> <li>The
   * number of states: {@link #size()} <li>The set of final states: {@link
   * #isAccept(int)} <li>The transition function: {@link #transition(int, int,
   * int)} <li>Minimal boundary function: {@link #getPosition(int)}
   * </ol>
   */
public:
  class ParametricDescription
      : public std::enable_shared_from_this<ParametricDescription>
  {
    GET_CLASS_NAME(ParametricDescription)
  protected:
    const int w;
    const int n;

  private:
    std::deque<int> const minErrors;

  public:
    ParametricDescription(int w, int n, std::deque<int> &minErrors);

    /**
     * Return the number of states needed to compute a Levenshtein DFA
     */
    virtual int size();

    /**
     * Returns true if the <code>state</code> in any Levenshtein DFA is an
     * accept state (final state).
     */
    virtual bool isAccept(int absState);

    /**
     * Returns the position in the input word for a given <code>state</code>.
     * This is the minimal boundary for the state.
     */
    virtual int getPosition(int absState);

    /**
     * Returns the state number for a transition from the given
     * <code>state</code>, assuming <code>position</code> and characteristic
     * deque <code>deque</code>
     */
    virtual int transition(int state, int position, int deque) = 0;

  private:
    static std::deque<int64_t> const MASKS;

  protected:
    virtual int unpack(std::deque<int64_t> &data, int index,
                       int bitsPerValue);
  };
};

} // namespace org::apache::lucene::util::automaton
