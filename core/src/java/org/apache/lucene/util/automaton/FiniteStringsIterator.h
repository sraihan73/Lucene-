#pragma once
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"

#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/util/IntsRefBuilder.h"
#include  "core/src/java/org/apache/lucene/util/automaton/PathNode.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Transition.h"

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
 * Iterates all accepted strings.
 *
 * <p>If the {@link Automaton} has cycles then this iterator may throw an {@code
 * IllegalArgumentException}, but this is not guaranteed!
 *
 * <p>Be aware that the iteration order is implementation dependent
 * and may change across releases.
 *
 * <p>If the automaton is not determinized then it's possible this iterator
 * will return duplicates.
 *
 * @lucene.experimental
 */
class FiniteStringsIterator
    : public std::enable_shared_from_this<FiniteStringsIterator>
{
  GET_CLASS_NAME(FiniteStringsIterator)
  /**
   * Empty string.
   */
private:
  static const std::shared_ptr<IntsRef> EMPTY;

  /**
   * Automaton to create finite string from.
   */
  const std::shared_ptr<Automaton> a;

  /**
   * The state where each path should stop or -1 if only accepted states should
   * be final.
   */
  const int endState;

  /**
   * Tracks which states are in the current path, for cycle detection.
   */
  const std::shared_ptr<BitSet> pathStates;

  /**
   * Builder for current finite string.
   */
  const std::shared_ptr<IntsRefBuilder> string;

  /**
   * Stack to hold our current state in the recursion/iteration.
   */
  std::deque<std::shared_ptr<PathNode>> nodes;

  /**
   * Emit empty string?.
   */
  bool emitEmptyString = false;

  /**
   * Constructor.
   *
   * @param a Automaton to create finite string from.
   */
public:
  FiniteStringsIterator(std::shared_ptr<Automaton> a);

  /**
   * Constructor.
   *
   * @param a Automaton to create finite string from.
   * @param startState The starting state for each path.
   * @param endState The state where each path should stop or -1 if only
   * accepted states should be final.
   */
  FiniteStringsIterator(std::shared_ptr<Automaton> a, int startState,
                        int endState);

  /**
   * Generate next finite string.
   * The return value is just valid until the next call of this method!
   *
   * @return Finite string or null, if no more finite strings are available.
   */
  virtual std::shared_ptr<IntsRef> next();

  /**
   * Grow path stack, if required.
   */
private:
  void growStack(int depth);

  /**
   * Nodes for path stack.
   */
private:
  class PathNode : public std::enable_shared_from_this<PathNode>
  {
    GET_CLASS_NAME(PathNode)

    /** Which state the path node ends on, whose
     *  transitions we are enumerating. */
  public:
    int state = 0;

    /** Which state the current transition leads to. */
    int to = 0;

    /** Which transition we are on. */
    int transition = 0;

    /** Which label we are on, in the min-max range of the
     *  current Transition */
    int label = 0;

  private:
    const std::shared_ptr<Transition> t = std::make_shared<Transition>();

  public:
    virtual void resetState(std::shared_ptr<Automaton> a, int state);

    /** Returns next label of current transition, or
     *  advances to next transition and returns its first
     *  label, if current one is exhausted.  If there are
     *  no more transitions, returns -1. */
    virtual int nextLabel(std::shared_ptr<Automaton> a);
  };
};

} // #include  "core/src/java/org/apache/lucene/util/automaton/
