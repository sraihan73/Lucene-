#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <deque>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"

#include  "core/src/java/org/apache/lucene/util/ByteBlockPool.h"
#include  "core/src/java/org/apache/lucene/util/Counter.h"
#include  "core/src/java/org/apache/lucene/index/DocumentsWriterPerThread.h"
#include  "core/src/java/org/apache/lucene/util/BytesRef.h"
#include  "core/src/java/org/apache/lucene/codecs/PointsWriter.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/index/Sorter.h"
#include  "core/src/java/org/apache/lucene/index/DocMap.h"
#include  "core/src/java/org/apache/lucene/index/PointValues.h"
#include  "core/src/java/org/apache/lucene/codecs/MutablePointValues.h"
#include  "core/src/java/org/apache/lucene/index/IntersectVisitor.h"

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
namespace org::apache::lucene::index
{

using MutablePointValues = org::apache::lucene::codecs::MutablePointValues;
using PointsWriter = org::apache::lucene::codecs::PointsWriter;
using ByteBlockPool = org::apache::lucene::util::ByteBlockPool;
using BytesRef = org::apache::lucene::util::BytesRef;
using Counter = org::apache::lucene::util::Counter;

/** Buffers up pending byte[][] value(s) per doc, then flushes when segment
 * flushes. */
class PointValuesWriter : public std::enable_shared_from_this<PointValuesWriter>
{
  GET_CLASS_NAME(PointValuesWriter)
private:
  const std::shared_ptr<FieldInfo> fieldInfo;
  const std::shared_ptr<ByteBlockPool> bytes;
  const std::shared_ptr<Counter> iwBytesUsed;
  std::deque<int> docIDs;
  int numPoints = 0;
  int numDocs = 0;
  int lastDocID = -1;
  const int packedBytesLength;

public:
  PointValuesWriter(std::shared_ptr<DocumentsWriterPerThread> docWriter,
                    std::shared_ptr<FieldInfo> fieldInfo);

  // TODO: if exactly the same value is added to exactly the same doc, should we
  // dedup?
  virtual void addPackedValue(int docID, std::shared_ptr<BytesRef> value);

  virtual void flush(std::shared_ptr<SegmentWriteState> state,
                     std::shared_ptr<Sorter::DocMap> sortMap,
                     std::shared_ptr<PointsWriter> writer) ;

private:
  class MutablePointValuesAnonymousInnerClass : public MutablePointValues
  {
    GET_CLASS_NAME(MutablePointValuesAnonymousInnerClass)
  private:
    std::shared_ptr<PointValuesWriter> outerInstance;

  public:
    MutablePointValuesAnonymousInnerClass(
        std::shared_ptr<PointValuesWriter> outerInstance);

    std::deque<int> const ords;

    void intersect(std::shared_ptr<IntersectVisitor> visitor) throw(
        IOException) override;

    int64_t
    estimatePointCount(std::shared_ptr<IntersectVisitor> visitor) override;

    std::deque<char> getMinPackedValue() override;

    std::deque<char> getMaxPackedValue() override;

    int getNumDimensions() override;

    int getBytesPerDimension() override;

    int64_t size() override;

    int getDocCount() override;

    void swap(int i, int j) override;

    int getDocID(int i) override;

    void getValue(int i, std::shared_ptr<BytesRef> packedValue) override;

    char getByteAt(int i, int k) override;

  protected:
    std::shared_ptr<MutablePointValuesAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<MutablePointValuesAnonymousInnerClass>(
          org.apache.lucene.codecs.MutablePointValues::shared_from_this());
    }
  };

private:
  class PointsReaderAnonymousInnerClass : public PointsReader
  {
    GET_CLASS_NAME(PointsReaderAnonymousInnerClass)
  private:
    std::shared_ptr<PointValuesWriter> outerInstance;

    std::shared_ptr<org::apache::lucene::index::PointValues> values;

  public:
    PointsReaderAnonymousInnerClass(
        std::shared_ptr<PointValuesWriter> outerInstance,
        std::shared_ptr<org::apache::lucene::index::PointValues> values);

    std::shared_ptr<PointValues>
    getValues(const std::wstring &fieldName) override;

    void checkIntegrity() override;

    int64_t ramBytesUsed() override;

    virtual ~PointsReaderAnonymousInnerClass();

  protected:
    std::shared_ptr<PointsReaderAnonymousInnerClass> shared_from_this()
    {
      return std::static_pointer_cast<PointsReaderAnonymousInnerClass>(
          org.apache.lucene.codecs.PointsReader::shared_from_this());
    }
  };

public:
  class MutableSortingPointValues final : public MutablePointValues
  {
    GET_CLASS_NAME(MutableSortingPointValues)

  private:
    const std::shared_ptr<MutablePointValues> in_;
    const std::shared_ptr<Sorter::DocMap> docMap;

  public:
    MutableSortingPointValues(std::shared_ptr<MutablePointValues> in_,
                              std::shared_ptr<Sorter::DocMap> docMap);

    void intersect(std::shared_ptr<IntersectVisitor> visitor) throw(
        IOException) override;

  private:
    class IntersectVisitorAnonymousInnerClass
        : public std::enable_shared_from_this<
              IntersectVisitorAnonymousInnerClass>,
          public IntersectVisitor
    {
      GET_CLASS_NAME(IntersectVisitorAnonymousInnerClass)
    private:
      std::shared_ptr<MutableSortingPointValues> outerInstance;

      std::shared_ptr<org::apache::lucene::index::PointValues::IntersectVisitor>
          visitor;

    public:
      IntersectVisitorAnonymousInnerClass(
          std::shared_ptr<MutableSortingPointValues> outerInstance,
          std::shared_ptr<
              org::apache::lucene::index::PointValues::IntersectVisitor>
              visitor);

      void visit(int docID)  override;

      void visit(int docID,
                 std::deque<char> &packedValue)  override;

      Relation compare(std::deque<char> &minPackedValue,
                       std::deque<char> &maxPackedValue) override;
    };

  public:
    int64_t
    estimatePointCount(std::shared_ptr<IntersectVisitor> visitor) override;

    std::deque<char> getMinPackedValue()  override;

    std::deque<char> getMaxPackedValue()  override;

    int getNumDimensions()  override;

    int getBytesPerDimension()  override;

    int64_t size() override;

    int getDocCount() override;

    void getValue(int i, std::shared_ptr<BytesRef> packedValue) override;

    char getByteAt(int i, int k) override;

    int getDocID(int i) override;

    void swap(int i, int j) override;

  protected:
    std::shared_ptr<MutableSortingPointValues> shared_from_this()
    {
      return std::static_pointer_cast<MutableSortingPointValues>(
          org.apache.lucene.codecs.MutablePointValues::shared_from_this());
    }
  };
};

} // #include  "core/src/java/org/apache/lucene/index/
