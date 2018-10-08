#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::search
{
class DocIdSet;
}

namespace org::apache::lucene::index
{
class TermsEnum;
}
namespace org::apache::lucene::index
{
class PointValues;
}
namespace org::apache::lucene::index
{
class IntersectVisitor;
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
namespace org::apache::lucene::util
{

using PointValues = org::apache::lucene::index::PointValues;
using Terms = org::apache::lucene::index::Terms;
using TermsEnum = org::apache::lucene::index::TermsEnum;
using DocIdSet = org::apache::lucene::search::DocIdSet;

class TestDocIdSetBuilder : public LuceneTestCase
{
  GET_CLASS_NAME(TestDocIdSetBuilder)

public:
  virtual void testEmpty() ;

private:
  void assertEquals(std::shared_ptr<DocIdSet> d1,
                    std::shared_ptr<DocIdSet> d2) ;

public:
  virtual void testSparse() ;

  virtual void testDense() ;

  virtual void testRandom() ;

  virtual void testMisleadingDISICost() ;

  virtual void testEmptyPoints() ;

  virtual void testLeverageStats() ;

private:
  class DummyTerms : public Terms
  {
    GET_CLASS_NAME(DummyTerms)

  private:
    const int docCount;
    const int64_t numValues;

  public:
    DummyTerms(int docCount, int64_t numValues);

    std::shared_ptr<TermsEnum> iterator()  override;

    int64_t size()  override;

    int64_t getSumTotalTermFreq()  override;

    int64_t getSumDocFreq()  override;

    int getDocCount()  override;

    bool hasFreqs() override;

    bool hasOffsets() override;

    bool hasPositions() override;

    bool hasPayloads() override;

  protected:
    std::shared_ptr<DummyTerms> shared_from_this()
    {
      return std::static_pointer_cast<DummyTerms>(
          org.apache.lucene.index.Terms::shared_from_this());
    }
  };

private:
  class DummyPointValues : public PointValues
  {
    GET_CLASS_NAME(DummyPointValues)

  private:
    const int docCount;
    const int64_t numPoints;

  public:
    DummyPointValues(int docCount, int64_t numPoints);

    void
    intersect(std::shared_ptr<PointValues::IntersectVisitor> visitor) throw(
        IOException) override;

    int64_t estimatePointCount(
        std::shared_ptr<PointValues::IntersectVisitor> visitor) override;

    std::deque<char> getMinPackedValue()  override;

    std::deque<char> getMaxPackedValue()  override;

    int getNumDimensions()  override;

    int getBytesPerDimension()  override;

    int64_t size() override;

    int getDocCount() override;

  protected:
    std::shared_ptr<DummyPointValues> shared_from_this()
    {
      return std::static_pointer_cast<DummyPointValues>(
          org.apache.lucene.index.PointValues::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestDocIdSetBuilder> shared_from_this()
  {
    return std::static_pointer_cast<TestDocIdSetBuilder>(
        LuceneTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::util
