#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class Term;
}

namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Query;
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
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Matches;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search::similarities
{
class SimScorer;
}
namespace org::apache::lucene::search
{
class DisiWrapper;
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
using Term = org::apache::lucene::index::Term;
using TermContext = org::apache::lucene::index::TermContext;
using Similarity = org::apache::lucene::search::similarities::Similarity;
using SimScorer =
    org::apache::lucene::search::similarities::Similarity::SimScorer;

/**
 * A query that treats multiple terms as synonyms.
 * <p>
 * For scoring purposes, this query tries to score the terms as if you
 * had indexed them as one term: it will match any of the terms but
 * only invoke the similarity a single time, scoring the sum of all
 * term frequencies for the document.
 */
class SynonymQuery final : public Query
{
  GET_CLASS_NAME(SynonymQuery)
private:
  std::deque<std::shared_ptr<Term>> const terms;

  /**
   * Creates a new SynonymQuery, matching any of the supplied terms.
   * <p>
   * The terms must all have the same field.
   */
public:
  SynonymQuery(std::deque<Term> &terms);

  std::deque<std::shared_ptr<Term>> getTerms();

  std::wstring toString(const std::wstring &field) override;

  virtual int hashCode();

  bool equals(std::any other) override;

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

public:
  class SynonymWeight : public Weight
  {
    GET_CLASS_NAME(SynonymWeight)
  private:
    std::shared_ptr<SynonymQuery> outerInstance;

    std::deque<std::shared_ptr<TermContext>> const termContexts;
    const std::shared_ptr<Similarity> similarity;
    const std::shared_ptr<Similarity::SimWeight> simWeight;

  public:
    SynonymWeight(std::shared_ptr<SynonymQuery> outerInstance,
                  std::shared_ptr<Query> query,
                  std::shared_ptr<IndexSearcher> searcher,
                  float boost) ;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<SynonymWeight> shared_from_this()
    {
      return std::static_pointer_cast<SynonymWeight>(
          Weight::shared_from_this());
    }
  };

public:
  class SynonymScorer : public DisjunctionScorer
  {
    GET_CLASS_NAME(SynonymScorer)
  private:
    const std::shared_ptr<Similarity::SimScorer> similarity;

  public:
    SynonymScorer(std::shared_ptr<Similarity::SimScorer> similarity,
                  std::shared_ptr<Weight> weight,
                  std::deque<std::shared_ptr<Scorer>> &subScorers);

  protected:
    float
    score(std::shared_ptr<DisiWrapper> topList)  override;

    /** combines TF of all subs. */
  public:
    int tf(std::shared_ptr<DisiWrapper> topList) ;

  protected:
    std::shared_ptr<SynonymScorer> shared_from_this()
    {
      return std::static_pointer_cast<SynonymScorer>(
          DisjunctionScorer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SynonymQuery> shared_from_this()
  {
    return std::static_pointer_cast<SynonymQuery>(Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
