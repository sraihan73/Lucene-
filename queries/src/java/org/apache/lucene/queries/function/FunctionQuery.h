#pragma once
#include "stringhelper.h"
#include <any>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::queries::function
{
class ValueSource;
}

namespace org::apache::lucene::search
{
class IndexSearcher;
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
class DocIdSetIterator;
}
namespace org::apache::lucene::queries::function
{
class FunctionValues;
}
namespace org::apache::lucene::search
{
class Weight;
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
namespace org::apache::lucene::queries::function
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReaderContext = org::apache::lucene::index::LeafReaderContext;
using Term = org::apache::lucene::index::Term;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using Explanation = org::apache::lucene::search::Explanation;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;

/**
 * Returns a score for each document based on a ValueSource,
 * often some function of the value of a field.
 *
 * @see ValueSourceScorer
 * @lucene.experimental
 */
class FunctionQuery : public Query
{
  GET_CLASS_NAME(FunctionQuery)
public:
  const std::shared_ptr<ValueSource> func;

  /**
   * @param func defines the function to be used for scoring
   */
  FunctionQuery(std::shared_ptr<ValueSource> func);

  /** @return The associated ValueSource */
  virtual std::shared_ptr<ValueSource> getValueSource();

protected:
  class FunctionWeight : public Weight
  {
    GET_CLASS_NAME(FunctionWeight)
  private:
    std::shared_ptr<FunctionQuery> outerInstance;

  protected:
    const std::shared_ptr<IndexSearcher> searcher;
    const float boost;
    const std::unordered_map context;

  public:
    FunctionWeight(std::shared_ptr<FunctionQuery> outerInstance,
                   std::shared_ptr<IndexSearcher> searcher,
                   float boost) ;

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;
    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

  protected:
    std::shared_ptr<FunctionWeight> shared_from_this()
    {
      return std::static_pointer_cast<FunctionWeight>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

protected:
  class AllScorer : public Scorer
  {
    GET_CLASS_NAME(AllScorer)
  private:
    std::shared_ptr<FunctionQuery> outerInstance;

  public:
    const std::shared_ptr<IndexReader> reader;
    const std::shared_ptr<FunctionWeight> weight;
    const int maxDoc;
    const float boost;
    // C++ NOTE: Fields cannot have the same name as methods:
    const std::shared_ptr<DocIdSetIterator> iterator_;
    const std::shared_ptr<FunctionValues> vals;

    AllScorer(std::shared_ptr<FunctionQuery> outerInstance,
              std::shared_ptr<LeafReaderContext> context,
              std::shared_ptr<FunctionWeight> w,
              float boost) ;

    std::shared_ptr<DocIdSetIterator> iterator() override;

    int docID() override;

    float score()  override;

    virtual std::shared_ptr<Explanation> explain(int doc) ;

  protected:
    std::shared_ptr<AllScorer> shared_from_this()
    {
      return std::static_pointer_cast<AllScorer>(
          org.apache.lucene.search.Scorer::shared_from_this());
    }
  };

public:
  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

  /** Prints a user-readable version of this query. */
  std::wstring toString(const std::wstring &field) override;

  /** Returns true if <code>o</code> is equal to this. */
  bool equals(std::any other) override;

  virtual int hashCode();

protected:
  std::shared_ptr<FunctionQuery> shared_from_this()
  {
    return std::static_pointer_cast<FunctionQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::queries::function
