#pragma once
#include "exceptionhelper.h"
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class Query;
}

namespace org::apache::lucene::index
{
class PrefixCodedTerms;
}
namespace org::apache::lucene::search
{
class PointInSetQuery;
}
namespace org::apache::lucene::search
{
class Stream;
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
class Term;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Explanation;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::util
{
class FixedBitSet;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
}
namespace org::apache::lucene::index
{
class TermIterator;
}
namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class PointValues;
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

namespace org::apache::lucene::search::join
{

using PointValues = org::apache::lucene::index::PointValues;
using IntersectVisitor =
    org::apache::lucene::index::PointValues::IntersectVisitor;
using Relation = org::apache::lucene::index::PointValues::Relation;
using PrefixCodedTerms = org::apache::lucene::index::PrefixCodedTerms;
using TermIterator = org::apache::lucene::index::PrefixCodedTerms::TermIterator;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using PointInSetQuery = org::apache::lucene::search::PointInSetQuery;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;
using BytesRef = org::apache::lucene::util::BytesRef;
using FixedBitSet = org::apache::lucene::util::FixedBitSet;

// A TermsIncludingScoreQuery variant for point values:
class PointInSetIncludingScoreQuery : public Query
{
  GET_CLASS_NAME(PointInSetIncludingScoreQuery)

public:
  // C++ TODO: There is no native C++ equivalent to 'toString':
  // C++ NOTE: Fields cannot have the same name as methods:
  static std::function<std::wstring *(char[], std::type_info *)> toString_;

  const ScoreMode scoreMode;
  const std::shared_ptr<Query> originalQuery;
  const bool multipleValuesPerDocument;
  const std::shared_ptr<PrefixCodedTerms> sortedPackedPoints;
  const int sortedPackedPointsHashCode;
  const std::wstring field;
  const int bytesPerDim;

  const std::deque<float> aggregatedJoinScores;

public:
  class Stream : public PointInSetQuery::Stream
  {
    GET_CLASS_NAME(Stream)

  public:
    float score = 0;

  protected:
    std::shared_ptr<Stream> shared_from_this()
    {
      return std::static_pointer_cast<Stream>(
          org.apache.lucene.search.PointInSetQuery.Stream::shared_from_this());
    }
  };

public:
  PointInSetIncludingScoreQuery(
      ScoreMode scoreMode, std::shared_ptr<Query> originalQuery,
      bool multipleValuesPerDocument, const std::wstring &field,
      int bytesPerDim, std::shared_ptr<PointInSetQuery::Stream> packedPoints);

  std::shared_ptr<Weight>
  createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
               float boost)  override final;

private:
  class WeightAnonymousInnerClass : public Weight
  {
    GET_CLASS_NAME(WeightAnonymousInnerClass)
  private:
    std::shared_ptr<PointInSetIncludingScoreQuery> outerInstance;

  public:
    WeightAnonymousInnerClass(
        std::shared_ptr<PointInSetIncludingScoreQuery> outerInstance);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Explanation>
    explain(std::shared_ptr<LeafReaderContext> context,
            int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class ScorerAnonymousInnerClass : public Scorer
    {
      GET_CLASS_NAME(ScorerAnonymousInnerClass)
    private:
      std::shared_ptr<WeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<FixedBitSet> result;
      std::deque<float> scores;

    public:
      ScorerAnonymousInnerClass(
          std::shared_ptr<WeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<FixedBitSet> result, std::deque<float> &scores);

      std::shared_ptr<DocIdSetIterator> disi;

      float score()  override;

      int docID() override;

      std::shared_ptr<DocIdSetIterator> iterator() override;

    protected:
      std::shared_ptr<ScorerAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ScorerAnonymousInnerClass>(
            org.apache.lucene.search.Scorer::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<WeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<WeightAnonymousInnerClass>(
          org.apache.lucene.search.Weight::shared_from_this());
    }
  };

private:
  class MergePointVisitor
      : public std::enable_shared_from_this<MergePointVisitor>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(MergePointVisitor)
  private:
    std::shared_ptr<PointInSetIncludingScoreQuery> outerInstance;

    const std::shared_ptr<FixedBitSet> result;
    std::deque<float> const scores;

    std::shared_ptr<PrefixCodedTerms::TermIterator> iterator;
    std::shared_ptr<Iterator<float>> scoreIterator;
    std::shared_ptr<BytesRef> nextQueryPoint;

  public:
    float nextScore = 0;

  private:
    const std::shared_ptr<BytesRef> scratch = std::make_shared<BytesRef>();

    MergePointVisitor(
        std::shared_ptr<PointInSetIncludingScoreQuery> outerInstance,
        std::shared_ptr<PrefixCodedTerms> sortedPackedPoints,
        std::shared_ptr<FixedBitSet> result,
        std::deque<float> &scores) ;

  public:
    void visit(int docID) override;

    void visit(int docID, std::deque<char> &packedValue) override;

    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue) override;
  };

public:
  int hashCode() override final;

  bool equals(std::any other) override final;

private:
  bool equalsTo(std::shared_ptr<PointInSetIncludingScoreQuery> other);

public:
  std::wstring toString(const std::wstring &field) override final;

protected:
  virtual std::wstring toString(std::deque<char> &value) = 0;

protected:
  std::shared_ptr<PointInSetIncludingScoreQuery> shared_from_this()
  {
    return std::static_pointer_cast<PointInSetIncludingScoreQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search::join
