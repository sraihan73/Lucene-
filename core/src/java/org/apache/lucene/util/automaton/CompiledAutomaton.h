#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::util::automaton
{
class ByteRunAutomaton;
}
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
class BytesRefBuilder;
}
namespace org::apache::lucene::index
{
class Terms;
}
namespace org::apache::lucene::index
{
class TermsEnum;
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

using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using BytesRef = org::apache::lucene::util::BytesRef;
using BytesRefBuilder = org::apache::lucene::util::BytesRefBuilder;

/**
 * Immutable class holding compiled details for a given
 * Automaton.  The Automaton is deterministic, must not have
 * dead states but is not necessarily minimal.
 *
 * @lucene.experimental
 */
class CompiledAutomaton : public std::enable_shared_from_this<CompiledAutomaton>
{
  GET_CLASS_NAME(CompiledAutomaton)
  /**
   * Automata are compiled into different internal forms for the
   * most efficient execution depending upon the language they accept.
   */
public:
  enum class AUTOMATON_TYPE {
    GET_CLASS_NAME(AUTOMATON_TYPE)
    /** Automaton that accepts no strings. */
    NONE,
    /** Automaton that accepts all possible strings. */
    ALL,
    /** Automaton that accepts only a single fixed string. */
    SINGLE,
    /** Catch-all for any other automata. */
    NORMAL
  };

  /** If simplify is true this will be the "simplified" type; else, this is
   * NORMAL */
public:
  const AUTOMATON_TYPE type;

  /**
   * For {@link AUTOMATON_TYPE#SINGLE} this is the singleton term.
   */
  const std::shared_ptr<BytesRef> term;

  /**
   * Matcher for quickly determining if a byte[] is accepted.
   * only valid for {@link AUTOMATON_TYPE#NORMAL}.
   */
  const std::shared_ptr<ByteRunAutomaton> runAutomaton;

  /**
   * Two dimensional array of transitions, indexed by state
   * number for traversal. The state numbering is consistent with
   * {@link #runAutomaton}.
   * Only valid for {@link AUTOMATON_TYPE#NORMAL}.
   */
  const std::shared_ptr<Automaton> automaton;

  /**
   * Shared common suffix accepted by the automaton. Only valid
   * for {@link AUTOMATON_TYPE#NORMAL}, and only when the
   * automaton accepts an infinite language.  This will be null
   * if the common prefix is length 0.
   */
  const std::shared_ptr<BytesRef> commonSuffixRef;

  /**
   * Indicates if the automaton accepts a finite set of strings.
   * Null if this was not computed.
   * Only valid for {@link AUTOMATON_TYPE#NORMAL}.
   */
  const std::optional<bool> finite;

  /** Which state, if any, accepts all suffixes, else -1. */
  const int sinkState;

  /** Create this, passing simplify=true and finite=null, so that we try
   *  to simplify the automaton and determine if it is finite. */
  CompiledAutomaton(std::shared_ptr<Automaton> automaton);

  /** Returns sink state, if present, else -1. */
private:
  static int findSinkState(std::shared_ptr<Automaton> automaton);

  /** Create this.  If finite is null, we use {@link Operations#isFinite}
   *  to determine whether it is finite.  If simplify is true, we run
   *  possibly expensive operations to determine if the automaton is one
   *  the cases in {@link CompiledAutomaton.AUTOMATON_TYPE}. */
public:
  CompiledAutomaton(std::shared_ptr<Automaton> automaton,
                    std::optional<bool> &finite, bool simplify);

  /** Create this.  If finite is null, we use {@link Operations#isFinite}
   *  to determine whether it is finite.  If simplify is true, we run
   *  possibly expensive operations to determine if the automaton is one
   *  the cases in {@link CompiledAutomaton.AUTOMATON_TYPE}. If simplify
   *  requires determinizing the autaomaton then only maxDeterminizedStates
   *  will be created.  Any more than that will cause a
   *  TooComplexToDeterminizeException.
   */
  CompiledAutomaton(std::shared_ptr<Automaton> automaton,
                    std::optional<bool> &finite, bool simplify,
                    int maxDeterminizedStates, bool isBinary);

private:
  std::shared_ptr<Transition> transition = std::make_shared<Transition>();

  // private static final bool DEBUG = BlockTreeTermsWriter.DEBUG;

  std::shared_ptr<BytesRef> addTail(int state,
                                    std::shared_ptr<BytesRefBuilder> term,
                                    int idx, int leadLabel);

  // TODO: should this take startTerm too?  This way
  // Terms.intersect could forward to this method if type !=
  // NORMAL:
  /** Return a {@link TermsEnum} intersecting the provided {@link Terms}
   *  with the terms accepted by this automaton. */
public:
  virtual std::shared_ptr<TermsEnum>
  getTermsEnum(std::shared_ptr<Terms> terms) ;

  /** Finds largest term accepted by this Automaton, that's
   *  &lt;= the provided input term.  The result is placed in
   *  output; it's fine for output and input to point to
   *  the same bytes.  The returned result is either the
   *  provided output, or null if there is no floor term
   *  (ie, the provided input term is before the first term
   *  accepted by this Automaton). */
  virtual std::shared_ptr<BytesRef>
  floor(std::shared_ptr<BytesRef> input,
        std::shared_ptr<BytesRefBuilder> output);

  virtual int hashCode();

  bool equals(std::any obj) override;
};

} // namespace org::apache::lucene::util::automaton
