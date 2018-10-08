#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class PrefixCodedTerms;
}

namespace org::apache::lucene::util
{
class BytesRef;
}
namespace org::apache::lucene::index
{
class IndexReader;
}
namespace org::apache::lucene::search
{
class Query;
}
namespace org::apache::lucene::util
{
class Accountable;
}
namespace org::apache::lucene::index
{
class TermsEnum;
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
class Term;
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
class Scorer;
}
namespace org::apache::lucene::search
{
class BulkScorer;
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
using PrefixCodedTerms = org::apache::lucene::index::PrefixCodedTerms;
using TermState = org::apache::lucene::index::TermState;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using Accountable = org::apache::lucene::util::Accountable;
using BytesRef = org::apache::lucene::util::BytesRef;
using RamUsageEstimator = org::apache::lucene::util::RamUsageEstimator;

/**
 * Specialization for a disjunction over many terms that behaves like a
 * {@link ConstantScoreQuery} over a {@link BooleanQuery} containing only
 * {@link org.apache.lucene.search.BooleanClause.Occur#SHOULD} clauses.
 * <p>For instance in the following example, both @{code q1} and {@code q2}
 * would yield the same scores:
 * <pre class="prettyprint">
 * Query q1 = new TermInSetQuery(new Term("field", "foo"), new Term("field",
 * "bar"));
 *
 * BooleanQuery bq = new BooleanQuery();
 * bq.add(new TermQuery(new Term("field", "foo")), Occur.SHOULD);
 * bq.add(new TermQuery(new Term("field", "bar")), Occur.SHOULD);
 * Query q2 = new ConstantScoreQuery(bq);
 * </pre>
 * <p>When there are few terms, this query executes like a regular disjunction.
 * However, when there are many terms, instead of merging iterators on the fly,
 * it will populate a bit set with matching docs and return a {@link Scorer}
 * over this bit set.
 * <p>NOTE: This query produces scores that are equal to its boost
 */
class TermInSetQuery : public Query, public Accountable
{
  GET_CLASS_NAME(TermInSetQuery)

private:
  static const int64_t BASE_RAM_BYTES_USED =
      RamUsageEstimator::shallowSizeOfInstance(TermInSetQuery::typeid);
  // Same threshold as MultiTermQueryConstantScoreWrapper
public:
  static constexpr int BOOLEAN_REWRITE_TERM_COUNT_THRESHOLD = 16;

private:
  const std::wstring field;
  const std::shared_ptr<PrefixCodedTerms> termData;
  const int termDataHashCode; // cached hashcode of termData

  /**
   * Creates a new {@link TermInSetQuery} from the given collection of terms.
   */
public:
  TermInSetQuery(const std::wstring &field,
                 std::shared_ptr<std::deque<std::shared_ptr<BytesRef>>> terms);

  /**
   * Creates a new {@link TermInSetQuery} from the given array of terms.
   */
  TermInSetQuery(const std::wstring &field, std::deque<BytesRef> &terms);

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<TermInSetQuery> other);

public:
  virtual int hashCode();

  /** Returns the terms wrapped in a PrefixCodedTerms. */
  virtual std::shared_ptr<PrefixCodedTerms> getTermData();

  std::wstring toString(const std::wstring &defaultField) override;

  int64_t ramBytesUsed() override;

  std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
  getChildResources() override;

private:
  class TermAndState : public std::enable_shared_from_this<TermAndState>
  {
    GET_CLASS_NAME(TermAndState)
  public:
    const std::wstring field;
    const std::shared_ptr<TermsEnum> termsEnum;
    const std::shared_ptr<BytesRef> term;
    const std::shared_ptr<TermState> state;
    const int docFreq;
    const int64_t totalTermFreq;

    TermAndState(const std::wstring &field,
                 std::shared_ptr<TermsEnum> termsEnum) ;
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

public:
  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<TermInSetQuery> outerInstance;

    std::shared_ptr<org::apache::lucene::search::IndexSearcher> searcher;
    bool needsScores = false;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<TermInSetQuery> outerInstance, float boost,
        std::shared_ptr<org::apache::lucene::search::IndexSearcher> searcher,
        bool needsScores);

    void
    extractTerms(std::shared_ptr<Set<std::shared_ptr<Term>>> terms) override;

    std::shared_ptr<Matches> matches(std::shared_ptr<LeafReaderContext> context,
                                     int doc)  override;

    /**
     * On the given leaf context, try to either rewrite to a disjunction if
     * there are few matching terms, or build a bitset containing matching docs.
     */
  private:
    std::shared_ptr<WeightOrDocIdSet>
    rewrite(std::shared_ptr<LeafReaderContext> context) ;

    std::shared_ptr<Scorer>
    scorer(std::shared_ptr<DocIdSet> set) ;

  public:
    std::shared_ptr<BulkScorer> bulkScorer(
        std::shared_ptr<LeafReaderContext> context)  override;

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
  std::shared_ptr<TermInSetQuery> shared_from_this()
  {
    return std::static_pointer_cast<TermInSetQuery>(Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
