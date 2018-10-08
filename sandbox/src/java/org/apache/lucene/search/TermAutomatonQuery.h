#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
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
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::index
{
class PostingsEnum;
}
namespace org::apache::lucene::index
{
class TermContext;
}
namespace org::apache::lucene::search::similarities
{
class Similarity;
}
namespace org::apache::lucene::search::similarities
{
class SimWeight;
}
namespace org::apache::lucene::index
{
class Term;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Query;
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
namespace org::apache::lucene::search
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using PostingsEnum = org::apache::lucene::index::PostingsEnum;
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using BytesRef = org::apache::lucene::util::BytesRef;
using Automaton = org::apache::lucene::util::automaton::Automaton;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static
//    org.apache.lucene.util.automaton.Operations.DEFAULT_MAX_DETERMINIZED_STATES;

// TODO
//    - compare perf to PhraseQuery exact and sloppy
//    - optimize: find terms that are in fact MUST (because all paths
//      through the A include that term)
//    - if we ever store posLength in the index, it would be easy[ish]
//      to take it into account here

/** A proximity query that lets you express an automaton, whose
 *  transitions are terms, to match documents.  This is a generalization
 *  of other proximity queries like  {@link PhraseQuery}, {@link
 *  MultiPhraseQuery} and {@link SpanNearQuery}.  It is likely
 *  slow, since it visits any document having any of the terms (i.e. it
 *  acts like a disjunction, not a conjunction like {@link
 *  PhraseQuery}), and then it must merge-sort all positions within each
 *  document to test whether/how many times the automaton matches.
 *
 *  <p>After creating the query, use {@link #createState}, {@link
 *  #setAccept}, {@link #addTransition} and {@link #addAnyTransition} to
 *  build up the automaton.  Once you are done, call {@link #finish} and
 *  then execute the query.
 *
 *  <p>This code is very new and likely has exciting bugs!
 *
 *  @lucene.experimental */

class TermAutomatonQuery : public Query
{
  GET_CLASS_NAME(TermAutomatonQuery)
private:
  const std::wstring field;
  const std::shared_ptr<Automaton::Builder> builder;

public:
  std::shared_ptr<Automaton> det;

private:
  const std::unordered_map<std::shared_ptr<BytesRef>, int> termToID =
      std::unordered_map<std::shared_ptr<BytesRef>, int>();
  const std::unordered_map<int, std::shared_ptr<BytesRef>> idToTerm =
      std::unordered_map<int, std::shared_ptr<BytesRef>>();
  int anyTermID = -1;

public:
  TermAutomatonQuery(const std::wstring &field);

  /** Returns a new state; state 0 is always the initial state. */
  virtual int createState();

  /** Marks the specified state as accept or not. */
  virtual void setAccept(int state, bool accept);

  /** Adds a transition to the automaton. */
  virtual void addTransition(int source, int dest, const std::wstring &term);

  /** Adds a transition to the automaton. */
  virtual void addTransition(int source, int dest,
                             std::shared_ptr<BytesRef> term);

  /** Adds a transition matching any term. */
  virtual void addAnyTransition(int source, int dest);

  /** Call this once you are done adding states/transitions. */
  virtual void finish();

  /**
   * Call this once you are done adding states/transitions.
   * @param maxDeterminizedStates Maximum number of states created when
   *   determinizing the automaton.  Higher numbers allow this operation to
   *   consume more memory but allow more complex automatons.
   */
  virtual void finish(int maxDeterminizedStates);

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  std::wstring toString(const std::wstring &field) override;

private:
  int getTermID(std::shared_ptr<BytesRef> term);

  /** Returns true iff <code>o</code> is equal to this. */
public:
  bool equals(std::any other) override;

private:
  static bool checkFinished(std::shared_ptr<TermAutomatonQuery> q);

  bool equalsTo(std::shared_ptr<TermAutomatonQuery> other);

public:
  virtual int hashCode();

  /** Returns the dot (graphviz) representation of this automaton.
   *  This is extremely useful for visualizing the automaton. */
  virtual std::wstring toDot();

  // TODO: should we impl rewrite to return BooleanQuery of PhraseQuery,
  // when 1) automaton is finite, 2) doesn't use ANY transition, 3) is
  // "small enough"?

public:
  class EnumAndScorer : public std::enable_shared_from_this<EnumAndScorer>
  {
    GET_CLASS_NAME(EnumAndScorer)
  public:
    const int termID;
    const std::shared_ptr<PostingsEnum> posEnum;

    // How many positions left in the current document:
    int posLeft = 0;

    // Current position
    int pos = 0;

    EnumAndScorer(int termID, std::shared_ptr<PostingsEnum> posEnum);
  };

public:
  class TermAutomatonWeight final : public Weight
  {
    GET_CLASS_NAME(TermAutomatonWeight)
  private:
    std::shared_ptr<TermAutomatonQuery> outerInstance;

  public:
    const std::shared_ptr<Automaton> automaton;

  private:
    const std::unordered_map<int, std::shared_ptr<TermContext>> termStates;
    const std::shared_ptr<Similarity::SimWeight> stats;
    const std::shared_ptr<Similarity> similarity;

  public:
    TermAutomatonWeight(
        std::shared_ptr<TermAutomatonQuery> outerInstance,
        std::shared_ptr<Automaton> automaton,
        std::shared_ptr<IndexSearcher> searcher,
        std::unordered_map<int, std::shared_ptr<TermContext>> &termStates,
        float boost) ;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    virtual std::wstring toString();

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

  protected:
    std::shared_ptr<TermAutomatonWeight> shared_from_this()
    {
      return std::static_pointer_cast<TermAutomatonWeight>(
          Weight::shared_from_this());
    }
  };

public:
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

protected:
  std::shared_ptr<TermAutomatonQuery> shared_from_this()
  {
    return std::static_pointer_cast<TermAutomatonQuery>(
        Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
