#pragma once
#include "stringbuilder.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class LongHashSet;
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
class Scorer;
}
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
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

/**
 * Like {@link DocValuesTermsQuery}, but this query only
 * runs on a long {@link NumericDocValuesField} or a
 * {@link SortedNumericDocValuesField}, matching
 * all documents whose value in the specified field is
 * contained in the provided set of long values.
 *
 * <p>
 * <b>NOTE</b>: be very careful using this query: it is
 * typically much slower than using {@code TermsQuery},
 * but in certain specialized cases may be faster.
 *
 * @lucene.experimental
 */
class DocValuesNumbersQuery : public Query
{
  GET_CLASS_NAME(DocValuesNumbersQuery)

private:
  const std::wstring field;
  const std::shared_ptr<LongHashSet> numbers;

public:
  DocValuesNumbersQuery(const std::wstring &field,
                        std::deque<int64_t> &numbers);

  DocValuesNumbersQuery(const std::wstring &field,
                        std::shared_ptr<std::deque<int64_t>> numbers);

  DocValuesNumbersQuery(const std::wstring &field,
                        std::deque<int64_t> &numbers);

  bool equals(std::any other) override;

private:
  bool equalsTo(std::shared_ptr<DocValuesNumbersQuery> other);

public:
  virtual int hashCode();

  virtual std::wstring getField();

  virtual std::shared_ptr<Set<int64_t>> getNumbers();

  std::wstring toString(const std::wstring &defaultField) override;

  std::shared_ptr<Weight> createWeight(std::shared_ptr<IndexSearcher> searcher,
                                       bool needsScores,
                                       float boost)  override;

private:
  class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
  {
    GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
  private:
    std::shared_ptr<DocValuesNumbersQuery> outerInstance;

  public:
    ConstantScoreWeightAnonymousInnerClass(
        std::shared_ptr<DocValuesNumbersQuery> outerInstance, float boost);

    std::shared_ptr<Scorer> scorer(
        std::shared_ptr<LeafReaderContext> context)  override;

  private:
    class TwoPhaseIteratorAnonymousInnerClass : public TwoPhaseIterator
    {
      GET_CLASS_NAME(TwoPhaseIteratorAnonymousInnerClass)
    private:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance;

      std::shared_ptr<SortedNumericDocValues> values;

    public:
      TwoPhaseIteratorAnonymousInnerClass(
          std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> outerInstance,
          std::shared_ptr<SortedNumericDocValues> values);

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
  std::shared_ptr<DocValuesNumbersQuery> shared_from_this()
  {
    return std::static_pointer_cast<DocValuesNumbersQuery>(
        Query::shared_from_this());
  }
};

} // namespace org::apache::lucene::search
