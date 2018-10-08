#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <type_traits>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util
{
class BytesRef;
}

namespace org::apache::lucene::index
{
class TermState;
}
namespace org::apache::lucene::search
{
class Weight;
}
namespace org::apache::lucene::search
{
class DocIdSet;
}
namespace org::apache::lucene::search
{
class IndexSearcher;
}
namespace org::apache::lucene::index
{
class LeafReaderContext;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::search
{
class BulkScorer;
}
namespace org::apache::lucene::search
{
class Matches;
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

using TermState = org::apache::lucene::index::TermState;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * This class also provides the functionality behind
 * {@link MultiTermQuery#CONSTANT_SCORE_REWRITE}.
GET_CLASS_NAME(also)
 * It tries to rewrite per-segment as a bool query
 * that returns a constant score and otherwise fills a
 * bit set with matches and builds a Scorer on top of
 * this bit set.
 */
template <typename Q>
class MultiTermQueryConstantScoreWrapper final : public Query
{
  GET_CLASS_NAME(MultiTermQueryConstantScoreWrapper)
  static_assert(std::is_base_of<MultiTermQuery, Q>::value,
                L"Q must inherit from MultiTermQuery");

  // mtq that matches 16 terms or less will be executed as a regular disjunction
private:
  static constexpr int BOOLEAN_REWRITE_TERM_COUNT_THRESHOLD = 16;

private:
  class TermAndState : public std::enable_shared_from_this<TermAndState>
  {
    GET_CLASS_NAME(TermAndState)
  public:
    const std::shared_ptr<BytesRef> term;
    const std::shared_ptr<TermState> state;
    const int docFreq;
    const int64_t totalTermFreq;

    TermAndState(std::shared_ptr<BytesRef> term,
                 std::shared_ptr<TermState> state, int docFreq,
                 int64_t totalTermFreq);
  };

private:
  class WeightOrDocIdSet : public std::enable_shared_from_this<WeightOrDocIdSet>
  {
    GET_CLASS_NAME(WeightOrDocIdSet)
  public:
    const std::shared_ptr<Weight> weight;
    const std::shared_ptr<DocIdSet> set;

    WeightOrDocIdSet(std::shared_ptr<Weight> weight);

    WeightOrDocIdSet(std::shared_ptr<DocIdSet> bitset);
  };

protected:
  const Q query;

  /**
   * Wrap a {@link MultiTermQuery} as a Filter.
   */
  MultiTermQueryConstantScoreWrapper(Q query) : query(query) {}

public:
  std::wstring toString(const std::wstring &field) override
  {
    // query.toString should be ok for the filter, too, if the query boost
    // is 1.0f
    // C++ TODO: There is no native C++ equivalent to 'toString':
    return query->toString(field);
  }

  bool equals(std::any const other) override final
  {
    // C++ TODO: Java wildcard generics are not converted to C++:
    // ORIGINAL LINE: return sameClassAs(other) &&
    // query.equals(((MultiTermQueryConstantScoreWrapper<?>) other).query);
        return sameClassAs(other) && query->equals((std::any_cast<MultiTermQueryConstantScoreWrapper<?>>(other)).query);
  }

  int hashCode() override final { return 31 * classHash() + query->hashCode(); }

  /** Returns the encapsulated query */
  Q getQuery() { return query; }

  /** Returns the field name for this query */
  std::wstring getField() { return query->getField(); }

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override
  {
    return std::make_shared<ConstantScoreWeightAnonymousInnerClass>(
        shared_from_this(), boost, searcher, needsScores);
  }

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<MultiTermQueryConstantScoreWrapper<std::shared_ptr<Q>>>
        outerInstance;

    std::shared_ptr<org::apache::lucene::search::IndexSearcher> searcher;
    bool needsScores = false;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<MultiTermQueryConstantScoreWrapper<std::shared_ptr<Q>>>
            outerInstance,
        float boost,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> searcher,
        bool needsScores);

    /** Try to collect terms from the given terms enum and return true iff all
     *  terms could be collected. If {@code false} is returned, the enum is
     *  left positioned on the next term. */
  private:
    bool collectTerms(
        std::shared_ptr<LeafReaderContext> context,
        std::shared_ptr<TermsEnum> termsEnum,
        std::deque<std::shared_ptr<TermAndState>> &terms) ;

    /**
     * On the given leaf context, try to either rewrite to a disjunction if
     * there are few terms, or build a bitset containing matching docs.
     */
    std::shared_ptr<WeightOrDocIdSet>
    rewrite(std::shared_ptr<LeafReaderContext> context) ;

    std::shared_ptr<Scorer>
    scorer(std::shared_ptr<DocIdSet> set) ;

  public:
    std::shared_ptr<BulkScorer> bulkScorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          ConstantScoreWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<MultiTermQueryConstantScoreWrapper> shared_from_this()
  {
    return std::static_pointer_cast<MultiTermQueryConstantScoreWrapper>(
        Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
