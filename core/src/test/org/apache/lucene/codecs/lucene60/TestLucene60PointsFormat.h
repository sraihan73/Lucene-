#pragma once
#include "stringhelper.h"
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::codecs
{
class Codec;
}

namespace org::apache::lucene::codecs
{
class PointsFormat;
}
namespace org::apache::lucene::codecs
{
class PointsWriter;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::codecs
{
class PointsReader;
}
namespace org::apache::lucene::index
{
class SegmentReadState;
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
namespace org::apache::lucene::codecs::lucene60
{

using Codec = org::apache::lucene::codecs::Codec;
using BasePointsFormatTestCase =
    org::apache::lucene::index::BasePointsFormatTestCase;

/**
 * Tests Lucene60PointsFormat
 */
class TestLucene60PointsFormat : public BasePointsFormatTestCase
{
  GET_CLASS_NAME(TestLucene60PointsFormat)
private:
  const std::shared_ptr<Codec> codec;
  const int maxPointsInLeafNode;

public:
  TestLucene60PointsFormat();

private:
  class FilterCodecAnonymousInnerClass : public FilterCodec
  {
    GET_CLASS_NAME(FilterCodecAnonymousInnerClass)
  private:
    std::shared_ptr<TestLucene60PointsFormat> outerInstance;

    double maxMBSortInHeap = 0;

  public:
    FilterCodecAnonymousInnerClass(
        std::shared_ptr<TestLucene60PointsFormat> outerInstance,
        const std::wstring &getName, std::shared_ptr<Codec> defaultCodec,
        double maxMBSortInHeap);

    std::shared_ptr<PointsFormat> pointsFormat() override;

  private:
    class PointsFormatAnonymousInnerClass : public PointsFormat
    {
      GET_CLASS_NAME(PointsFormatAnonymousInnerClass)
    private:
      std::shared_ptr<FilterCodecAnonymousInnerClass> outerInstance;

    public:
      PointsFormatAnonymousInnerClass(
          std::shared_ptr<FilterCodecAnonymousInnerClass> outerInstance);

      std::shared_ptr<PointsWriter>
      fieldsWriter(std::shared_ptr<SegmentWriteState> writeState) throw(
          IOException) override;

      std::shared_ptr<PointsReader>
      fieldsReader(std::shared_ptr<SegmentReadState> readState) throw(
          IOException) override;

    protected:
      std::shared_ptr<PointsFormatAnonymousInnerClass> shared_from_this()
      {
        return std::static_pointer_cast<PointsFormatAnonymousInnerClass>(
            org.apache.lucene.codecs.PointsFormat::shared_from_this());
      }
    };

  protected:
    std::shared_ptr<FilterCodecAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<FilterCodecAnonymousInnerClass>(
          org.apache.lucene.codecs.FilterCodec::shared_from_this());
    }
  };

protected:
  std::shared_ptr<Codec> getCodec() override;

public:
  void testMergeStability()  override;

  virtual void testEstimatePointCount() ;

private:
  class IntersectVisitorAnonymousInnerClass
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
  private:
    std::shared_ptr<TestLucene60PointsFormat> outerInstance;

  public:
    IntersectVisitorAnonymousInnerClass(
        std::shared_ptr<TestLucene60PointsFormat> outerInstance);

    void visit(int docID,
               std::deque<char> &packedValue)  override;
    void visit(int docID)  override;
    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue);
  };

private:
  class IntersectVisitorAnonymousInnerClass2
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass2>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass2)
  private:
    std::shared_ptr<TestLucene60PointsFormat> outerInstance;

  public:
    IntersectVisitorAnonymousInnerClass2(
        std::shared_ptr<TestLucene60PointsFormat> outerInstance);

    void visit(int docID,
               std::deque<char> &packedValue)  override;
    void visit(int docID)  override;
    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue);
  };

private:
  class IntersectVisitorAnonymousInnerClass3
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass3>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass3)
  private:
    std::shared_ptr<TestLucene60PointsFormat> outerInstance;

    std::deque<char> uniquePointValue;

  public:
    IntersectVisitorAnonymousInnerClass3(
        std::shared_ptr<TestLucene60PointsFormat> outerInstance,
        std::deque<char> &uniquePointValue);

    void visit(int docID,
               std::deque<char> &packedValue)  override;
    void visit(int docID)  override;
    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue);
  };

  // The tree is always balanced in the N dims case, and leaves are
  // not all full so things are a bit different
public:
  virtual void testEstimatePointCount2Dims() ;

private:
  class IntersectVisitorAnonymousInnerClass4
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass4>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass4)
  private:
    std::shared_ptr<TestLucene60PointsFormat> outerInstance;

  public:
    IntersectVisitorAnonymousInnerClass4(
        std::shared_ptr<TestLucene60PointsFormat> outerInstance);

    void visit(int docID,
               std::deque<char> &packedValue)  override;
    void visit(int docID)  override;
    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue);
  };

private:
  class IntersectVisitorAnonymousInnerClass5
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass5>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass5)
  private:
    std::shared_ptr<TestLucene60PointsFormat> outerInstance;

  public:
    IntersectVisitorAnonymousInnerClass5(
        std::shared_ptr<TestLucene60PointsFormat> outerInstance);

    void visit(int docID,
               std::deque<char> &packedValue)  override;
    void visit(int docID)  override;
    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue);
  };

private:
  class IntersectVisitorAnonymousInnerClass6
      : public std::enable_shared_from_this<
            IntersectVisitorAnonymousInnerClass6>,
        public PointValues::IntersectVisitor
  {
    GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass6)
  private:
    std::shared_ptr<TestLucene60PointsFormat> outerInstance;

    std::deque<std::deque<char>> uniquePointValue;

  public:
    IntersectVisitorAnonymousInnerClass6(
        std::shared_ptr<TestLucene60PointsFormat> outerInstance,
        std::deque<std::deque<char>> &uniquePointValue);

    void visit(int docID,
               std::deque<char> &packedValue)  override;
    void visit(int docID)  override;
    PointValues::Relation compare(std::deque<char> &minPackedValue,
                                  std::deque<char> &maxPackedValue);
  };

protected:
  std::shared_ptr<TestLucene60PointsFormat> shared_from_this()
  {
    return std::static_pointer_cast<TestLucene60PointsFormat>(
        org.apache.lucene.index.BasePointsFormatTestCase::shared_from_this());
  }
};

} // namespace org::apache::lucene::codecs::lucene60
