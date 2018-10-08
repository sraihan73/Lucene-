#pragma once
#include "exceptionhelper.h"
#include "stringhelper.h"
#include <memory>
#include <stdexcept>
#include <deque>

// C++ NOTE: Forward class declarations:
namespace org::apache::lucene::util::packed
{
class PackedLongValues;
}

namespace org::apache::lucene::util::packed
{
class Builder;
}
namespace org::apache::lucene::index
{
class DocsWithFieldSet;
}
namespace org::apache::lucene::util
{
class Counter;
}
namespace org::apache::lucene::index
{
class FieldInfo;
}
namespace org::apache::lucene::index
{
class Sorter;
}
namespace org::apache::lucene::index
{
class DocComparator;
}
namespace org::apache::lucene::search
{
class SortField;
}
namespace org::apache::lucene::index
{
class SortedNumericDocValues;
}
namespace org::apache::lucene::index
{
class DocMap;
}
namespace org::apache::lucene::codecs
{
class DocValuesConsumer;
}
namespace org::apache::lucene::index
{
class SegmentWriteState;
}
namespace org::apache::lucene::util::packed
{
class Iterator;
}
namespace org::apache::lucene::search
{
class DocIdSetIterator;
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
namespace org::apache::lucene::index
{

using DocValuesConsumer = org::apache::lucene::codecs::DocValuesConsumer;
using DocIdSetIterator = org::apache::lucene::search::DocIdSetIterator;
using SortField = org::apache::lucene::search::SortField;
using Counter = org::apache::lucene::util::Counter;
using PackedLongValues = org::apache::lucene::util::packed::PackedLongValues;

// C++ TODO: The Java 'import static' statement cannot be converted to C++:
//    import static org.apache.lucene.search.DocIdSetIterator.NO_MORE_DOCS;

/** Buffers up pending long[] per doc, sorts, then flushes when segment flushes.
 */
class SortedNumericDocValuesWriter : public DocValuesWriter
{
  GET_CLASS_NAME(SortedNumericDocValuesWriter)
private:
  std::shared_ptr<PackedLongValues::Builder> pending; // stream of all values
  std::shared_ptr<PackedLongValues::Builder>
      pendingCounts; // count of values per doc
  std::shared_ptr<DocsWithFieldSet> docsWithField;
  const std::shared_ptr<Counter> iwBytesUsed;
  int64_t bytesUsed =
      0; // this only tracks differences in 'pending' and 'pendingCounts'
  const std::shared_ptr<FieldInfo> fieldInfo;
  int currentDoc = -1;
  std::deque<int64_t> currentValues = std::deque<int64_t>(8);
  int currentUpto = 0;

  std::shared_ptr<PackedLongValues> finalValues;
  std::shared_ptr<PackedLongValues> finalValuesCount;

public:
  SortedNumericDocValuesWriter(std::shared_ptr<FieldInfo> fieldInfo,
                               std::shared_ptr<Counter> iwBytesUsed);

  virtual void addValue(int docID, int64_t value);

  // finalize currentDoc: this sorts the values in the current doc
private:
  void finishCurrentDoc();

public:
  void finish(int maxDoc) override;

private:
  void addOneValue(int64_t value);

  void updateBytesUsed();

public:
  std::shared_ptr<Sorter::DocComparator> getDocComparator(
      int maxDoc,
      std::shared_ptr<SortField> sortField)  override;

private:
  std::deque<std::deque<int64_t>> sortDocValues(
      int maxDoc, std::shared_ptr<Sorter::DocMap> sortMap,
      std::shared_ptr<SortedNumericDocValues> oldValues) ;

public:
  void flush(std::shared_ptr<SegmentWriteState> state,
             std::shared_ptr<Sorter::DocMap> sortMap,
             std::shared_ptr<DocValuesConsumer> dvConsumer) 
      override;

private:
  class EmptyDocValuesProducerAnonymousInnerClass
      : public EmptyDocValuesProducer
  {
    GET_CLASS_NAME(EmptyDocValuesProducerAnonymousInnerClass)
  private:
    std::shared_ptr<SortedNumericDocValuesWriter> outerInstance;

    std::shared_ptr<PackedLongValues> values;
    std::shared_ptr<PackedLongValues> valueCounts;
    std::deque<std::deque<int64_t>> sorted;

  public:
    EmptyDocValuesProducerAnonymousInnerClass(
        std::shared_ptr<SortedNumericDocValuesWriter> outerInstance,
        std::shared_ptr<PackedLongValues> values,
        std::shared_ptr<PackedLongValues> valueCounts,
        std::deque<std::deque<int64_t>> &sorted);

    std::shared_ptr<SortedNumericDocValues>
    getSortedNumeric(std::shared_ptr<FieldInfo> fieldInfoIn) override;

  protected:
    std::shared_ptr<EmptyDocValuesProducerAnonymousInnerClass>
    shared_from_this()
    {
      return std::static_pointer_cast<
          EmptyDocValuesProducerAnonymousInnerClass>(
          EmptyDocValuesProducer::shared_from_this());
    }
  };

private:
  class BufferedSortedNumericDocValues : public SortedNumericDocValues
  {
    GET_CLASS_NAME(BufferedSortedNumericDocValues)
  public:
    const std::shared_ptr<PackedLongValues::Iterator> valuesIter;
    const std::shared_ptr<PackedLongValues::Iterator> valueCountsIter;
    const std::shared_ptr<DocIdSetIterator> docsWithField;

  private:
    int valueCount = 0;
    int valueUpto = 0;

  public:
    BufferedSortedNumericDocValues(
        std::shared_ptr<PackedLongValues> values,
        std::shared_ptr<PackedLongValues> valueCounts,
        std::shared_ptr<DocIdSetIterator> docsWithField);

    int docID() override;

    int nextDoc()  override;

    int advance(int target) override;

    bool advanceExact(int target)  override;

    int docValueCount() override;

    int64_t nextValue() override;

    int64_t cost() override;

  protected:
    std::shared_ptr<BufferedSortedNumericDocValues> shared_from_this()
    {
      return std::static_pointer_cast<BufferedSortedNumericDocValues>(
          SortedNumericDocValues::shared_from_this());
    }
  };

public:
  std::shared_ptr<DocIdSetIterator> getDocIdSet() override;

protected:
  std::shared_ptr<SortedNumericDocValuesWriter> shared_from_this()
  {
    return std::static_pointer_cast<SortedNumericDocValuesWriter>(
        DocValuesWriter::shared_from_this());
  }
};

} // namespace org::apache::lucene::index
