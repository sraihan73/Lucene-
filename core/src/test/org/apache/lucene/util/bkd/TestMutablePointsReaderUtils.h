#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <any>
#include <memory>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/util/bkd/Point.h"

#include  "core/src/java/org/apache/lucene/util/BytesRef.h"

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
namespace org::apache::lucene::util::bkd
{

using MutablePointValues = org::apache::lucene::codecs::MutablePointValues;
using BytesRef = org::apache::lucene::util::BytesRef;
using LuceneTestCase = org::apache::lucene::util::LuceneTestCase;

class TestMutablePointsReaderUtils : public LuceneTestCase
{
  GET_CLASS_NAME(TestMutablePointsReaderUtils)

public:
  virtual void testSort();

private:
  void doTestSort();

private:
  class ComparatorAnonymousInnerClass
      : public std::enable_shared_from_this<ComparatorAnonymousInnerClass>,
        public Comparator<std::shared_ptr<Point>>
  {
    GET_CLASS_NAME(ComparatorAnonymousInnerClass)
  private:
    std::shared_ptr<TestMutablePointsReaderUtils> outerInstance;

  public:
    ComparatorAnonymousInnerClass(
        std::shared_ptr<TestMutablePointsReaderUtils> outerInstance);

    int compare(std::shared_ptr<Point> o1, std::shared_ptr<Point> o2);
  };

public:
  virtual void testSortByDim();

private:
  void doTestSortByDim();

public:
  virtual void testPartition();

private:
  void doTestPartition();

  static std::deque<std::shared_ptr<Point>>
  createRandomPoints(int numDims, int bytesPerDim, int maxDoc);

private:
  class Point : public std::enable_shared_from_this<Point>
  {
    GET_CLASS_NAME(Point)
  public:
    const std::shared_ptr<BytesRef> packedValue;
    const int doc;

    Point(std::deque<char> &packedValue, int doc);

    bool equals(std::any obj) override;

    virtual int hashCode();

    virtual std::wstring toString();
  };

private:
  class DummyPointsReader : public MutablePointValues
  {
    GET_CLASS_NAME(DummyPointsReader)

  private:
    std::deque<std::shared_ptr<Point>> const points;

  public:
    DummyPointsReader(std::deque<std::shared_ptr<Point>> &points);

    void getValue(int i, std::shared_ptr<BytesRef> packedValue) override;

    char getByteAt(int i, int k) override;

    int getDocID(int i) override;

    void swap(int i, int j) override;

    void intersect(std::shared_ptr<IntersectVisitor> visitor) throw(
        IOException) override;

    int64_t
    estimatePointCount(std::shared_ptr<IntersectVisitor> visitor) override;

    std::deque<char> getMinPackedValue()  override;

    std::deque<char> getMaxPackedValue()  override;

    int getNumDimensions()  override;

    int getBytesPerDimension()  override;

    int64_t size() override;

    int getDocCount() override;

  protected:
    std::shared_ptr<DummyPointsReader> shared_from_this()
    {
      return std::static_pointer_cast<DummyPointsReader>(
          org.apache.lucene.codecs.MutablePointValues::shared_from_this());
    }
  };

protected:
  std::shared_ptr<TestMutablePointsReaderUtils> shared_from_this()
  {
    return std::static_pointer_cast<TestMutablePointsReaderUtils>(
        org.apache.lucene.util.LuceneTestCase::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/util/bkd/
