#pragma once
#include "stringhelper.h"
#include <limits>
#include <memory>
#include <string>

// C++ NOTE: Forward class declarations:
#include  "core/src/java/org/apache/lucene/codecs/DocValuesFormat.h"

#include  "core/src/java/org/apache/lucene/codecs/DocValuesConsumer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentWriteState.h"
#include  "core/src/java/org/apache/lucene/codecs/DocValuesProducer.h"
#include  "core/src/java/org/apache/lucene/index/SegmentReadState.h"
#include  "core/src/java/org/apache/lucene/index/FieldInfo.h"
#include  "core/src/java/org/apache/lucene/index/NumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/BinaryDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedNumericDocValues.h"
#include  "core/src/java/org/apache/lucene/index/SortedSetDocValues.h"
#include  "core/src/java/org/apache/lucene/util/Accountable.h"

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
namespace org::apache::lucene::codecs::asserting
{

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocValuesFormat = org::apache::lucene::codecs::DocValuesFormat;
using DocValuesProducer = org::apache::lucene::codecs::DocValuesProducer;
using BinaryDocValues = org::apache::lucene::index::BinaryDocValues;
using FieldInfo = org::apache::lucene::index::FieldInfo;
using NumericDocValues = org::apache::lucene::index::NumericDocValues;
using SegmentReadState = org::apache::lucene::index::SegmentReadState;
using SegmentWriteState = org::apache::lucene::index::SegmentWriteState;
using SortedDocValues = org::apache::lucene::index::SortedDocValues;
using SortedNumericDocValues =
    org::apache::lucene::index::SortedNumericDocValues;
using SortedSetDocValues = org::apache::lucene::index::SortedSetDocValues;
using Accountable = org::apache::lucene::util::Accountable;
using TestUtil = org::apache::lucene::util::TestUtil;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/**
 * Just like the default but with additional asserts.
 */
class AssertingDocValuesFormat : public DocValuesFormat
{
  GET_CLASS_NAME(AssertingDocValuesFormat)
private:
  const std::shared_ptr<DocValuesFormat> in_ =
      TestUtil::getDefaultDocValuesFormat();

public:
  AssertingDocValuesFormat();

  std::shared_ptr<DocValuesConsumer> fieldsConsumer(
      std::shared_ptr<SegmentWriteState> state)  override;

  std::shared_ptr<DocValuesProducer> fieldsProducer(
      std::shared_ptr<SegmentReadState> state)  override;

public:
  class AssertingDocValuesConsumer : public DocValuesConsumer
  {
    GET_CLASS_NAME(AssertingDocValuesConsumer)
  private:
    const std::shared_ptr<DocValuesConsumer> in_;
    const int maxDoc;

  public:
    AssertingDocValuesConsumer(std::shared_ptr<DocValuesConsumer> in_,
                               int maxDoc);

    void addNumericField(std::shared_ptr<FieldInfo> field,
                         std::shared_ptr<DocValuesProducer>
                             valuesProducer)  override;

    void addBinaryField(std::shared_ptr<FieldInfo> field,
                        std::shared_ptr<DocValuesProducer>
                            valuesProducer)  override;

    void addSortedField(std::shared_ptr<FieldInfo> field,
                        std::shared_ptr<DocValuesProducer>
                            valuesProducer)  override;

    void addSortedNumericField(std::shared_ptr<FieldInfo> field,
                               std::shared_ptr<DocValuesProducer>
                                   valuesProducer)  override;

    void addSortedSetField(std::shared_ptr<FieldInfo> field,
                           std::shared_ptr<DocValuesProducer>
                               valuesProducer)  override;

    virtual ~AssertingDocValuesConsumer();

  protected:
    std::shared_ptr<AssertingDocValuesConsumer> shared_from_this()
    {
      return std::static_pointer_cast<AssertingDocValuesConsumer>(
          org.apache.lucene.codecs.DocValuesConsumer::shared_from_this());
    }
  };

public:
  class AssertingDocValuesProducer : public DocValuesProducer
  {
    GET_CLASS_NAME(AssertingDocValuesProducer)
  private:
    const std::shared_ptr<DocValuesProducer> in_;
    const int maxDoc;

  public:
    AssertingDocValuesProducer(std::shared_ptr<DocValuesProducer> in_,
                               int maxDoc);

    std::shared_ptr<NumericDocValues>
    getNumeric(std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<BinaryDocValues>
    getBinary(std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<SortedDocValues>
    getSorted(std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<SortedNumericDocValues> getSortedNumeric(
        std::shared_ptr<FieldInfo> field)  override;

    std::shared_ptr<SortedSetDocValues>
    getSortedSet(std::shared_ptr<FieldInfo> field)  override;

    virtual ~AssertingDocValuesProducer();

    int64_t ramBytesUsed() override;

    std::shared_ptr<std::deque<std::shared_ptr<Accountable>>>
    getChildResources() override;

    void checkIntegrity()  override;

    std::shared_ptr<DocValuesProducer>
    getMergeInstance()  override;

    virtual std::wstring toString();

  protected:
    std::shared_ptr<AssertingDocValuesProducer> shared_from_this()
    {
      return std::static_pointer_cast<AssertingDocValuesProducer>(
          org.apache.lucene.codecs.DocValuesProducer::shared_from_this());
    }
  };

protected:
  std::shared_ptr<AssertingDocValuesFormat> shared_from_this()
  {
    return std::static_pointer_cast<AssertingDocValuesFormat>(
        org.apache.lucene.codecs.DocValuesFormat::shared_from_this());
  }
};

} // #include  "core/src/java/org/apache/lucene/codecs/asserting/
