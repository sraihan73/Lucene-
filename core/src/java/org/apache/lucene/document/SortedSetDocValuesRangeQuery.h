#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

#include  "core/src/java/org/apache/lucene/index/IndexReader.h"
#include  "core/src/java/org/apache/lucene/search/Query.h"
#include  "core/src/java/org/apache/lucene/index/LeafReader.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"

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
namespace org::apache::lucene::document
{

using IndexReader = org::apache::lucene::index::IndexReader;
using LeafReader = org::apache::lucene::index::LeafReader;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using IndexSearcher = org::apache::lucene::search::IndexSearcher;
using Query = org::apache::lucene::search::Query;
using Weight = org::apache::lucene::search::Weight;
using BytesRef = org::apache::lucene::util::BytesRef;

class SortedSetDocValuesRangeQuery : public Query
{
  GET_CLASS_NAME(SortedSetDocValuesRangeQuery)

private:
  const std::wstring field;
  const std::shared_ptr<BytesRef> lowerValue;
  const std::shared_ptr<BytesRef> upperValue;
  const bool lowerInclusive;
  const bool upperInclusive;

public:
  SortedSetDocValuesRangeQuery(const std::wstring &field,
                               std::shared_ptr<BytesRef> lowerValue,
                               std::shared_ptr<BytesRef> upperValue,
                               bool lowerInclusive, bool upperInclusive);

  bool equals(std::any obj) override;

  virtual int hashCode();

  std::wstring toString(const std::wstring &field) override;

  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader)  override;

  virtual std::shared_ptr<SortedSetDocValues>
  getValues(std::shared_ptr<LeafReader> reader, const std::wstring &field) = 0;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<SortedSetDocValuesRangeQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<SortedSetDocValuesRangeQuery> outerInstance,
        float boost);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
    {
      GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      int64_t minOrd = 0;
      int64_t maxOrd = 0;
      std::shared_ptr<SortedDocValues> singleton;

    public:
      TwoPhaseIteratorAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          int64_t minOrd, int64_t maxOrd,
          std::shared_ptr<SortedDocValues> singleton);

      bool matches()  override;

      float matchCost() override;

    protected:
      std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass>(
            org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
      }
    };

  private:
    class TwoPhaseIteratorAnonymousInnerClass2 : public TwoPhaseIterator
    {
      GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass2)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<SortedSetDocValues> values;
      int64_t minOrd = 0;
      int64_t maxOrd = 0;

    public:
      TwoPhaseIteratorAnonymousInnerClass2(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<SortedSetDocValues> values, int64_t minOrd,
          int64_t maxOrd);

      bool matches()  override;

      float matchCost() override;

    protected:
      std::shared_ptr<TwoPhaseIteratorAnonymousInnerClass2> shared_from_this()
      {
        return std::static_pointer_cast<TwoPhaseIteratorAnonymousInnerClass2>(
            org.apache.lucene.search.TwoPhaseIterator::shared_from_this());
      }
    };

  public:
    bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

  protected:
    std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
          org.apache.lucene.search.ConstantScoreWeight::shared_from_this());
    }
  };

protected:
  std::shared_ptr<SortedSetDocValuesRangeQuery> shared_from_this()
  {
    return std::static_pointer_cast<SortedSetDocValuesRangeQuery>(
        org.apache.lucene.search.Query::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/document/
