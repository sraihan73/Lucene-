#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/util/automaton/Automaton.h"
#include  "core/src/java/org/apache/lucene/util/automaton/Transition.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/BytesTermAttribute.h"
#include  "core/src/java/org/apache/lucene/analysis/tokenattributes/PositionIncrementAttribute.h"
#include  "core/src/java/org/apache/lucene/util/IntsRef.h"
#include  "core/src/java/org/apache/lucene/analysis/TokenStream.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/util/automaton/FiniteStringsIterator.h"

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

namespace org::apache::lucene::util::graph
{

using TokenStream = org::apache::lucene::analysis::TokenStream;
using BytesTermAttribute =
    org::apache::lucene::analysis::tokenattributes::BytesTermAttribute;
using PositionIncrementAttribute =
    org::apache::lucene::analysis::tokenattributes::PositionIncrementAttribute;
using Term = org::apache::lucene::index::Term;
using BytesRef = org::apache::lucene::util::BytesRef;
using IntsRef = org::apache::lucene::util::IntsRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;
using Transition = org::apache::lucene::util::automaton::Transition;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

/**
 * Consumes a TokenStream and creates an {@link Automaton} where the transition
 * labels are terms from the {@link TermToBytesRefAttribute}. This class also
 * provides helpers to explore the different paths of the {@link Automaton}.
 */
class GraphTokenStreamFiniteStrings final
    : public std::enable_shared_from_this<GraphTokenStreamFiniteStrings>
{
  GET_CLASS_NAME(GraphTokenStreamFiniteStrings)
private:
  const std::unordered_map<int, std::shared_ptr<BytesRef>> idToTerm =
      std::unordered_map<int, std::shared_ptr<BytesRef>>();
  const std::unordered_map<int, int> idToInc = std::unordered_map<int, int>();
  const std::shared_ptr<Automaton> det;
  const std::shared_ptr<Transition> transition = std::make_shared<Transition>();

private:
  class FiniteStringsTokenStream : public TokenStream
  {
    GET_CLASS_NAME(FiniteStringsTokenStream)
  private:
    std::shared_ptr<GraphTokenStreamFiniteStrings> outerInstance;

    const std::shared_ptr<BytesTermAttribute> termAtt =
        addAttribute(BytesTermAttribute::typeid);
    const std::shared_ptr<PositionIncrementAttribute> posIncAtt =
        addAttribute(PositionIncrementAttribute::typeid);
    const std::shared_ptr<IntsRef> ids;
    const int end;
    int offset = 0;

  public:
    FiniteStringsTokenStream(
        std::shared_ptr<GraphTokenStreamFiniteStrings> outerInstance,
        std::shared_ptr<IntsRef> ids);

    bool incrementToken()  override;

  protected:
    std::shared_ptr<FiniteStringsTokenStream> shared_from_this()
    {
      return std::static_pointer_cast<FiniteStringsTokenStream>(
          org.apache.lucene.analysis.TokenStream::shared_from_this());
    }
  };

public:
  GraphTokenStreamFiniteStrings(std::shared_ptr<TokenStream> in_) throw(
      IOException);

  /**
   * Returns whether the provided state is the start of multiple side paths of
   * different length (eg: new york, ny)
   */
  bool hasSidePath(int state);

  /**
   * Returns the deque of terms that start at the provided state
   */
  std::deque<std::shared_ptr<Term>> getTerms(const std::wstring &field,
                                              int state);

  /**
   * Get all finite strings from the automaton.
   */
  std::shared_ptr<Iterator<std::shared_ptr<TokenStream>>>
  getFiniteStrings() ;

  /**
   * Get all finite strings that start at {@code startState} and end at {@code
   * endState}.
   */
  std::shared_ptr<Iterator<std::shared_ptr<TokenStream>>>
  getFiniteStrings(int startState, int endState) ;

private:
  class IteratorAnonymousInnerClass
      : public std::enable_shared_from_this<IteratorAnonymousInnerClass>,
        public Iterator<std::shared_ptr<TokenStream>>
  {
    GET_CLASS_NAME(IteratorAnonymousInnerClass)
  private:
    std::shared_ptr<GraphTokenStreamFiniteStrings> outerInstance;

    std::shared_ptr<FiniteStringsIterator> it;

  public:
    IteratorAnonymousInnerClass(
        std::shared_ptr<GraphTokenStreamFiniteStrings> outerInstance,
        std::shared_ptr<FiniteStringsIterator> it);

    std::shared_ptr<IntsRef> current;
    bool finished = false;

    bool hasNext();

    std::shared_ptr<TokenStream> next();
  };

  /**
   * Returns the articulation points (or cut vertices) of the graph:
   * https://en.wikipedia.org/wiki/Biconnected_component
   */
public:
  std::deque<int> articulationPoints();

  /**
   * Build an automaton from the provided {@link TokenStream}.
   */
private:
  std::shared_ptr<Automaton>
  build(std::shared_ptr<TokenStream> in_) ;

  /**
   * Gets an integer id for a given term and saves the position increment if
   * needed.
   */
  int getTermID(int incr, int prevIncr, std::shared_ptr<BytesRef> term);

  static void articulationPointsRecurse(std::shared_ptr<Automaton> a, int state,
                                        int d, std::deque<int> &depth,
                                        std::deque<int> &low,
                                        std::deque<int> &parent,
                                        std::shared_ptr<BitSet> visited,
                                        std::deque<int> &points);
};
} // #include  "core/src/java/org/apache/lucene/util/graph/
