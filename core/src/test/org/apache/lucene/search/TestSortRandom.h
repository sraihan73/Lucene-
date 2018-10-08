#pragma once
#include "stringhelper.h"
#include <algorithm>
#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/search/SortField.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/search/IndexSearcher.h"
#include  "core/src/java/org/apache/lucene/search/Weight.h"
#include  "core/src/java/org/apache/lucene/index/LeafReaderContext.h"
#include  "core/src/java/org/apache/lucene/search/Scorer.h"

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

using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

/** random sorting tests */
class TestSortRandom : public LuceneTestCase
{
  GET_CLASS_NAME(TestSortRandom)

public:
  virtual void testRandomStringSort() ;

  virtual void testRandomStringValSort() ;

private:
  void testRandomStringSort(SortField::Type type) ;

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<BytesRef>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestSortRandom> outerInstance;

    bool sortMissingLast = false;

  public:
    ComparatorAnonymousInnerClass(std::shared_ptr<TestSortRandom> outerInstance,
                                  bool sortMissingLast);

    int compare(std::shared_ptr<BytesRef> a, std::shared_ptr<BytesRef> b);
  };

private:
  class RandomQuery : public Query
  {
    GET_CLASS_NAME(RandomQuery)
  private:
    const int64_t seed;
    float density = 0;
    const std::deque<std::shared_ptr<BytesRef>> docValues;

  public:
    const std::deque<std::shared_ptr<BytesRef>> matchValues =
        Collections::synchronizedList(std::deque<std::shared_ptr<BytesRef>>());

    // density should be 0.0 ... 1.0
    RandomQuery(int64_t seed, float density,
                std::deque<std::shared_ptr<BytesRef>> &docValues);

    std::shared_ptr<Weight>
    createWeight(std::shared_ptr<IndexSearcher> searcher, bool needsScores,
                 float boost)  override;

  private:
    class ConstantScoreWeightAnonymousInnerClass : public ConstantScoreWeight
    {
      GET_CLASS_NAME(ConstantScoreWeightAnonymousInnerClass)
    private:
      std::shared_ptr<RandomQuery> outerInstance;

    public:
      ConstantScoreWeightAnonymousInnerClass(
          std::shared_ptr<RandomQuery> outerInstance, float boost);

      std::shared_ptr<Scorer>
      scorer(std::shared_ptr<LeafReaderContext> context) throw(
          IOException) override;

      bool isCacheable(std::shared_ptr<LeafReaderContext> ctx) override;

    protected:
      std::shared_ptr<ConstantScoreWeightAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<ConstantScoreWeightAnonymousInnerClass>(
            ConstantScoreWeight::shared_from_this());
      }
    };

  public:
    std::wstring toString(const std::wstring &field) override;

    bool equals(std::any other) override;

  private:
    bool equalsTo(std::shared_ptr<RandomQuery> other);

  public:
    virtual int hashCode();

  protected:
    std::shared_ptr<RandomQuery> shared_from_this()
    {
      return std::static_pointer_cast<RandomQuery>(Query::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestSortRandom> shared_from_this()
  {
    return std::static_pointer_cast<TestSortRandom>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/search/
