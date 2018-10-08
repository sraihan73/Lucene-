#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRefHash.h"

#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/Term.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Explanation.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/search/DocIdSetIterator.h"
#include  "core/src/java/org/apache/lucene/index/TermsEnum.h"
#include  "core/src/java/org/apache/lucene/util/FixedBitSet.h"

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
namespace org::apache::lucene::search::join
{

using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Scorer = org::apache::lucene::search::Scorer;
using Weight = org::apache::lucene::search::Weight;
using BytesRefHash = org::apache::lucene::util::BytesRefHash;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

class TermsIncludingScoreQuery : public Query
{
  GET_CLASS_NAME(TermsIncludingScoreQuery)

private:
  const ScoreMode scoreMode;
  const std::wstring toField;
  const bool multipleValuesPerDocument;
  const std::shared_ptr<BytesRefHash> terms;
  std::deque<float> const scores;
  std::deque<int> const ords;

  // These fields are used for equals() and hashcode() only
  const std::shared_ptr<Query> fromQuery;
  const std::wstring fromField;
  // id of the context rather than the context itself in order not to hold
  // references to index readers
  const std::any topReaderContextId;

public:
  TermsIncludingScoreQuery(ScoreMode scoreMode, const std::wstring &toField,
                           bool multipleValuesPerDocument,
                           std::shared_ptr<BytesRefHash> terms,
                           std::deque<float> &scores,
                           const std::wstring &fromField,
                           std::shared_ptr<Query> fromQuery,
                           std::any indexReaderContextId);

  std::wstring toString(const std::wstring &string) override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<TermsIncludingScoreQuery> other);

public:
  virtual int hashCode();

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class WeightAnonymousInnerClass : public Weight
  {
    GET_CLASS_NAME(WeightAnonymousInnerClass)
  private:
    std::shared_ptr<TermsIncludingScoreQuery> outerInstance;

  public:
    // C++ TODO: You cannot use 'shared_from_this' in a constructor:
    WeightAnonymousInnerClass(
        std::shared_ptr<TermsIncludingScoreQuery> outerInstance,
        std::shared_ptr<
            org::apache::lucene::search::join::TermsIncludingScoreQuery>
            shared_from_this());

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;
    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<WeightAnonymousInnerClass>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

public:
  class SVInOrderScorer : public Scorer
  {
    GET_CLASS_NAME(SVInOrderScorer)
  private:
    std::shared_ptr<TermsIncludingScoreQuery> outerInstance;

  public:
    const std::shared_ptr<DocIdSetIterator> matchingDocsIterator;
    std::deque<float> const scores;
    const int64_t cost;

    SVInOrderScorer(std::shared_ptr<TermsIncludingScoreQuery> outerInstance,
                    std::shared_ptr<Weight> weight,
                    std::shared_ptr<TermsEnum> termsEnum, int maxDoc,
                    int64_t cost) ;

  protected:
    virtual void
    fillDocsAndScores(std::shared_ptr<FixedBitSet> matchingDocs,
                      std::shared_ptr<TermsEnum> termsEnum) ;

  public:
    float score()  override;

    int docID() override;

    std::shared_ptr<DocIdSetIterator> iterator() override;

  protected:
    std::shared_ptr<SVInOrderScorer> shared_from_this()
    {
      return std::static_pointer_cast<SVInOrderScorer>(
          org.apache.lucene.search.Scorer::shared_from_this());
    }
  };

  // This scorer deals with the fact that a document can have more than one
  // score from multiple related documents.
public:
  class MVInOrderScorer : public SVInOrderScorer
  {
    GET_CLASS_NAME(MVInOrderScorer)
  private:
    std::shared_ptr<TermsIncludingScoreQuery> outerInstance;

  public:
    MVInOrderScorer(std::shared_ptr<TermsIncludingScoreQuery> outerInstance,
                    std::shared_ptr<Weight> weight,
                    std::shared_ptr<TermsEnum> termsEnum, int maxDoc,
                    int64_t cost) ;

  protected:
    void fillDocsAndScores(
        std::shared_ptr<FixedBitSet> matchingDocs,
        std::shared_ptr<TermsEnum> termsEnum)  override;

  protected:
    std::shared_ptr<MVInOrderScorer> shared_from_this()
    {
      return std::static_pointer_cast<MVInOrderScorer>(
          SVInOrderScorer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TermsIncludingScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<TermsIncludingScoreQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/join/
