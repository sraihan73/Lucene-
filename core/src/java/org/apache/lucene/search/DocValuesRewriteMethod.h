#pragma once
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::index
{
class IndexReader;
}

namespace org::apache::lucene::search
{
class MultiTermQuery;
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
class LeafReaderContext;
}
namespace org::apache::lucene::search
{
class Matches;
}
namespace org::apache::lucene::index
{
class SortedSetDocValues;
}
namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::search
{
class Scorer;
}
namespace org::apache::lucene::util
{
class LongBitSet;
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

/**
 * Rewrites MultiTermQueries into a filter, using DocValues for term
 * enumeration. <p> This can be used to perform these queries against an
 * unindexed docvalues field.
 * @lucene.experimental
 */
class DocValuesRewriteMethod final : public MultiTermQuery::RewriteMethod
{
  GET_CLASS_NAME(DocValuesRewriteMethod)

public:
  std::shared_ptr<Query>
  rewrite(std::shared_ptr<IndexReader> reader,
          std::shared_ptr<MultiTermQuery> query) override;

public:
  class MultiTermQueryDocValuesWrapper : public Query
  {
    GET_CLASS_NAME(MultiTermQueryDocValuesWrapper)

  protected:
    const std::shared_ptr<MultiTermQuery> query;

    /**
     * Wrap a {@link MultiTermQuery} as a Filter.
     */
    MultiTermQueryDocValuesWrapper(std::shared_ptr<MultiTermQuery> query);

  public:
    std::wstring toString(const std::wstring &field) override;

    bool equals(std::any const other) override final;

    int hashCode() override final;

    /** Returns the field name for this query */
    std::wstring getField();

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
    {
      GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
    private:
      std::shared_ptr<MultiTermQueryDocValuesWrapper> outerInstance;

    public:
      ConstantScoreWeightAnonymousInnerClass(
          std::shared_ptr<MultiTermQueryDocValuesWrapper> outerInstance,
          float boost);

      std::shared_ptr<Matches>
      matches(std::shared_ptr<LeafReaderContext> context,
              int doc)  override;

    private:
      std::shared_ptr<TermsEnum>
      getTermsEnum(std::shared_ptr<SortedSetDocValues> fcsi) ;

    private:
      class TermsAnonymousInnerClass : public Terms
      {
        GET_CLASS_NAME(TermsAnonymousInnerClass)
      private:
        std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

        std::shared_ptr<SortedSetDocValues> fcsi;

      public:
        TermsAnonymousInnerClass(
            std::shared_ptr<ConstantScoreWeightAnonymousInnerClass>
                outerInstance,
            std::shared_ptr<SortedSetDocValues> fcsi);

        std::shared_ptr<TermsEnum> iterator()  override;

        int64_t getSumTotalTermFreq() override;

        int64_t getSumDocFreq() override;

        int getDocCount() override;

        int64_t size() override;

        bool hasFreqs() override;

        bool hasOffsets() override;

        bool hasPositions() override;

        bool hasPayloads() override;

      protected:
        std::shared_ptr<TermsAnonymousInnerClass> shared_from_this()
        {
          return std::static_pointer_cast<TermsAnonymousInnerClass>(
              org.apache.lucene.index.Terms::shared_from_this());
        }
      };

    public:
      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

    private:
      class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
      {
        GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
      private:
        std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

        std::shared_ptr<SortedSetDocValues> fcsi;
        std::shared_ptr<LongBitSet> termSet;
        int64_t ord = 0;

      public:
        TwoPhaseIteratorAnonymousInnerClass(
            std::shared_ptr<ConstantScoreWeightAnonymousInnerClass>
                outerInstance,
            std::shared_ptr<SortedSetDocValues> fcsi,
            std::shared_ptr<LongBitSet> termSet, int64_t ord);

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
    std::shared_ptr<MultiTermQueryDocValuesWrapper> shared_from_this()
    {
      return std::static_pointer_cast<MultiTermQueryDocValuesWrapper>(
          Query::shared_from_this());
    }
  };

public:
  bool equals(std::any other) override;

  virtual int hashCode();

protected:
  std::shared_ptr<DocValuesRewriteMethod> shared_from_this()
  {
    return std::static_pointer_cast<DocValuesRewriteMethod>(
        MultiTermQuery.RewriteMethod::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
