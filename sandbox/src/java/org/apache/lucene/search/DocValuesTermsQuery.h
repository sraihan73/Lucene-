#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/PrefixCodedTerms.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/util/LongBitSet.h"

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

using PrefixCodedTerms = org::apache::lucene::index::PrefixCodedTerms;
using BytesRef = org::apache::lucene::util::BytesRef;

/**
 * A {@link Query} that only accepts documents whose
 * term value in the specified field is contained in the
 * provided set of allowed terms.
 *
 * <p>
 * This is the same functionality as TermsQuery (from
 * queries/), but because of drastically different
 * implementations, they also have different performance
 * characteristics, as described below.
 *
 * <p>
 * <b>NOTE</b>: be very careful using this query: it is
 * typically much slower than using {@code TermsQuery},
 * but in certain specialized cases may be faster.
 *
 * <p>
 * With each search, this query translates the specified
 * set of Terms into a private {@link LongBitSet} keyed by
 * term number per unique {@link IndexReader} (normally one
 * reader per segment).  Then, during matching, the term
 * number for each docID is retrieved from the cache and
 * then checked for inclusion using the {@link LongBitSet}.
 * Since all testing is done using RAM resident data
 * structures, performance should be very fast, most likely
 * fast enough to not require further caching of the
 * DocIdSet for each possible combination of terms.
 * However, because docIDs are simply scanned linearly, an
 * index with a great many small documents may find this
 * linear scan too costly.
 *
 * <p>
 * In contrast, TermsQuery builds up an {@link FixedBitSet},
 * keyed by docID, every time it's created, by enumerating
 * through all matching docs using {@link org.apache.lucene.index.PostingsEnum}
 * to seek and scan through each term's docID deque.  While there is no linear
 * scan of all docIDs, besides the allocation of the underlying array in the
 * {@link FixedBitSet}, this approach requires a number of "disk seeks" in
 * proportion to the number of terms, which can be exceptionally costly when
 * there are cache misses in the OS's IO cache.
 *
 * <p>
 * Generally, this filter will be slower on the first
 * invocation for a given field, but subsequent invocations,
 * even if you change the allowed set of Terms, should be
 * faster than TermsQuery, especially as the number of
 * Terms being matched increases.  If you are matching only
 * a very small number of terms, and those terms in turn
 * match a very small number of documents, TermsQuery may
 * perform faster.
 *
 * <p>
 * Which query is best is very application dependent.
 *
 * @lucene.experimental
 */
class DocValuesTermsQuery : public Query
{
  GET_CLASS_NAME(DocValuesTermsQuery)

private:
  const std::wstring field;
  const std::shared_ptr<PrefixCodedTerms> termData;
  const int termDataHashCode; // cached hashcode of termData

public:
  DocValuesTermsQuery(
      const std::wstring &field,
      std::shared_ptr<std::deque<std::shared_ptr<BytesRef>>> terms);

  DocValuesTermsQuery(const std::wstring &field, std::deque<BytesRef> &terms);

  DocValuesTermsQuery(const std::wstring &field,
                      std::deque<std::wstring> &terms);

private:
  class AbstractListAnonymousInnerClass
      : public AbstractList<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(AbstractListAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesTermsQuery> outerInstance;

  public:
    AbstractListAnonymousInnerClass(
        std::shared_ptr<DocValuesTermsQuery> outerInstance);

    std::shared_ptr<BytesRef> get(int index) override;
    int size() override;

  protected:
    std::shared_ptr<AbstractListAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<AbstractListAnonymousInnerClass>(
          java.util.AbstractList<
              org.apache.lucene.util.BytesRef>::shared_from_this());
    }
  };

public:
  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<DocValuesTermsQuery> other);

public:
  virtual int hashCode();

  std::wstring toString(const std::wstring &defaultField) override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesTermsQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<DocValuesTermsQuery> outerInstance, float boost);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
    {
      GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<SortedSetDocValues> values;
      std::shared_ptr<LongBitSet> bits;

    public:
      TwoPhaseIteratorAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<SortedSetDocValues> values,
          std::shared_ptr<LongBitSet> bits);

      bool matches()  override;

      float matchCost() override;

    protected:
      std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
            TwoPhaseIterator::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          ConstantScoreWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<DocValuesTermsQuery> shared_from_this()
  {
    return std::static_pointer_cast<DocValuesTermsQuery>(
        Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
